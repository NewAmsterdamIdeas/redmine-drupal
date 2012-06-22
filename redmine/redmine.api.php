<?php
/**
 * @file
 * This file documents the hooks defined by the Redmine API module.
 */

/**
 * Hook to alter configuration information for accessing a Redmine website.
 *
 * @param $config
 *   An object of type RedmineAPIConfig.
 * @param $site
 *   The full URL (including Redmine username and password) needed to
 *   access the specified Redmine site.
 *
 * @see redmine.config.inc
 */
function hook_redmine_config_alter(&$config, &$site=NULL) {
  global $user;
  $user_data = user_load($user->uid);
  $config->user = $user_data->field_redmine_username['und'][0]['value'];
  $config->password = $user_data->field_redmine_password['und'][0]['value'];
}

