<?php
/**
 * File for the CouchbaseMutateInBuilder class.
 *
 * @author Sergey Avseyev <sergey.avseyev@gmail.com>
 */

/**
 * Represents a builder for subdocument mutation command.
 *
 * Note: This class must be constructed by calling the mutateIn()
 * method of the CouchbaseBucket class.
 *
 * @property integer $id
 *
 * @package Couchbase
 *
 * @see CouchbaseBucket::mutateIn()
 */
class CouchbaseMutateInBuilder {
    /**
     * @var string
     *
     * Unique identifier for the document
     */
    public $id;

    /**
     * @var string
     *
     * Unique and opaque value which identifies current state
     * of the document and regenerated on mutation. Useful to
     * control integrity of the document.
     */
    public $cas;

    /**
     * @var array
     *
     * List of chained commands
     */
    private $commands = array();

    private $bucket;

    public function __construct($bucket, $id, $cas) {
        $this->bucket = $bucket;
        $this->id = $id;
        $this->cas = $cas;
    }

    /**
     * Inserts an element into a JSON document at a given path.
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param string $value An array value, scalar or any other valid JSON item.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function insert($path, $value, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_DICT_ADD,
            'path' => $path,
            'value' => $value,
            'createParents' => $createParents,
        );
        return $this;
    }

    /**
     * Inserts or updates an element within or into a JSON document at a given path.
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param string $value An array value, scalar or any other valid JSON item.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function upsert($path, $value, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_DICT_UPSERT,
            'path' => $path,
            'value' => $value,
            'createParents' => $createParents,
        );
        return $this;
    }

    /**
     * Replaces an element or value within a JSON document at a given path.
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param string $value An array value, scalar or any other valid JSON item.
     */
    public function replace($path, $value) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_REPLACE,
            'path' => $path,
            'value' => $value,
        );
        return $this;
    }

    /**
     * Removes an element or value from a JSON document at a given path.
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     */
    public function remove($path) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_REMOVE,
            'path' => $path,
        );
        return $this;
    }

    /**
     * Add a value to the beginning of an array at given path of a JSON document
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param mixed $value An array value, scalar or any other valid JSON item.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function arrayPrepend($path, $value, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_ADD_FIRST,
            'path' => $path,
            'value' => $value,
            'createParents' => $createParents,
        );
        return $this;
    }

    /**
     * Add a value to the end of an array at given path of a JSON document
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param mixed $value An array value, scalar or any other valid JSON item.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function arrayAppend($path, $value, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_ADD_LAST,
            'path' => $path,
            'value' => $value,
            'createParents' => $createParents,
        );
        return $this;
    }

    /**
     * Insert a value at given path of an array in a JSON document
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param mixed $value An array value, scalar or any other valid JSON item.
     */
    public function arrayInsert($path, $value) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_INSERT,
            'path' => $path,
            'value' => $value,
        );
        return $this;
    }

    /**
     * Add all values to the beginning of an array at given path of a JSON document
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param array $values An array of array values, scalars or any other valid JSON items.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function arrayPrependAll($path, $values, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_ADD_FIRST,
            'path' => $path,
            'value' => $values,
            'createParents' => $createParents,
            'removeBrackets' => true,
        );
        return $this;
    }

    /**
     * Add all values to the end of an array at given path of a JSON document
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param array $values An array of array values, scalars or any other valid JSON items.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function arrayAppendAll($path, $values, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_ADD_LAST,
            'path' => $path,
            'value' => $values,
            'createParents' => $createParents,
            'removeBrackets' => true,
        );
        return $this;
    }

    /**
     * Insert a value at given path of an array in a JSON document
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param array $values An array of array values, scalars or any other valid JSON items.
     */
    public function arrayInsertAll($path, $values) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_INSERT,
            'path' => $path,
            'value' => $values,
            'removeBrackets' => true,
        );
        return $this;
    }

    /**
     * Add a value to an array at given path of a JSON document if it does not exist yet
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param string $value An array value, scalar or any other valid JSON item.
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function arrayAddUnique($path, $value, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_ARRAY_ADD_UNIQUE,
            'path' => $path,
            'value' => $value,
            'createParents' => $createParents,
        );
        return $this;
    }

    /**
     * Perform and arithmetic operation on a numeric value in a JSON document at given path
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     * @param string $delta The value to increment or decrement the original value by
     * @param boolean $createParents If true, the parent will be added to the document.
     */
    public function counter($path, $delta, $createParents = false) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_COUNTER,
            'path' => $path,
            'value' => $delta,
            'createParents' => $createParents,
        );
        return $this;
    }

    public function execute() {
        if (empty($this->commands)) {
            throw new CouchbaseException("There is should be at least command requested.");
        }
        return $this->bucket->_subdoc($this->id, $this->commands, $this->cas);
    }
}
