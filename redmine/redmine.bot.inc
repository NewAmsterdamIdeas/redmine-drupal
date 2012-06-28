<?php

/**
 * Bot URL integration for the Redmine module.
 */

function redmine_bot_get_server_issue_regexes() {
  $regexes = &drupal_static(__FUNCTION__);
  if (!isset($regex)) {
    $regexes = array();
    foreach (redmine_server_load_all_enabled() as $server) {
      // Servers must have a global api key enabled for bot issue fetching.
      if (!empty($server->global_api_key)) {
        $server->url_issue_base = rtrim($server->url, '/') . '/issues/';
        $regex = '/^' . preg_quote($server->url_issue_base) . '(\d)+/';
        $regexes[$regex] = $server;
      }
    }
  }
  return $regexes;
}

/**
 * Implements hook_bot_url_irc_message_alter().
 */
function redmine_bot_url_irc_message_alter(&$message, $request, $data) {
  $issue_message = variable_get('redmine_bot_issue_message', 'Issue #[redmine-issue:id]: [redmine-issue:subject] (Assigned to [redmine-issue:assignee])');

  foreach (redmine_bot_get_server_issue_regexes() as $regex => $server) {
    if (preg_match($regex, $request->url, $matches)) {
      try {
        $connection = new DrupalRedmineConnection($server, $server->global_api_key);
        $issue = RedmineIssue::load($connection, $matches[1]);
        $message = token_replace($issue_message, array('redmine-issue' => $issue->getData()), array('clear' => TRUE));
      }
      catch (Exception $e) {
        // Do nothing.
      }
    }
  }
}
