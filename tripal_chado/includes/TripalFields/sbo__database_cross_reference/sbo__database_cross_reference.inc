<?php

class sbo__database_cross_reference extends ChadoField {


  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendent class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------

  // The default lable for this field.
  public static $default_label = 'Cross reference';

  // The default description for this field.
  public static $description = 'This record can be cross referenced with a record in
          another online database. This field is intended for one or more
          references.  At a minimum, the database and accession must be provided.';

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instnace.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings  = array(
    // The short name for the vocabulary (e.g. shcema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'SBO',
    // The name of the term.
    'term_name' => 'Database Cross Reference',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => '0000554',
    // Set to TRUE if the site admin is allowed to change the term
    // type. This will create form elements when editing the field instance
    // to allow the site admin to change the term settings above.
    'term_fixed' => FALSE,
  );

  // The default widget for this field.
  public static $default_widget = 'sbo__database_cross_reference_widget';

  // The default formatter for this field.
  public static $default_formatter = 'sbo__database_cross_reference_formatter';

  // --------------------------------------------------------------------------
  //              PROTECTED CLASS MEMBERS -- DO NOT OVERRIDE
  // --------------------------------------------------------------------------
  // An array containing details about the field. The format of this array
  // is the same as that returned by field_info_fields()
  protected $field;
  // An array containing details about an instance of the field. A field does
  // not have to have an instance.  But if dealing with an instance (such as
  // when using the widgetForm, formatterSettingsForm, etc.) it should be set.
  protected $instance;


  /**
   *
   * @see TripalField::load()
   */
  public function load($entity) {
    $record = $entity->chado_record;

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $record->tablename;

    $schema = chado_get_schema($field_table);
    $pkey = $schema['primary key'][0];
    $fkey_lcolumn = key($schema['foreign keys'][$base_table]['columns']);
    $fkey_rcolumn = $schema['foreign keys'][$base_table]['columns'][$fkey_lcolumn];

    // Set some defaults for the empty record.
    $entity->{$field_name}['und'][0] = array(
      'value' => array(
        /* sub elements that may be present in the values array
        // Database name
        'data:1048' => '',
        // Accession
        'data:2091' => '',
        'schema:url' => '',
        */
      ),
      'chado-' . $field_table . '__' . $pkey => '',
      'chado-' . $field_table . '__' . $fkey_lcolumn => '',
      'chado-' . $field_table . '__dbxref_id' => '',
      'dbxref_id' => '',
      'db_id' => '',
      'accession' => '',
      'version' => '',
      'description' => '',
    );

    $linker_table = $base_table . '_dbxref';
    $options = array('return_array' => 1);
    $record = chado_expand_var($record, 'table', $linker_table, $options);
    if (count($record->$linker_table) > 0) {
      $i = 0;
      foreach ($record->$linker_table as $index => $linker) {
        $dbxref = $linker->dbxref_id;

        // Ignore the GFF_source database. This is a weird thing required by
        // GBrowse and is added by the GFF loader. We don't want to show it.
        if ($dbxref->db_id->name == 'GFF_source') {
          continue;
        }

        $URL = tripal_get_dbxref_url($dbxref);
        $entity->{$field_name}['und'][$i] = array(
          'value' => array(
            'data:1048' => $dbxref->db_id->name,
            'data:2091' => $dbxref->accession,
            'schema:url' => $URL,
          ),
          'chado-' . $field_table . '__' . $pkey => $linker->$pkey,
          'chado-' . $field_table . '__' . $fkey_lcolumn => $linker->$fkey_lcolumn->$fkey_lcolumn,
          'chado-' . $field_table . '__dbxref_id' => $dbxref->dbxref_id,
          'dbxref_id' => $dbxref->dbxref_id,
          'db_id' => $dbxref->db_id->db_id,
          'accession' => $dbxref->accession,
          'version' => $dbxref->version,
          'description' => $dbxref->description,
        );
        $i++;
      }
    }
  }

  /**
   * @see ChadoField::query()
   */
  public function query($query, $condition) {
    $dbxref_linker = $this->instance['settings']['chado_table'];
    $base_table = $this->instance['settings']['base_table'];
    $bschema = chado_get_schema($base_table);
    $bpkey = $bschema['primary key'][0];
    $alias = 'dbx_linker';
    $operator = $condition['operator'];

    if ($condition['column'] == 'database_cross_reference.database_name') {
      $query->join($dbxref_linker, $alias, "base.$bpkey = $alias.$bpkey");
      $query->join('dbxref', 'DBX', "DBX.dbxref_id = $alias.dbxref_id");
      $query->join('db', 'DB', "DB.db_id = DBX.db_id");
      $query->condition("DB.name", $condition['value'], $operator);
    }
    if ($condition['column'] == 'database_cross_reference.accession') {
      $query->join($dbxref_linker, $alias, "base.$bpkey = $alias.$bpkey");
      $query->join('dbxref', 'DBX', "DBX.dbxref_id = $alias.dbxref_id");
      $query->condition("DBX.accession", $condition['value'], $operator);
    }
  }

  /**
   * @see ChadoField::queryOrder()
   */
  public function queryOrder($query, $order) {

  }

  /**
   * @see TripalField::validate()
   */
  public function validate($entity_type, $entity, $field, $items, &$errors) {

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $table_name = $this->instance['settings']['chado_table'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $this->instance['settings']['base_table'];
    $schema = chado_get_schema($table_name);
    $pkey = $schema['primary key'][0];
    $fkeys = array_values($schema['foreign keys'][$base_table]['columns']);
    $fkey = $fkeys[0];


    // Get the field values.
    foreach ($items as $delta => $values) {

      // Get the field values.
      $dbxref_id = $values['chado-' . $field_table . '__dbxref_id'];
      $db_id = $values['db_id'];
      $accession = $values['accession'];
      $version = $values['version'];
      $description = $values['description'];

      // Make sure that if a database ID is provided that an accession is also
      // provided.  Here we use the form_set_error function rather than the
      // form_error function because the form_error will add a red_highlight
      // around all of the fields in the fieldset which is confusing as it's not
      // clear to the user what field is required and which isn't. Therefore,
      // we borrow the code from the 'form_error' function and append the field
      // so that the proper field is highlighted on error.
      if (!$db_id and $accession) {
        $errors[$field_name][$delta]['und'][] = array(
          'message' => t("A database and the accession must both be provided."),
          'error' => 'sbo__database_cross_reference',
        );
      }
      if ($db_id and !$accession) {
        $errors[$field_name][$delta]['und'][] = array(
          'message' => t("A database and the accession must both be provided."),
          'error' => 'sbo__database_cross_reference',
        );
      }
      if (!$db_id and !$accession and ($version or $description)) {
        $errors[$field_name][$delta]['und'][] = array(
          'message' => t("A database and the accession must both be provided."),
          'error' => 'sbo__database_cross_reference',
        );
      }
    }
  }
}