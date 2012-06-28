<?php

function redmine_settings_form($form, &$form_state) {
  $form['redmine_bot_issue_message'] = array(
    '#type' => 'textfield',
    '#title' => t('What to output when a Redmine issue is mentiond in IRC.'),
    '#default_value' => variable_get('redmine_bot_issue_message', 'Issue #[redmine-issue:id]: [redmine-issue:subject] (Assigned to [redmine-issue:assignee])'),
    '#access' => module_exists('bot'),
    '#description' => t('This field can use tokens.'),
    '#element_validate' => array('token_element_validate'),
    '#token_types' => array('redmine-issue'),
  );

  $form['tokens'] = array(
    '#theme' => 'token_tree',
    '#token_types' => array('redmine-issue'),
    '#access' => module_exists('bot'),
  );

  return system_settings_form($form);
}

function redmine_settings_form_validate($form, &$form_state) {
}
