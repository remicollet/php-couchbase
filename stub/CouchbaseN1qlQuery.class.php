<?php
/**
 * File for the CouchbaseN1qlQuery class.
 */

/**
 * Represents a N1QL query to be executed against a Couchbase bucket.
 *
 * @package Couchbase
 */
class CouchbaseN1qlQuery {
    /**
     * @var string
     * @internal
     */
    public $options = array();

    const NOT_BOUNDED = 1;
    const REQUEST_PLUS = 2;
    const STATEMENT_PLUS = 3;

    /**
     * Creates a new N1qlQuery instance directly from a N1QL DML.
     * @param $str
     * @return CouchbaseN1qlQuery
     */
    static public function fromString($str) {
        $res = new CouchbaseN1qlQuery();
        $res->options['statement'] = $str;
        $res->adhoc = true;
        return $res;
    }

    /**
     * Supply positional parameters for query.
     *
     * <code>
     * $query = CouchbaseN1qlQuery::fromString('SELECT * FROM `travel-sample` WHERE city=$1 LIMIT $2');
     * $query->positionalParams(array('New York', 3));
     * </code>
     *
     * @param array $params
     */
    public function positionalParams($params) {
        $this->options['args'] = $params;
        return $this;
    }

    /**
     * Supply named parameters for query.
     *
     * <code>
     * $query = CouchbaseN1qlQuery::fromString('SELECT * FROM `travel-sample` WHERE city=$city LIMIT $limit');
     * $query->namedParams(array('city' => 'New York', 'limit' => 3));
     * </code>
     *
     * @param array $params
     */
    public function namedParams($params) {
        foreach ($params as $key => $value) {
            $this->options['$' . $key] = $value;
        }
        return $this;
    }

    /**
     * Specify the consistency level for this query.
     *
     * @param $consistency
     * @return $this
     * @throws CouchbaseN1qlQuery
     */
    public function consistency($consistency) {
        if ($consistency == self::NOT_BOUNDED) {
            $this->options['scan_consistency'] = 'not_bounded';
        } else if ($consistency == self::REQUEST_PLUS) {
            $this->options['scan_consistency'] = 'request_plus';
        } else if ($consistency == self::STATEMENT_PLUS) {
            $this->options['scan_consistency'] = 'statement_plus';
        } else {
            throw new CouchbaseException('invalid option passed.');
        }
        return $this;
    }

    public function consistentWith($mutationState) {
        $this->options['scan_consistency'] = 'at_plus';
        $this->options['scan_vectors'] = $mutationState->exportForN1ql();
        return $this;
    }

    /**
     * Specify whether this query is a one-time query, or if it
     *   if it should be prepared.
     *
     * @param $adhoc
     * @return $this
     * @throws CouchbaseN1qlQuery
     */
    public function adhoc($adhoc) {
        $this->adhoc = !!$adhoc;
        return $this;
    }

    /**
     * Generates the N1QL object as it will be passed to the server.
     *
     * @return object
     */
    public function toObject() {
        return $this->options;
    }

    /**
     * Returns the string representation of this N1ql query (the statement).
     *
     * @return string
     */
    public function toString() {
        return $this->options['statement'];
    }
}
