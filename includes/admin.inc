<?php

/**
 * @file
 * Administrative page callbacks for menu_import module.
 */

/**
 * Main form for menu file upload and menu preview.
 */
function menu_import_form($form, &$form_state) {
  if (isset($form_state['storage']['step'])) {
    $step = $form_state['storage']['step'];
  }
  else {
    $step = 1;
  }

  $form = array();
  // Call the step-specific form alters.
  $function = "menu_import_form_step$step";
  if (function_exists($function)) {
    $function($form, $form_state);
  }

  return $form;
}

/**
 * Step 1 of menu import process.
 */
function menu_import_form_step1(&$form, &$form_state) {
  $form['menu_name'] = array(
    '#type' => 'select',
    '#title' => t('Target menu'),
    '#options' => menu_get_menus(),
    '#description' => t('This menu will contain the imported items.'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['menu_name']) ? $form_state['values']['menu_name'] : NULL,
  );

  $form['import_source'] = array(
    '#type' => 'fieldset',
    '#title' => t('Source'),
    '#description' => t('The source file/text must contain menu definition. See !help for format details.',
        array('!help' => l(t('help'), 'admin/help/menu_import'))),
  );

  $form['import_source']['upload'] = array(
    '#type' => 'file',
    '#title' => t('Menu file'),
  );

  $form['import_source']['text_import'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Text import'),
    '#description' => t('Type/paste menu definition here'),
  );

  $form['import_source']['text_import']['text'] = array(
    '#type' => 'textarea',
    '#default_value' => !empty($form_state['values']['text']) ? $form_state['values']['text'] : '',
  );

  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
  );

  $form['options']['link_to_content'] = array(
    '#type' => 'checkbox',
    '#title' => t('Link to existing content'),
    '#description' => t('Look for existing content by path or title.'),
    '#default_value' => !empty($form_state['values']['link_to_content']) ? $form_state['values']['link_to_content'] : 1,
  );

  $form['options']['remove_menu_items'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove existing menu items'),
    '#description' => t('Remove current menu items before running import.'),
    '#default_value' => !empty($form_state['values']['remove_menu_items']) ? $form_state['values']['remove_menu_items'] : 0,
  );

  $form['options']['create_content'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create initial content'),
    '#description' => t('Create initial content if none was found.'),
    '#default_value' => !empty($form_state['values']['create_content']) ? $form_state['values']['create_content'] : 0,
  );

  $langs = language_list();
  // We do not use lambda function here for the sake of 5.2.x support.
  function language_list_get_names($item) {
    return $item->name;
  }
  $lang_opts = array_map('language_list_get_names', $langs);
  $form['options']['language'] = array(
    '#type' => 'select',
    '#title' => t('Default language'),
    '#description' =>
      t('Here you can set the default language of newly created content and/or menu items.'
      . ' If menu item has its own valid language it will override the default one.'),
    '#empty_option' => t('- Not defined -'),
    '#empty_value'  => LANGUAGE_NONE,
    '#options' => $lang_opts,
    '#required' => FALSE,
  );

  $form['node'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content settings'),
    '#attributes' => empty($form_state['values']['create_content'])
      // if validation failed need to check 'input' since no 'values' exist
      && empty($form_state['input']['create_content']) ? array('style' => 'display: none') : NULL
  );

  $node_types = node_type_get_names();

  // Disable content generation if no content types were found.
  if (empty($node_types)) {
    $form['main']['create_content']['#disabled'] = TRUE;
  }

  $form['node']['node_type'] = array(
    '#type' => 'select',
    '#title' => t('Content type'),
    '#options' => $node_types,
    '#description' => t('Type of the newly created content.'),
    '#default_value' => !empty($form_state['values']['node_type']) ? $form_state['values']['node_type'] : NULL,
  );

  // Provide lorem ipsum as default body value.
  $default_body_value =
    'Lorem ipsum dolor sit amet, quis aenean at euismod venenatis '
    . 'interdum ante, placerat nunc metus, velit adipiscing purus mauris, aliquet '
    . 'eros molestie mauris morbi odio egestas, porttitor amet. Suspendisse '
    . 'vestibulum maiores non, nonummy mauris velit, habitasse montes quam eros. '
    . 'Et condimentum lorem adipiscing dolor, in sed libero, tellus purus magnis, '
    . 'nec non bibendum. Erat magnis ut, mauris suspendisse vestibulum dictum tincidunt.';

  $form['node']['node_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Body'),
    '#description' => t('The initial content of the "Body" field. "Filtered HTML" filter will be used.'),
    '#default_value' => !empty($form_state['values']['node_body']) ? $form_state['values']['node_body'] : $default_body_value,
  );

  $formats = filter_formats();
  function filter_formats_get_names($item) {
    return $item->name;
  }
  $filter_opts = array_map('filter_formats_get_names', $formats);
  $form['node']['node_format'] = array(
    '#type' => 'select',
    '#options' => $filter_opts,
    '#title' => t('Body format'),
    '#default_value' => !empty($form_state['values']['node_format']) ? $form_state['values']['node_format'] : 'filtered_html',
  );

  $users = db_query('SELECT uid, name FROM {users} WHERE uid <> 0')->fetchAllKeyed();

  $form['node']['node_author'] = array(
    '#type' => 'select',
    '#title' => t('Authored by'),
    '#options' => $users,
    '#description' => t('Attribute authorship to the selected user.'),
    '#default_value' => !empty($form_state['values']['node_author']) ? $form_state['values']['node_author'] : $GLOBALS['user']->uid
  );

  $form['node']['node_status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#description' => t('Check to automatically publish the created content.'),
    '#default_value' => !empty($form_state['values']['node_status']) ? $form_state['values']['node_status'] : 1,
  );

  $form['node']['node_alias'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create path alias'),
    '#description' => t('Check to use the provided path as alias. The path has to go right after the title in your text file.'),
    '#default_value' => !empty($form_state['values']['node_status']) ? $form_state['values']['node_status'] : 1,
  );

  backdrop_add_js(
    '(function($) {
      $(function() {
        $("input[name=create_content]").change(function() {
          if ($(this).attr("checked")) {
            $("#edit-node").fadeIn();
          }
          else {
            $("#edit-node").fadeOut();
          }
        });
        // Show text input area if it has any contents.
        if ($("textarea[name=text]").val()) {
          $("#edit-text-import a.fieldset-title").click();
        };
      });
    })(jQuery)',
    'inline'
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['sumbit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload & preview'),
    '#submit' => array('menu_import_form_step1_submit'),
    '#validate' => array('menu_import_form_step1_validate'),
  );
}

/**
 * Step 1 form validate handler.
 */
function menu_import_form_step1_validate($form, &$form_state) {
  $node_types = node_type_get_names();
  if ($form_state['values']['create_content'] && empty($node_types)) {
    form_set_error('create_content', t('No content types were found, cannot create initial content.'));
  }

  $validators = array(
    'file_validate_extensions' => array('txt'),
  );

  if (empty($form_state['values']['text'])) {
    $file = file_save_upload('upload', $validators);
    if (!$file) {
      form_set_error('upload', t('You must select a valid text file to upload.'));
    }
    else {
      $form_state['values']['file'] = $file;
    }
  }
}

/**
 * Step 1 form submit handler.
 */
function menu_import_form_step1_submit($form, &$form_state) {
  $form_state['storage']['options'] = array(
    'create_content' => $form_state['values']['create_content'],
    'link_to_content' => $form_state['values']['link_to_content'],
    'remove_menu_items' => $form_state['values']['remove_menu_items'],
    'language' => $form_state['values']['language'],
    'node_type' => $form_state['values']['node_type'],
    'node_body' => $form_state['values']['node_body'],
    'node_format' => $form_state['values']['node_format'],
    'node_author' => $form_state['values']['node_author'],
    'node_status' => $form_state['values']['node_status'],
    'node_alias' => $form_state['values']['node_alias'],
  );

  module_load_include('inc', 'menu_import', 'includes/import');

  $text = $form_state['values']['text'];
  $menu_name = $form_state['values']['menu_name'];
  $options = $form_state['storage']['options'];
  if ($text) {
    $menu = menu_import_parse_menu_from_string(trim($text), $menu_name, $options);
  }
  else {
    $file = $form_state['values']['file'];
    $menu = menu_import_parse_menu_from_file($file->uri, $menu_name, $options);
    $fid = $file->fid;
    file_delete($fid);
  }

  if (!empty($menu['errors'])) {
    foreach ($menu['errors'] as $error) {
      backdrop_set_message(check_plain($error), 'error');
    }
  }

  if (!empty($menu['warnings'])) {
    foreach ($menu['warnings'] as $warn) {
      backdrop_set_message($warn, 'warning');
    }
  }

  $form_state['storage']['menu'] = $menu;
  $form_state['storage']['step'] = 2;
  $form_state['rebuild'] = TRUE;
  $form_state['page_values'][1] = $form_state['values'];
}

/**
 * Step 2.
 */
function menu_import_form_step2(&$form, &$form_state) {
  $menu = $form_state['storage']['menu'];
  $options = $form_state['storage']['options'];

  $form['menu'] = menu_import_preview($menu, $options);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => array('menu_import_form_back_submit'),
    '#weight' => -1,
    '#limit_validation_errors' => array(),
  );
  $form['actions']['sumbit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array('menu_import_form_step2_submit'),
  );
}

/**
 * Step 2 form submit handler.
 */
function menu_import_form_step2_submit($form, &$form_state) {
  $menu = $form_state['storage']['menu'];
  $options = $form_state['storage']['options'];
  $menu_name = $menu[0]['menu_name'];

  module_load_include('inc', 'menu_import', 'includes/import');
  $result = menu_import_save_menu($menu, $options);
  menu_import_display_result($result);

  $form_state['redirect'] = 'admin/structure/menu/manage/' . $menu_name;
}

/**
 * Helper function to display result.
 */
function menu_import_display_result(array $result) {
  $msgs = menu_import_get_messages();

  $total_items = $result['new_nodes'] + $result['matched_nodes'] + $result['external_links'] + $result['unknown_links'];
  backdrop_set_message(t($msgs['items_imported'], array('@count' => $total_items)));

  if (empty($result['failed'])) {
    unset($result['failed']);
  }

  foreach ($result as $type => $value) {
    $msg_type = $type == 'failed' ? 'error' : 'status';
    backdrop_set_message(t($msgs[$type], array('@count' => $value)), $msg_type);
  }
}

/**
 * Back button submit handler.
 */
function menu_import_form_back_submit($form, &$form_state) {
  // We have to re-render the form on Back action.
  $form_state['no_cache'] = TRUE;

  $form_state['rebuild'] = TRUE;
  $form_state['values'] = $form_state['page_values'][1];
  $form_state['storage']['step'] = 1;
}

/**
 * Renders the menu for preview.
 *
 * @param $menu
 *   The whole menu structure.
 * @param $options
 *   Array of import options.
 *
 * @return
 *   Array understandable by Backdrop's theme engine.
 */
function menu_import_preview($menu, array $options) {
  $menu_name = $menu[0]['menu_name'];
  $menus = menu_get_menus();

  $descriptions = array();

  if ($options['remove_menu_items']) {
    $descriptions[] = t('Current menu items will be removed');
  }

  $descriptions[] = t('Import @count items into "@menu_title" menu (@menu_name)',
    array(
      '@count' => count($menu) - 3, // Minus errors/warnings and root item.
      '@menu_title' => $menus[$menu_name],
      '@menu_name' => $menu_name
    ));

  if ($options['link_to_content']) {
    $descriptions[] = t('Link to existing content if found');
  }

  if ($options['create_content']) {
    $descriptions[] = t('Initial content of type "@type" will be created', array('@type' => $options['node_type']));

    $author = user_load($options['node_author']);
    $descriptions[] = t('Content author: @author', array('@author' => $author->name));

    $status = $options['node_status'] ? t('published') : t('not published');
    $descriptions[] = t('Content status: @status', array('@status' => $status));

    if ($options['node_alias']) {
      $descriptions[] = t('Path aliases will be created.');
    }
  }

  $descriptions[] = t('Language: @lang', array('@lang' => $options['language'] == LANGUAGE_NONE ? t('not defined') : $options['language']));

  $rows = array();
  $depth = 0;
  foreach ($menu[0]['children'] as $index) {
    $new_rows = _menu_import_preview_row_recurse($menu, $index, $depth, $options);
    foreach ($new_rows as $row) {
      $rows[] = $row;
    }
  }

  $build['options'] = array(
    '#theme' => 'item_list',
    '#items' => $descriptions,
    '#title' => t('Import options'),
    '#type' => 'ul',
  );

  $header = array(t('Menu Item'), t('Content exists'), t('Path'), t('System path'));

  $header[] = t('Weight');

  $build['menu'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#caption' => t('Menu preview'),
  );
  return $build;
}

/**
 * Recursive function for theming a menu item and its children.
 */
function _menu_import_preview_row_recurse(&$menu, $index, $level, $options) {
  $rows = array();

  $title  = theme('indentation', array('size' => $level));
  $title  .= $menu[$index]['link_title'];
  $exists = $menu[$index]['nid'] ? t('Yes') : t('No');
  $weight = $menu[$index]['weight'];
  $path       = !empty($menu[$index]['path']) ? $menu[$index]['path'] : ' - ';
  $link_path  = !empty($menu[$index]['link_path']) ? $menu[$index]['link_path'] : ' - ';
  $description= isset($menu[$index]['description']) ? $menu[$index]['description'] : '';

  $row_data = array(
    array('data' => '<span title="' . check_plain($description) . '">' . $title . '</span>'),
    array('data' => $exists),
    array('data' => check_url($path)),
    array('data' => check_url($link_path))
  );
  $row_data[] = array('data' => $weight);

  $rows[] = array(
    'data' => $row_data
  );

  foreach ($menu[$index]['children'] as $child) {
    $new_rows = _menu_import_preview_row_recurse($menu, $child, ($level +1), $options);
    foreach ($new_rows as $row) {
      $rows[] = $row;
    }
  }
  return $rows;
}
