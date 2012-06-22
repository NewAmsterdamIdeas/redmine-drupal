The Redmine API module provides a Drupal API to the REST API of
Redmine, a project-management and issue-tracking web-application 
and open source software package. It provides functions for listing,
getting, setting and deleting objects corresponding to various
data types defined by the Redmine API. Available object classes
include:

* RedmineIssue
* RedmineIssueJournal
* RedmineProject
* RedmineProjectMembership
* RedmineUser
* RedmineTimeEntry
* RedmineQuery
* RedmineIssueStatus
* RedmineTracker
* RedmineRole
* RedmineGroup

Each of these classes has a data() method that can be used to
extract the various field values for the object.

------------
INSTALLATION
------------

Copy to your module directory and then enable on the admin
modules page. The Redmine API module is dependent upon the libraries
module for Drupal, so you will have to install it also:
http://drupal.org/project/libraries

You will also need to install the PHP ActiveResource external
library, which is available at
https://github.com/lux/phpactiveresource

To install the PHP ActiveResource library, download it from 
the URL above, and place the phpactiveresource library 
into sites/all/libraries.

The Redmine API module includes a Redmine Usernames module to
help in connecting Drupal user accounts to Redmine user accounts.
The Redmine Usernames module requires that you install the 
Features module:
http://drupal.org/project/features

--------
EXAMPLES
--------

ISSUES

  // Get and display data from a list of issues
  $issues = redmine_rest_api_issues();
  foreach ($issues as $issue) {
    print_r($issue->data());
  }

  // Get issues assigned to the Redmine user with id 1
  $issues = redmine_rest_api_issues(array(
    'assigned_to_id' => 1,
  ));

  // Get the Redmine issue with id 6
  $issue = redmine_rest_api_issue_load(6);

  // Get the Redmine issue with id 6, as well as its child issues and related issues
  $issue = redmine_rest_api_issue_load(6);

  // Create a new issue
  $issue = redmine_rest_api_issue_create(array(
    'project_id' => 1,
    'subject' => 'This is a subject I created programmatically',
    'description' => 'This is a description',
    'assigned_to_id' => 3,
  ));

  // Add a new note to an issue, and assign it to
  // the Redmine user with id 3
  redmine_rest_api_issue_update($issue, array(
    'notes' => 'This is another automatically created note',
    'assigned_to_id' => 3,
  ));

  // Delete the issue with ID 5
  redmine_rest_api_issue_delete(5);


PROJECTS

  // Get and display data from the list of projects
  $projects = redmine_rest_api_projects();
  foreach ($projects as $project) {
    print_r($project->data());
  }

  // Get the Redmine project with id 1
  $project = redmine_rest_api_project_load(1, array(
    'trackers', 'issue_categories'
  ));

  // Create a new project
  $project = redmine_rest_api_project_create(array(
    'name' => 'Project name',
    'identifier' => 'project-id-666x',
    'description' => 'Project description.',
  ));

  // Change the name and description of a project
  redmine_rest_api_project_update($project, array(
    'name' => 'Shiny new project name',
    'description' => 'Shiny new project description.',
  ));

  //Delete the project with id 4
  redmine_rest_api_project_delete(4);


PROJECT MEMBERSHIPS

  // Get memberships from the project with id 2
  $project = redmine_rest_api_project_load(2);
  $memberships = redmine_rest_api_memberships($project);
  foreach ($memberships as $membership) {
    print_r($membership->data());
  }

  // Get the membership with membership id 5
  $membership = redmine_rest_api_membership_load(5);

  // Add the Redmine user with id 7 to a project, with roles 3 and 4
  redmine_rest_api_membership_create($project, 7, array(3, 4));

  // Update membership 9 to have roles 4 and 5
  $membership = redmine_rest_api_membership_load(9);
  redmine_rest_api_membership_update($membership, array(4, 5));

  // Delete the membership with membership id 10
  redmine_rest_api_membership_delete(10);


USERS

  // Get and display data from the list of Redmine users
  $users = redmine_rest_api_users();
  foreach ($users as $user) {
    print_r($user->data());
  }

  // Get the user with id 3
  $user = redmine_rest_api_user_load(3, array('memberships', 'groups'));

  // Get the current user
  $user = redmine_rest_api_user_load('current', array('memberships', 'groups'));

  // Create a user
  $user = redmine_rest_api_user_create(array(
    'login' => 'aaron',
    'password' => 'packersfan',
    'firstname' => 'Aaron',
    'lastname' => 'Rogers',
    'mail' => 'aaron@sheldonrampton.com',
  ));

  // Get and change values for Redmine user 7
  $user = redmine_rest_api_user_load(7, array(
    'memberships', 'groups'
  ));
  redmine_rest_api_user_update($user, array(
    'login' => 'brett',
    'password' => 'formerpackersfan',
    'firstname' => 'Brett',
    'lastname' => 'Favre',
    'mail' => 'brett@sheldonrampton.com',
  ));


TIME ENTRIES

  // Get and display data from the list of time entries
  $time_entries = redmine_rest_api_time_entries();
  foreach($time_entries as $time_entry) {
    print_r($time_entry->data());
  }

  // Create a new time entry
  redmine_rest_api_time_entry_create(array(
    'issue_id' => 4,
    'hours' => 3.5,
    'activity_id' => 10,
    'comments' => 'This is a brief comment about this time entry',
    'spent_on' => '2012-05-01',
  ));

  // Get and change the time entry with id 5
  $time_entry = redmine_rest_api_time_entry_load(5);
  redmine_rest_api_time_entry_update($time_entry, array(
    'issue_id' => 2,
    'hours' => 6.7,
    'activity_id' => 9,
    'comments' => 'This is a spectacularly important comment',
    'spent_on' => '2012-06-01',
  ));

  // Delete the time entry with id 2
  redmine_rest_api_time_entry_delete(2);


COUNTING ITEMS

// Count the number of issues assigned to user 1
$count = redmine_api_count('issues', array('assigned_to_id' => 6));

// Count the number of time entries
$count = redmine_api_count('time_entries');

