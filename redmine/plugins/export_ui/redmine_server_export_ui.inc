<?php

/**
* Define this Export UI plugin.
*/
$plugin = array(
  'schema' => 'redmine_server',
  'access' => 'administer redmine',
  'menu' => array(
    'menu item' => 'redmine',
    'menu title' => 'Redmine',
    'menu description' => 'Configure available Redmine server and settings.',
    'menu prefix' => 'admin/config/services',
  ),
  'title singular' => t('Redmine server'),
  'title plural' => t('Redmine servers'),
  'title singular proper' => t('Redmine server'),
  'title plural proper' => t('Redmine servers'),
  'form' => array(
    'settings' => 'redmine_server_edit_form',
    'validate' => 'redmine_server_edit_form_validate',
    //'submit' => 'redmine_server_edit_form_submit',
  ),
  'export' => array(
    'admin_title' => 'name',
  ),
  'handler' => 'redmine_server_ui',
);

function redmine_server_edit_form(&$form, &$form_state) {
  $server = $form_state['item'];

  $form['info']['name']['#title'] = t('Server name');
  $form['info']['name']['#attributes']['placeholder'] = t('My Redmine Server');
  $form['info']['machine_name']['#title'] = t('Machine name');

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => $server->url,
    '#required' => TRUE,
    '#attributes' => array(
      'placeholder' => 'http://redmine.example.com:3000/',
    ),
  );

  $form['global_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Global API key'),
    '#default_value' => $server->global_api_key,
    '#description' => t('The API key of a normal Redmine user to be used for global API requests.'),
    '#disabled' => TRUE,
  );
}

function redmine_server_edit_form_validate($form, &$form_state) {
  if (!valid_url($form_state['values']['url'], TRUE)) {
    form_set_error('url', t('The URL %url is not valid.', array('%url' => $form_state['values']['url'])));
  }
}

//function redmine_server_edit_form_submit($form, &$form_state) {
//  form_state_values_clean($form_state);
//  $server = (object) $form_state['values'];
//  redmine_server_save($server);
//}
