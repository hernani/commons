<?php

/**
 * @file
 *   All block-related functions of commons_core
 */

/**
 * Declare a list of all the available blocks
 */
function _commons_core_block_list() {
  $blocks = array();
  $blocks['user_statistics'] = array(
    'info' => t('User statistics'),
    'cache' => BLOCK_CACHE_GLOBAL,
  );
  $blocks['commons_info'] = array(
    'info' => t('Drupal Commons information'),
    'cache' => BLOCK_CACHE_GLOBAL,
  );
  $blocks['header_login'] = array(
    'info' => t('Header login block'),
    'cache' => BLOCK_CACHE_GLOBAL,
  );
  $blocks['join_learn'] = array(
    'info' => t('Join & learn more'),
    'cache' => BLOCK_CACHE_GLOBAL,
  );
  $blocks['group_menu'] = array(
    'info' => t('Group actions menu'),
    'cache' => BLOCK_CACHE_GLOBAL,
  );
  $blocks['group_create_content'] = array(
    'info' => t('Group create contenet selector'),
    'cache' => BLOCK_CACHE_GLOBAL,
  );
  return $blocks;
}

/**
 * Display a block with global user statistics
 */
function _commons_core_user_statistics_block() {
  $data = array();
  
  // Determine total users on the site
  $sql = "SELECT COUNT(uid) FROM {users} WHERE status = 1";
  $total = db_result(db_query($sql));
  $data[] = format_plural($total, '1 user', '@count users');
  
  // Determine recently joined users
  $sql = "SELECT COUNT(uid) FROM {users} WHERE status = 1 AND created > %d";
  $total = db_result(db_query($sql, strtotime('-' . variable_get('commons_recent_join_interval', '3 months'), time())));
  $data[] = format_plural($total, '1 recently joined member', '@count recently joined members');
  
  // Build the block
  $block = array(
    'subject' => t('User statistics'),
    'content' => theme('commons_core_user_stats_block', $data), 
  );
  
  return $block;
}

/**
 * Display the Commons information block
 */
function _commons_core_commons_info_block() {
  return array(
    'subject' => '',
    'content' => theme('commons_core_info_block'),
  );
}

/**
 * Display our custom login block for the header
 */
function _commons_core_header_login_block() {
  global $user;
  
  // Only anonymous users can log in
  if ($user->uid) {
    return NULL;
  }
  
  // No login needed if we're on the login page
  if (arg(0) == 'user') {
    return NULL;
  }
  
  // Determine if we're on the front page
  if (drupal_is_front_page()) {
    // Only provide a link
    return t('Already a member? !login', array('!login' => l(t('Login'), 'user')));
  }
  
  // Build the block
  $block = array(
    'subject' => '',
    'content' => drupal_get_form('user_login_block'), 
  );
  
  return $block;
}

/**
 * Display the "Join & learn more" registration block
 */
function _commons_core_join_learn_block() {
  // Check access
  if (!user_register_access()) {
    return NULL;
  }
  
  // Build the block
  $block = array(
    'subject' => '',
    'content' => drupal_get_form('commons_core_join_form'), 
  );
  
  return $block;
}

/**
 * Configure the "Join & learn more" block
 */
function _commons_core_join_learn_block_configure() {
  $form['commons_recent_join_interval'] = array(
    '#type' => 'select',
    '#title' => t('Recent joins time-frame'),
    '#default_value' => variable_get('commons_recent_join_interval', t('3 months')),
    '#options' => array(
      '3 days' => t('3 days'),
      '1 week' => t('1 week'),
      '2 weeks' => t('2 weeks'),
      '1 month' => t('1 month'),
      '3 months' => t('3 months'),
      '6 months' => t('6 months'),
    ),
    '#description' => t('The !user_directory contains a block that indicates the amount of recently joined members. Select the time-frame to be used to calculate this value.', array('!user_directory' => l(t('user directory'), 'users'))),
  );
  return $form;
}

/**
 * Save the "Join & learn more" block
 */
function _commons_core_join_learn_block_save() {
  variable_set('commons_recent_join_interval', $edit['commons_recent_join_interval']);
}

/**
 * Display the customized group actions menu block
 */
function _commons_core_group_menu_block() {
  // Detect group context
  if ($group = commons_core_get_group_context()) {
    // Get the standard group details block
    $details = og_block_details();
    if (isset($details['content'])) {
      return array(
        'subject' => l(t('Actions'), "node/{$group->nid}", array('attributes' => array('title' => t('Return to !group', array('!group' => $group->title))))),
        'content' => $details['content']
      );
    }
  }
  
  return NULL;
}

/**
 * Display the dropdown group create content selector
 */
function _commons_core_group_create_content_block() {
  if ($group = commons_core_get_group_context()) {
    if ($form = drupal_get_form('commons_core_group_create_content_block_form')) {
      return array(
        'subject' => '',
        'content' => $form,
      );
    }
  }
  
  return NULL;
}

/**
 * Provide the form for the group create content selector
 */
function commons_core_group_create_content_block_form(&$form_state) {
  $form = array();
  
  if (!($group = commons_core_get_group_context())) {
    return NULL;
  }
  
  // Gather group node type links
  $links = module_invoke_all('og_create_links', $group);
  
  // Allow og_features to alter them
  // We avoid allowing any module to alter them, because the alter 
  // hook is often used to add non-node links
  og_features_og_links_alter($links);
  
  // Turn the links into node types
  $types = array();
  foreach($links as $key => $link) {
    // Extract the node type
    if ($type = str_replace('create_', '', $key)) {
      // Load the node type
      if ($type = node_get_types('type', $type)) {
        // Add the type
        $types[$type->type] = $type->name;
      }
    }
  }
  
  // Make sure we have at least one type
  if (empty($types)) {
    return NULL;
  }
  
  // Provide the selector
  $form['node_type'] = array(
    '#type' => 'select',
    '#options' => $types,
  );
  
  // Store the group
  $form['group'] = array(
    '#type' => 'value',
    '#value' => $group,
  );
  
  // Submit button
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  
  return $form;
}

/**
 * Submit handler for the group create content selector
 */
function commons_core_group_create_content_block_form_submit(&$form, $form_state) {
  // Extract the group
  $group = $form_state['values']['group'];
  
  // Extract the node type
  $type = $form_state['values']['node_type'];
  
  // Redirect to the node form
  drupal_goto("node/add/{$type}", array('gids' => array($group->nid)));
}