<?php

/**
 * Implements hook_token_info()
 */
function redmine_token_info() {
  $info['types']['redmine-issue'] = array(
    'name' => t('Redmine issue'),
    'description' => t('Tokens related to redmine issues.'),
    'needs-data' => 'redmine-issue',
  );
  $info['tokens']['redmine-issue']['id'] = array(
    'name' => t('ID'),
    'description' => t('The unique ID of the Redmine issue.'),
  );
  $info['tokens']['redmine-issue']['project'] = array(
    'name' => t('Project'),
    'description' => t('The project of the Redmine issue.'),
    'type' => 'redmine-project',
  );
  $info['tokens']['redmine-issue']['status'] = array(
    'name' => t('Status'),
    'description' => t("The current status of the Redmine issue (i.e. 'New', 'In Progress' or 'Closed')."),
  );
  $info['tokens']['redmine-issue']['priority'] = array(
    'name' => t('Priority'),
    'description' => t("The current priority of the Redmine issue (i.e. 'Low', 'Normal' or 'High')."),
  );
  $info['tokens']['redmine-issue']['author'] = array(
    'name' => t('Author'),
    'description' => t('The Redmine user that created the issue.'),
    'type' => 'redmine-user',
  );
  $info['tokens']['redmine-issue']['assignee'] = array(
    'name' => t('Assignee'),
    'description' => t('The Redmine user that the issues is currently assigned to.'),
    'type' => 'redmine-user',
  );
  $info['tokens']['redmine-issue']['subject'] = array(
    'name' => t('Subject'),
    'description' => t('The subject of the Redmine issue.'),
  );
  $info['tokens']['redmine-issue']['created'] = array(
    'name' => t('Date created'),
    'description' => t('The date when the Redmine issue was created.'),
    'type' => 'date',
  );
  $info['tokens']['redmine-issue']['updated'] = array(
    'name' => t('Date updated'),
    'description' => t('The date when the Redmine issue was last updated.'),
    'type' => 'date',
  );
  //$info['tokens']['redmine-issue']['custom-field'] = array(
  //  'name' => t('Custom field'),
  //  'description' => t('A custom field value for the Redmine issue.'),
  //  'dynamic' => TRUE,
  //);

  $info['types']['redmine-project'] = array(
    'name' => t('Redmine project'),
    'description' => t('Tokens related to redmine projects.'),
    'needs-data' => 'redmine-project',
  );
  $info['tokens']['redmine-project']['id'] = array(
    'name' => t('ID'),
    'description' => t('The unique ID of the Redmine project.'),
  );
  $info['tokens']['redmine-project']['name'] = array(
    'name' => t('Name'),
    'description' => t('The name of the Redmine project.'),
  );
  $info['tokens']['redmine-project']['created'] = array(
    'name' => t('Date created'),
    'description' => t('The date when the Redmine project was created.'),
    'type' => 'date',
  );
  $info['tokens']['redmine-project']['updated'] = array(
    'name' => t('Date updated'),
    'description' => t('The date when the Redmine project was last updated.'),
    'type' => 'date',
  );

  $info['types']['redmine-user'] = array(
    'name' => t('Redmine user'),
    'description' => t('Tokens related to redmine users.'),
    'needs-data' => 'redmine-user',
  );
  $info['tokens']['redmine-user']['id'] = array(
    'name' => t('ID'),
    'description' => t('The unique ID of the Redmine user.'),
  );
  $info['tokens']['redmine-user']['login'] = array(
    'name' => t('Login name'),
    'description' => t('The unique login name of the Redmine user.'),
  );

  return $info;
}

function redmine_convert_date_to_timestamp($string) {
  if ($datetime = DateTime::createFromFormat(DATE_W3C, $string)) {
    return $datetime->getTimestamp();
  }
}

/**
 * Implements hook_tokens().
 */
function redmine_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $url_options = array('absolute' => TRUE);
  if (isset($options['language'])) {
    $url_options['language'] = $options['language'];
  }

  $langcode = !empty($options['language']->language) ? $options['language']->language : NULL;
  $sanitize = !empty($options['sanitize']);

  $replacements = array();

  // Redmine issue tokens.
  if ($type == 'redmine-issue' && !empty($data['redmine-issue'])) {
    $issue = $data['redmine-issue'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'id':
          $replacements[$original] = $sanitize ? check_plain($issue->id) : $issue->id;
          break;

        case 'project':
          $replacements[$original] = $sanitize ? check_plain($issue->project['name']) : $issue->project['name'];
          break;

        case 'status':
          $replacements[$original] = $sanitize ? check_plain($issue->status['name']) : $issue->status['name'];
          break;

        case 'priority':
          $replacements[$original] = $sanitize ? check_plain($issue->priority['name']) : $issue->priority['name'];
          break;

        case 'author':
          $replacements[$original] = $sanitize ? check_plain($issue->author['name']) : $issue->author['name'];
          break;

        case 'assignee':
          $replacements[$original] = $sanitize ? check_plain($issue->author['name']) : $issue->author['name'];
          break;

        case 'subject':
          $replacements[$original] = $sanitize ? check_plain($issue->subject) : $issue->subject;
          break;

        case 'created':
          if ($created = redmine_convert_date_to_timestamp($issue->created_on)) {
            $replacements[$original] = format_date($created, 'medium', '', NULL, $langcode);
          }
          break;

        case 'updated':
          if ($updated = redmine_convert_date_to_timestamp($issue->updated_on)) {
            $replacements[$original] = format_date($updated, 'medium', '', NULL, $langcode);
          }
          break;
      }
    }

    // [redmine-issue:project:*] nested tokens.
    if ($project_tokens = token_find_with_prefix($tokens, 'project')) {
      $project = new RedmineProject();
      $project->find($issue->project['id']);
      if (empty($project->error)) {
        $replacements += token_generate('redmine-project', $project_tokens, array('redmine-project' => $author), $options);
      }
    }

    // [redmine-issue:created:*] nested tokens.
    if ($created_tokens = token_find_with_prefix($tokens, 'created')) {
      if ($created = redmine_convert_date_to_timestamp($issue->created_on)) {
        $replacements += token_generate('date', $created_tokens, array('date' => $created), $options);
      }
    }

    // [redmine-issue:updated:*] nested tokens.
    if ($updated_tokens = token_find_with_prefix($tokens, 'updated')) {
      if ($updated = redmine_convert_date_to_timestamp($issue->updated_on)) {
        $replacements += token_generate('date', $updated_tokens, array('date' => $updated), $options);
      }
    }
  }

  // Redmine project tokens.
  if ($type == 'redmine-project' && !empty($data['redmine-project'])) {
    $project = $data['redmine-project'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'id':
          $replacements[$original] = $sanitize ? check_plain($project->id) : $project->id;
          break;

        case 'name':
          $replacements[$original] = $sanitize ? check_plain($project->name) : $project->name;
          break;

        case 'created':
          if ($created = redmine_convert_date_to_timestamp($project->created_on)) {
            $replacements[$original] = format_date($created, 'medium', '', NULL, $langcode);
          }
          break;

        case 'updated':
          if ($updated = redmine_convert_date_to_timestamp($project->updated_on)) {
            $replacements[$original] = format_date($updated, 'medium', '', NULL, $langcode);
          }
          break;
      }
    }

    // [redmine-project:created:*] nested tokens.
    if ($created_tokens = token_find_with_prefix($tokens, 'created')) {
      if ($created = redmine_convert_date_to_timestamp($project->created_on)) {
        $replacements += token_generate('date', $created_tokens, array('date' => $created), $options);
      }
    }

    // [redmine-project:updated:*] nested tokens.
    if ($updated_tokens = token_find_with_prefix($tokens, 'updated')) {
      if ($updated = redmine_convert_date_to_timestamp($project->updated_on)) {
        $replacements += token_generate('date', $updated_tokens, array('date' => $updated), $options);
      }
    }
  }
}
