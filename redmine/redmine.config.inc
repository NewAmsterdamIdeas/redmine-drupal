<?php
/**
 * @file
 * Defines the Redmine API Configuration class.
 */

/**
 * Defines a simple class that returns a $site value based
 * on the URL, username and password being used to access a
 * Redmine instance.
 */
class RedmineAPIConfig {
  public $user;
  public $password;
  public $url;
  public $limit_max = 100;

  function __construct ($data = array ()) {
    $this->user = 'username';
    $this->password = 'password';
    $this->url = 'http://127.0.0.1:8080/redmine/';
  }

  function site() {
    if (function_exists('drupal_alter')) {
      drupal_alter('redmine_config', $this, $site);
      if ($site) {
        return $site;
      }
    }
    preg_match('|^(https?://)(.*)$|', $this->url, $matches);
    return $matches[1] . $this->user . ':' . $this->password . '@' . $matches[2];
  }
}
