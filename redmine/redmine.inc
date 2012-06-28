<?php
require_once ('redmine.config.inc');

class RedmineActiveResource extends ActiveResource {
  /**
   * Fetch the specified request via drupal_http_request() if cURL is not enabled.
   */
  function _fetch ($url, $method, $params) {
    if (extension_loaded('curl')) {
      return parent::_fetch($url, $method, $params);
    }

    $options['headers'] = array();

    if ($this->request_format == 'xml') {
      $options['headers']['Content-Type'] = 'text/xml';
      $options['headers']['Length'] = strlen($params);
    }
    switch ($method) {
      case 'POST':
      case 'PUT':
        $options['method'] = $method;
        $options['data'] = $params;
          break;
      case 'DELETE':
        $options['method'] = 'DELETE';
        break;
      case 'GET':
      default:
        break;
    }

    $request = drupal_http_request($url, $options);

    if (!empty($request->error)) {
      $this->errno = $request->code;
      $this->error = $request->error;
    }

    if ($request->code == 401) {
      $this->errno = 401;
//OOPS
//      $this->error = $
    }

    $res = new stdClass();
//OOPS
//    $res->

      // Check HTTP status code for denied access
      $http_code = curl_getinfo ($ch, CURLINFO_HTTP_CODE);
      if ($http_code == 401) {
          $this->errno = $http_code;
          $this->error = "HTTP Basic: Access denied.";
          curl_close ($ch);
          return false;
      }

      if (! $res) {
          $this->errno = curl_errno ($ch);
          $this->error = curl_error ($ch);
          curl_close ($ch);
          return false;
      }
      curl_close ($ch);
      return $res;
  }
  function __construct ($data = array ()) {
    parent::__construct($data);
    $config = new RedmineAPIConfig();
    $this->site = $config->site();
    $this->request_format = 'xml';
  }

  /**
   * Helper function to return attributes of a resource.
   *
   * @param $section
   *   The name of the section whose attributes are requested
   * @param $roles
   *   An array of user roles to be added.
   *
   * @return
   *   A simple associative array.
   */
  function attributes($section) {
    if (!isset($this->_data[$section]) || get_class($this->_data[$section]) != 'SimpleXMLElement') {
      return NULL;
    }
    $attributes = array();
    foreach ($this->_data[$section]->attributes() as $name => $value) {
      $attributes['@'.$name] = (string)$value;
    }
    return $attributes;
  }

  function _data_to_array($section, $list_wrapper=NULL) {
    if (!isset($this->_data[$section])) {
      return NULL;
    }
    if (get_class($this->_data[$section]) == 'SimpleXMLElement') {
      $result = $this->sx_array($this->_data[$section]);
      if ($list_wrapper && isset($result[$list_wrapper]) && $this->isAssoc($result[$list_wrapper])) {
        $result[$list_wrapper] = array($result[$list_wrapper]);
      }
      return $result;
    }
    return (string) $this->_data[$section];
  }
  
  function sx_array($obj){
      $arr = (array)$obj;
      if(empty($arr)){
          $arr = "";
      } else {
          foreach($arr as $key=>$value){
              if(!is_scalar($value)){
                  $arr[$key] = $this->sx_array($value);
              }
          }
      }
      return $arr;
  }

  function isAssoc($arr) {
    return array_keys($arr) !== range(0, count($arr) - 1);
  }
  
  /**
   * Return attributes of a resource as an associative array.
   *
   * @param $schema
   *   A simple associative array of the form:
   *     array(
   *       key1 => value1,
   *       key2 => value2,
   *       ...
   *     )
   *   If the value of a key is NULL, the data returned for
   *   that key will only contain a single result. If the value is not
   *   null, the data for that key will consist of a simple
   *   array that may contain multiple results.
   *   Example:
   *      array(
   *        'id' => NULL,
   *        'children' => 'issue',
   *      )
   *   This tells the data method to return a result that would
   *   look something like this:
   *      array(
   *        'id' => 1,
   *        'children' => array(
   *           [issue] => array(
   *              0 => [data for the first issue]
   *              1 => [data for the second issue]
   *              ...
   *           )
   *        )
   *      )
   *   For additional examples, see the overridden data() methods
   *   for the RedmineIssue, RedmineProject and other classes, and
   *   examine the schemas provided by those overriden methods
   *   as well as results actually returned.
   *
   * @return
   *   A simple associative array.
   */
  function data($schema=NULL) {
    if (!$schema) {
      $schema=array('id' => NULL);
    }
    foreach ($schema as $key => $value) {
      if ($value) {
        $data[$key] = $this->_data_to_array($key, $value);
      }
      else {
        $data[$key] = $this->_data_to_array($key);
      }
    }
    return $data;
  }

  /**
   * Count the number of items of a particular type.
   *
   * Redmine API returns a "total_count" property with all index queries,
   * regardless of the offset or limit. For the purpose of counting, the
   * is set to 1, to optimize the query for performance.
   */
  function count_items($type, $options = array()) {
    $options['limit'] = 1;
    $url = $this->site . $type . '.xml';
    $url .= '?' . http_build_query ($options);
    $this->request_body = http_build_query ($options);
    $this->request_uri = $url;
    $this->request_method = 'GET';
    $res = $this->_fetch($url, 'GET', '');
    if ($res === false) {
      return 0;
    }
    // Keep splitting off any top headers until we get to the (XML) body:
    while (strpos($res, "HTTP/") === 0) {
      list ($headers, $res) = explode ("\r\n\r\n", $res, 2);
      $this->response_headers = $headers;
      $this->response_body = $res;
      if (preg_match ('/HTTP\/[0-9]\.[0-9] ([0-9]+)/', $headers, $regs)) {
        $this->response_code = $regs[1];
      } else {
        $this->response_code = false;
      }

      if (!$res) {
        return 0;
      } elseif ($res == ' ') {
        $this->error = 'Empty reply';
        return 0;
      }
    }

    // parse XML response
    $xml = new SimpleXMLElement ($res);
    // normalize xml element name in case rails ressource contains an underscore
    $attributes = $xml->attributes();
    if (isset($attributes['total_count'])) {
      return (string)$attributes['total_count'];
    }
  }

  /**
   * Returns a project's subentities
   */
  function subentities($singular_name, $plural_name, $object_name) {
    $get_result = $this->get($plural_name);
    $entities = array();
    if (!isset($get_result->_data[$singular_name])) {
      return $entities;
    }
    $raw_entities = $get_result->_data[$singular_name];

    if (!is_array($raw_entities)) {
      $raw_entities = array($raw_entities);
    }
    foreach ($raw_entities as $raw_entity) {
      $entity = new $object_name();
      $entity = $entity->get((string)$raw_entity->id);
      $entities[] = $entity;
    }
    return $entities;
  }

}

/**
 * Parameters accepted by the find() method for a RedmineIssue object include:
 *   offset: skip this number of issues in response (optional)
 *   limit: number of issues per page (optional)
 *   sort: column to sort with. Append :desc to invert the order.
 *   page: page number (optional)
 *
 * Optional filters:
 *
 *   project_id: get issues from the project with the given id, where id is either project id or project identifier
 *   tracker_id: get issues from the tracker with the given id
 *   status_id: get issues with the given status id only. Possible values: open, closed, * to get open and closed issues, status id
 *   assigned_to_id: get issues which are assigned to the given user id
 *   cf_x: get issues with the given value for custom field with an ID of x. (Custom field must have 'used as a filter' checked.)
 *
 * Optional filters:
 *
 *   project_id: get issues from the project with the given id, where id is either project id or project identifier
 *   tracker_id: get issues from the tracker with the given id
 *   status_id: get issues with the given status id only. Possible values: open, closed, * to get open and closed issues, status id
 *   assigned_to_id: get issues which are assigned to the given user id
 *   cf_x: get issues with the given value for custom field with an ID of x. (Custom field must have 'used as a filter' checked.)
 */
// TODO: implement changesets handling
class RedmineIssue extends RedmineActiveResource {
  
  var $element_name = 'issue';
  
  function data() {
    return parent::data(array(
      'id' => NULL,
      'project' => NULL,
      'tracker' => NULL,
      'status' => NULL,
      'priority' => NULL,
      'author' => NULL,
      'assigned_to' => NULL,
      'subject' => NULL,
      'description' => NULL,
      'start_date' => NULL,
      'due_date' => NULL,
      'done_ratio' => NULL,
      'estimated_hours' => NULL,
      'created_on' => NULL,
      'updated_on' => NULL,
      'children' => 'issue',
      'attachments' => 'attachment',
      'relations' => 'relation',
      'changesets' => 'changeset',
      'journals' => 'journal',
    ));
  }

  /**
   * Returns an issue's relations
   */
  function relations() {
    return $this->subentities('relation', 'relations', 'RedmineIssueRelation');
  }
}

class RedmineProject extends RedmineActiveResource {
  var $element_name = 'project';

  function data() {
    return parent::data(array(
      'id' => NULL,
      'name' => NULL,
      'identifier' => NULL,
      'homepage' => NULL,
      'description' => NULL,
      'created_on' => NULL,
      'updated_on' => NULL,
      'trackers' => 'tracker',
      'issue_categories' => 'issue_category',
    ));
  }
  

  // TODO: See if it is possible to make trackers and issue categories updateable.
  function save () {
    $updateable_elements = array(
      'name',
//      'identifier',
//      'homepage',
      'description',
//      'trackers',
//      'issue_categories',
    );
    $sanitized_array = array();
    foreach ($updateable_elements as $element) {
      $sanitized_array[$element] = $this->_data[$element];
    }
    if (isset ($this->_data['id'])) {
      return $this->_send_and_receive ($this->site . $this->element_name_plural . '/' . $this->_data['id'] . '.xml', 'PUT', $sanitized_array); // update
    }
    return $this->_send_and_receive ($this->site . $this->element_name_plural . '.xml', 'POST', $this->_data); // create
  }

  /**
   * Returns a project's memberships
   */
  function memberships() {
    return $this->subentities('membership', 'memberships', 'RedmineProjectMembership');
  }
  
  /**
   * Adds a new membership to the project.
   *
   * @param $user_id
   *   The Redmine user id of the person to be added as a member
   * @param $roles
   *   An array of user roles to be added.
   */
  function add_membership($user_id, $roles=array()) {
    if (!is_array($roles)) {
      $roles = array($roles);
    }
    $this->post('memberships', array(
      'user_id' => $user_id,
      'role_ids' => array(
          '@type' => 'array',
          'role_id' => $roles,
        ),
      ), 'membership'
    );
  }

  /**
   * Returns a project's versions
   */
  function versions() {
    return $this->subentities('version', 'versions', 'RedmineProjectVersion');
  }

  /**
   * Returns a project's versions
   */
  function issue_categories() {
    return $this->subentities('issue_category', 'issue_categories', 'RedmineIssueCategory');
  }
}

class RedmineProjectMembership extends RedmineActiveResource {
  var $element_name = 'membership';
  
  /**
   * Updates the roles for a particular membership
   *
   * @param $roles
   *   An array of user roles to be assigned to this membership.
   */
  function update($roles) { 
    if (is_int($roles)) {
      $roles = array($roles);
    }
    $req = $this->site . $this->element_name_plural;
    $req .= '/' . $this->_data['id'] . '.xml';
    $options = array(
      'role_ids' => array(
        '@type' => 'array',
        'role_id' => $roles,
      ),
    );
    return $this->_send_and_receive($req, 'PUT', $options);
  }

  /**
   * Returns a membership's data in simplified array form
   */
  function data() {
    return parent::data(array(
      'id' => NULL,
      'project' => NULL,
      'user' => NULL,
      'roles' => 'role',
    ));
  }
}

class RedmineUser extends RedmineActiveResource {
  var $element_name = 'user';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'login' => NULL,
      'firstname' => NULL,
      'lastname' => NULL,
      'mail' => NULL,
      'created_on' => NULL,
      'last_login_on' => NULL,
      'memberships' => 'membership',
      'groups' => 'group',
    ));
  }
}

class RedmineTimeEntry extends RedmineActiveResource {
  var $element_name = 'time_entry';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'project' => NULL,
      'issue' => NULL,
      'user' => NULL,
      'activity' => NULL,
      'hours' => NULL,
      'comments' => NULL,
      'spent_on' => NULL,
      'created_on' => NULL,
      'updated_on' => NULL,
    ));
  }
}

class RedmineIssueRelation extends RedmineActiveResource {
  var $element_name = 'relation';
    function data() {
      return parent::data(array(
        'id' => NULL,
        'issue_id' => NULL,
        'issue_to_id' => NULL,
        'relation_type' => NULL,
        'delay' => NULL,
      ));
    }
}

class RedmineProjectVersion extends RedmineActiveResource {
  var $element_name = 'version';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'project' => NULL,
      'name' => NULL,
      'status' => NULL,
      'due_date' => NULL,
      'created_on' => NULL,
      'updated_on' => NULL,
    ));
  }
}

class RedmineQuery extends RedmineActiveResource {
  var $element_name = 'query';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'name' => NULL,
      'is_public' => NULL,
      'project_id' => NULL,
    ));
  }
}

class RedmineAttachment extends RedmineActiveResource {
  var $element_name = 'attachment';
}

class RedmineIssueStatus extends RedmineActiveResource {
  var $element_name = 'issue_status';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'name' => NULL,
      'is_default' => NULL,
      'is_closed' => NULL,
    ));
  }
}

class RedmineTracker extends RedmineActiveResource {
  var $element_name = 'tracker';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'name' => NULL,
    ));
  }
}

class RedmineIssueCategory extends RedmineActiveResource {
  var $element_name = 'issue_category';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'project' => NULL,
      'name' => NULL,
      'assigned_to' => NULL,
    ));
  }
}

class RedmineRole extends RedmineActiveResource {
  var $element_name = 'role';
  function data() {
    return parent::data(array(
      'id' => NULL,
      'name' => NULL,
    ));
  }
  function simple_list() {
    $existing_roles = $this->find('all');
    $roles_table = array();
    foreach ($existing_roles as $role) {
      $roles_table[$role->_data['id']] = $role->_data['name'];
    }
    return $roles_table;
  }
}

class RedmineGroup extends RedmineActiveResource {
  var $element_name = 'group';
}

