<?php

/**
 * @file
 * Definition of DrupalRedmineConnection.
 */

/**
 * Extends the RedmineConnection class to use drupal_http_request() and caching.
 */
class DrupalRedmineConnection extends RedmineConnection {
  // Drupal specific user agent string.
  private $userAgent = 'Drupal Redmine module (+http://drupal.org/project/redmine)';

  public function setServer($server) {
    $this->server = $server;
  }

  public function getServer() {
    return $this->server->url;
  }

  public function request($resource, array $options = array()) {
    $options += array(
      'headers' => array(),
      'method' => 'GET',
      'query' => array(),
      'data' => NULL,
      'cache' => variable_get('redmine_cache_requests', 0),
      'cache expire' => variable_get('redmine_cache_expire', CACHE_PERMANENT),
      'timeout' => 45, // Increase the default timeout limit from 30 seconds.
    );

    $url = $this->getURL($resource, $options['query']);

    if (isset($options['data'])) {
      $options['data'] = json_encode($options['data']);
      $options['headers']['Content-Type'] = 'application/json';
    }

    // Add more default headers.
    $options['headers']['X-Redmine-API-Key'] = $this->getKey();
    $options['headers']['User-Agent'] = $this->userAgent;

    // Generate a unique CID for this request (and only GET requests can be cached).
    $is_cacheable = !empty($options['cache']) && $options['method'] == 'GET';
    $cid = drupal_hash_base64(serialize(array($url, array('cache' => NULL, 'cache expire' => NULL) + $options)));

    if ($is_cacheable) {
      if ($cache = cache_get($cid, 'cache_redmine', $options['cache expire'])) {
        return $cache->data;
      }
    }

    $response = drupal_http_request($url, $options);
    if (!empty($response->error)) {
      throw new RedmineException($response->error);
    }

    $response->data = json_decode($response->data, TRUE);
    $response->success = $response->code == 200;

    if ($is_cacheable) {
      cache_set($cid, $response, 'cache_toggl', $options['cache expire']);
    }

    return $response;
  }
}
