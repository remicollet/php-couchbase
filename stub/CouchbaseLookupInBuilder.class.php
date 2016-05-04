<?php
/**
 * File for the CouchbaseLookupInBuilder class.
 *
 * @author Sergey Avseyev <sergey.avseyev@gmail.com>
 */

/**
 * Represents a builder for subdocument lookup command.
 *
 * Note: This class must be constructed by calling the lookupIn()
 * method of the CouchbaseBucket class.
 *
 * @property integer $id
 *
 * @package Couchbase
 *
 * @see CouchbaseBucket::lookupIn()
 */
class CouchbaseLookupInBuilder {
    /**
     * @var string
     *
     * Unique identifier for the document
     */
    public $id;

    /**
     * @var array
     *
     * List of chained commands
     */
    private $commands = array();

    private $bucket;

    public function __construct($bucket, $id) {
        $this->bucket = $bucket;
        $this->id = $id;
    }

    /**
     * Gets the value at a specified N1QL path.
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     */
    public function get($path) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_GET,
            'path' => $path,
        );
        return $this;
    }

    /**
     * Checks for the existence of a given N1QL path.
     *
     * @param string $path A string (N1QL syntax) used to specify a location within the document
     */
    public function exists($path) {
        $this->commands[] = array(
            'opcode' => COUCHBASE_SDCMD_EXISTS,
            'path' => $path,
        );
        return $this;
    }

    public function execute() {
        if (empty($this->commands)) {
            throw new CouchbaseException("There is should be at least command requested.");
        }
        return $this->bucket->_subdoc($this->id, $this->commands);
    }
}
