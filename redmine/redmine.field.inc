<?php

/**
 * Implements hook_field_info().
 *
 * // @todo Convert this to use our own widget for selecting a combined server and api token.
 */
function redmine_field_info() {
  $info['redmine_api_key'] = array(
    'label' => t('Redmine API key'),
    'description' => t('This field stores a Redmine API key as varchar text in the database.'),
    // Need to provide defaults since we're reusing text widget and formatters.
    'settings' => array('max_length' => 255),
    'instance_settings' => array('text_processing' => 0),
    'default_widget' => 'redmine_api_key_default',
    'default_formatter' => 'text_plain',
    'default_token_formatter' => 'text_plain',
  );

  return $info;
}

/**
 * Implements hook_field_formatter_info_alter().
 */
function redmine_field_formatter_info_alter(&$info) {
  $info['text_plain']['field types'][] = 'redmine_api_key';
}

/**
 * Implements hook_field_is_empty().
 */
function redmine_field_is_empty($item, $field) {
  return !isset($item['value']) || trim($item['value']) === '';
}

/**
 * Implements hook_field_access().
 */
function redmine_field_access($op, $field, $entity_type, $entity, $account) {
  if ($field['type'] == 'redmine_api_key') {
    // For any API key field, disable access to fields if there are no servers
    // configured.
    if (!redmine_server_load_all_enabled()) {
      return FALSE;
    }
  }
}

/**
 * Implements hook_field_widget_form().
 */
function redmine_field_widget_info() {
  $info['redmine_api_key_default'] = array(
    'label' => t('Redmine server and API key'),
    'field types' => array('redmine_api_key'),
    'settings' => array(
      'size' => 42,
      // @todo Add a setting to allow the user to select which server.
      // @todo Add setting for the default server.
    ),
  );

  return $info;
}

/**
 * Implements hook_field_widget_form().
 */
function redmine_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  switch ($instance['widget']['type']) {
    case 'redmine_api_key_default':
      $element['value'] = array(
        '#type' => 'textfield',
        '#title' => $instance['label'],
        '#default_value' => isset($items[$delta]['value']) ? $items[$delta]['value'] : '',
        '#maxlength' => 255,
        '#size' => $instance['widget']['settings']['size'],
      );
      // @todo Figure out how to inline this with the API key textfield.
      $options = redmine_get_server_options();
      $element['server'] = array(
        '#type' => 'select',
        '#title' => t('Redmine server'),
        '#options' => $options,
        '#default_value' => isset($items[$delta]['server']) ? $items[$delta]['server'] : key($options),
        '#access' => count($options) > 1,
        '#prefix' => '<div class="container-inline">',
        '#suffix' => '</div>',
      );
      break;
  }

  return $element;
}

/**
 * Implements hook_field_widget_error().
 */
function redmine_field_widget_error($element, $error, $form, &$form_state) {
  form_error($element, $error['message']);
}

/**
 * Implements hook_field_validate().
 *
 * Possible error codes:
 * - 'redmine_api_key_invalid': An invalid Redmine API key for the server.
 */
function redmine_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  foreach ($items as $delta => $item) {
    $value = trim($item['value']);
    if ($value !== '' && $server = redmine_server_load($item['server'])) {
      if (preg_match('/^[a-f\d]{40}$/', $value)) {
        try {
          $connection = new DrupalRedmineConnection($server, $value);
          $data = $connection->getCurrentUser();
          if (empty($data)) {
            throw new RedmineException(t('Unable to contact server or invalid API key.'));
          }
        }
        catch (RedmineException $e) {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => 'redmine_api_key_invalid',
            'message' => t('%name: @message.', array('@message' => $e->getMessage(), '%name' => $instance['label'])),
          );
        }
      }
      else {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => 'redmine_api_key_invalid',
          'message' => t('%name: Invalid API key.', array('%name' => $instance['label'])),
        );
      }
    }
  }
}
