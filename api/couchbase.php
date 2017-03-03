<?php

/**
 * INI entries:
 *
 * couchbase.log_level (string), default: "WARN"
 *   controls amount of information, the module will send to PHP error log. Accepts the following values in order of
 *   increasing verbosity: "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE".
 *
 * couchbase.encoder.format (string), default: "json"
 *   selects serialization format for default encoder (\Couchbase\defaultEncoder). Accepts the following values:
 *   "json" - encodes objects and arrays as JSON object (using json_encode()), primitives written in stringified form,
 *   which is allowed for most of the JSON parsers as valid values. For empty arrays JSON array preferred, if it is
 *   necessary, use "new stdClass()" to persist empty JSON object. Note, that only JSON format considered supported by
 *   all Couchbase SDKs, everything else is private implementation (i.e. "php" format won't be readable by .NET SDK).
 *   "php" - uses PHP serialize() method to encode the document.
 *   "igbinary" - uses pecl/igbinary to encode the document in even more efficient than "php" form. Might not be
 *   available, if the Couchbase PHP SDK didn't find it during build phase, in this case constant
 *   \Couchbase\HAVE_IGBINARY will be false.
 *
 * couchbase.encoder.compression (string), default: "off"
 *   selects compression algorithm. Also see related compression options below. Accepts the following values:
 *   "fastlz" - uses FastLZ algorithm. The module might be configured to use system fastlz library during build,
 *   othewise vendored version will be used. This algorithm is always available.
 *   "zlib" - uses compression implemented by libz. Might not be available, if the system didn't have libz headers
 *   during build phase. In this case \Couchbase\HAVE_ZLIB will be false.
 *   "off" or "none" - compression will be disabled, but the library will still read compressed values.
 *
 * couchbase.encoder.compression_threshold (long), default: 0
 *   controls minimum size of the document value in bytes to use compression. For example, if threshold 100 bytes,
 *   and the document size is 50, compression will be disabled for this particular document.
 *
 * couchbase.encoder.compression_factor (float), default: 0.0
 *   controls the minimum ratio of the result value and original document value to proceed with persisting compressed
 *   bytes. For example, the original document consists of 100 bytes. In this case factor 1.0 will require compressor
 *   to yield values not larger than 100 bytes (100/1.0), and 1.5 -- not larger than 66 bytes (100/1.5).
 *
 * couchbase.decoder.json_arrays (boolean), default: false
 *   controls the form of the documents, returned by the server if they were in JSON format. When true, it will generate
 *   arrays of arrays, otherwise instances of sdtClass.
 */

namespace Couchbase {

    /* If igbinary extension was not found during build phase this constant will store 0 */
    define("HAVE_IGBINARY", 1);
    /* If libz headers was not found during build phase this constant will store 0 */
    define("HAVE_ZLIB", 1);

    /**
     * @param string $data
     * @return string
     */
    function fastlzCompress(string $data) {}

    /**
     * @param string $data
     * @return string
     */
    function fastlzDecompress(string $data) {}

    /**
     * @param string $data
     * @return string
     */
    function zlibCompress(string $data) {}

    /**
     * @param string $data
     * @return string
     */
    function zlibDecompress(string $data) {}

    /**
     * @param string $bytes
     * @param int $flags
     * @param int $datatype
     * @return mixed
     */
    function passthruDecoder(string $bytes, int $flags, int $datatype) {}

    /**
     * @param mixed $value
     * @return array [bytes, flags, datatype]
     */
    function passthruEncoder(mixed $value) {}

    /**
     * @param string $bytes
     * @param int $flags
     * @param int $datatype
     * @return mixed
     */
    function defaultDecoder(string $bytes, int $flags, int $datatype) {}

    /**
     * @param mixed $value
     * @return array [bytes, flags, datatype]
     */
    function defaultEncoder(mixed $value) {}

    /**
     * @param string $bytes
     * @param int $flags
     * @param int $datatype
     * @param array $options
     * @return mixed
     */
    function basicDecoderV1(string $bytes, int $flags, int $datatype, array $options) {}

    /**
     * @param mixed $value
     * @param array $options
     * @return array [bytes, flags, datatype]
     */
    function basicEncoderV1(mixed $value, array $options) {}

    class Exception extends \Exception {
    }

    class Document {
        /**
         * @var Exception
         */
        public $error;

        /**
         * @var mixed
         */
        public $value;

        /**
         * @var int
         */
        public $flags;

        /**
         * @var string
         */
        public $cas;

        /**
         * @var MutationToken
         */
        public $token;
    }

    class DocumentFragment {
        /**
         * @var Exception
         */
        public $error;

        /**
         * @var mixed
         */
        public $value;

        /**
         * @var string
         */
        public $cas;

        /**
         * @var MutationToken
         */
        public $token;
    }

    final class Cluster {
        /**
         * @param string $connstr
         */
        final public function __construct(string $connstr) {}

        /**
         * @param string $name
         * @param string $password
         * @return Bucket
         */
        final public function openBucket(string $name = "default", string $password = "") {}

        /**
         * @param string $username
         * @param string $password
         * @return ClusterManager
         */
        final public function manager(string $username = null, string $password = null) {}

        /**
         * @param Authenticator $authenticator
         * @return null
         */
        final public function authenticate(Authenticator $authenticator) {}
    }

    final class ClusterManager {
        final private function __construct() {}

        final public function listBuckets() {}

        /**
         * @param string $name
         * @param array $options
         */
        final public function createBucket(string $name, array $options = []) {}

        /**
         * @param string $name
         */
        final public function removeBucket(string $name) {}

        final public function info() {}
    }

    final class Bucket {
        final private function __construct() {}

        /**
         * @param $name Supported properties:
         *                * operationTimeout
         *                * viewTimeout
         *                * durabilityInterval
         *                * durabilityTimeout
         *                * httpTimeout
         *                * configTimeout
         *                * configDelay
         *                * configNodeTimeout
         *                * htconfigIdleTimeout
         * @return int
         */
        final private function __get(string $name) {}

        /**
         * @param $name Supported properties:
         *                * operationTimeout
         *                * viewTimeout
         *                * durabilityInterval
         *                * durabilityTimeout
         *                * httpTimeout
         *                * configTimeout
         *                * configDelay
         *                * configNodeTimeout
         *                * htconfigIdleTimeout
         * @return int
         */
        final private function __set(string $name, int $value) {}

        /**
         * @param callable $encoder
         * @param callable $decoder
         */
        final public function setTranscoder(callable $encoder, callable $decoder) {}

        /**
         * @param string|array $ids
         * @param array $options
         */
        final public function get(mixed $ids, array $options = []) {}

        /**
         * @param string|array $ids
         * @param int $lockTime
         * @param array $options
         */
        final public function getAndLock(mixed $ids, int $lockTime, array $options = []) {}

        /**
         * @param string $id
         * @param mixed $value
         * @param array $options
         */
        final public function upsert(string $id, mixed $value, array $options = []) {}

        /**
         * @param string $id
         * @param mixed $value
         * @param array $options
         */
        final public function insert(string $id, mixed $value, array $options = []) {}

        /**
         * @param string $id
         * @param mixed $value
         * @param array $options
         */
        final public function replace(string $id, mixed $value, array $options = []) {}

        /**
         * @param string $id
         * @param mixed $value
         * @param array $options
         */
        final public function append(string $id, mixed $value, array $options = []) {}

        /**
         * @param string $id
         * @param mixed $value
         * @param array $options
         */
        final public function prepend(string $id, mixed $value, array $options = []) {}

        /**
         * @param string $id
         * @param array $options
         */
        final public function remove(string $id, array $options = []) {}

        /**
         * @param string $id
         * @param int $expiry
         * @param array $options
         */
        final public function touch(string $id, int $expiry, array $options = []) {}

        /**
         * @param string $id
         * @param int $delta
         * @param array $options
         */
        final public function counter(string $id, int $delta = 1, array $options = []) {}

        /**
         * @param string $id
         * @return LookupInBuilder
         */
        final public function lookupIn(string $id) {}

        /**
         * @param string $id
         * @param string ...$paths
         * @return array
         */
        final public function retrieveIn(string $id, string ...$paths) {}

        /**
         * @param string $id
         * @param string $cas
         * @return MutateInBuilder
         */
        final public function mutateIn(string $id, string $cas) {}

        /**
         * @return BucketManager
         */
        final public function manager() {}

        /**
         * @param N1qlQuery|ViewQuery|SpatialViewQuery|SearchQuery $query
         * @return object
         */
        final public function query(object $query, boolean $jsonAsArray = false) {}

        /**
         * @param string $id
         * @return int
         */
        final public function mapSize(string $id) {}

        /**
         * @param string $id
         * @param string $key
         * @param mixed $value
         */
        final public function mapAdd(string $id, string $key, mixed $value) {}

        /**
         * @param string $id
         * @param string $key
         */
        final public function mapRemove(string $id, string $key) {}

        /**
         * @param string $id
         * @param string $key
         */
        final public function mapGet(string $id, string $key) {}

        /**
         * @param string $id
         * @return int
         */
        final public function setSize(string $id) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function setAdd(string $id, mixed $value) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function setExists(string $id, mixed $value) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function setRemove(string $id, mixed $value) {}

        /**
         * @param string $id
         */
        final public function listSize(string $id) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function listPush(string $id, mixed $value) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function listShift(string $id, mixed $value) {}

        /**
         * @param string $id
         * @param int $index
         */
        final public function listRemove(string $id, int $index) {}

        /**
         * @param string $id
         * @param int $index
         */
        final public function listGet(string $id, int $index) {}

        /**
         * @param string $id
         * @param int $index
         * @param mixed $value
         */
        final public function listSet(string $id, int $index, mixed $value) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function listExists(string $id, mixed $value) {}

        /**
         * @param string $id
         */
        final public function queueSize(string $id) {}

        /**
         * @param string $id
         * @param mixed $value
         */
        final public function queueExists(string $id, mixed $value) {}

        /**
         * @param string $id
         */
        final public function queueRemove(string $id) {}
    }

    final class BucketManager {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function info() {}

        final public function flush() {}

        /**
         * @return array
         */
        final public function listDesignDocuments() {}

        /**
         * @param string $name
         * @return array
         */
        final public function getDesignDocument(string $name) {}

        /**
         * @param string $name
         */
        final public function removeDesignDocument(string $name) {}

        /**
         * @param string $name
         * @param array $document
         */
        final public function upsertDesignDocument(string $name, array $document) {}

        /**
         * @param string $name
         * @param array $document
         */
        final public function insertDesignDocument(string $name, array $document) {}

        /**
         * @return array
         */
        final public function listN1qlIndexes() {}

        /**
         * @param string $customName
         * @param boolean $ignoreIfExist
         * @param boolean $defer
         */
        final public function createN1qlPrimaryIndex(string $customName = '', boolean $ignoreIfExist = false, boolean $defer = false) {}

        /**
         * @param string $name
         * @param array $fields
         * @param string $whereClause
         * @param boolean $ignoreIfExist
         * @param boolean $defer
         */
        final public function createN1qlIndex(string $name, array $fields, string $whereClause = '', boolean $ignoreIfExist = false, boolean $defer = false) {}

        /**
         * @param string $customName
         * @param boolean $ignoreIfNotExist
         * @param boolean $defer
         */
        final public function dropN1qlPrimaryIndex(string $customName = '', boolean $ignoreIfNotExist = false, boolean $defer = false) {}

        /**
         * @param string $name
         * @param boolean $ignoreIfNotExist
         * @param boolean $defer
         */
        final public function dropN1qlPrimaryIndex(string $name, boolean $ignoreIfNotExist = false, boolean $defer = false) {}
    }

    interface Authenticator {}

    final class ClassicAuthenticator implements Authenticator {
        /**
         * @param string $username
         * @param string $paassword
         */
        final public function cluster(string $username, string $password) {}

        /**
         * @param string $name
         * @param string $paassword
         */
        final public function bucket(string $name, string $password) {}
    }

    final class MutationToken {
        final private function __construct() {}

        /**
         * @param string $bucketName
         * @param int vbucketId
         * @param string vbucketUuid
         * @param string sequenceNumber
         */
        final public static function from(string $bucketName, int $vbucketId, string $vbucketUuid, string $sequenceNumber) {}

        /**
         * @return string
         */
        final public function bucketName() {}

        /**
         * @return int
         */
        final public function vbucketId() {}

        /**
         * @return string
         */
        final public function vbucketUuid() {}

        /**
         * @return string
         */
        final public function sequenceNumber() {}
    }

    final class MutationState {
        final private function __construct() {}

        /**
         * @param array|Document|DocumentFragment $source
         */
        final public static function from(mixed $source) {}

        /**
         * @param array|Document|DocumentFragment $source
         */
        final public function add(mixed $source) {}
    }

    interface ViewQueryEncodable {}

    final class ViewQuery implements ViewQueryEncodable {
        define("UPDATE_BEFORE", 1);
        define("UPDATE_NONE", 2);
        define("UPDATE_AFTER", 3);

        define("ORDER_ASCENDING", 1);
        define("ORDER_DESCENDING", 2);

        final private function __construct() {}

        /**
         * @param string $designDocumentName
         * @param string $viewName
         * @return ViewQuery
         */
        final public static function from(string $designDocumentName, string $viewName) {}

        /**
         * @param string $designDocumentName
         * @param string $viewName
         * @return SpatialViewQuery
         */
        final public static function fromSpatial(string $designDocumentName, string $viewName) {}

        /**
         * @param int $limit
         * @return ViewQuery
         */
        final public function limit(int $limit) {}

        /**
         * @param int $skip
         * @return ViewQuery
         */
        final public function skip(int $skip) {}

        /**
         * @param int $consistency
         *     Possible values are:
         *       - UPDATE_BEFORE
         *       - UPDATE_NONE
         *       - UPDATE_AFTER
         * @return ViewQuery
         */
        final public function consistency(int $consistency) {}

        /**
         * @param int $order
         *     Possible values are:
         *       - ORDER_ASCENDING
         *       - ORDER_DESCENDING
         * @return ViewQuery
         */
        final public function order(int $order) {}

        /**
         * @param boolean $reduce
         * @return ViewQuery
         */
        final public function reduce(boolean $reduce) {}

        /**
         * @param boolean $group
         * @return ViewQuery
         */
        final public function group(boolean $group) {}

        /**
         * @param int $groupLevel
         * @return ViewQuery
         */
        final public function groupLevel(int $groupLevel) {}

        /**
         * @param mixed $key
         * @return ViewQuery
         */
        final public function key(mixed $key) {}

        /**
         * @param array $keys
         * @return ViewQuery
         */
        final public function keys(array $keys) {}

        /**
         * @param mixed $startKey
         * @param mixed $endKey
         * @param boolean $inclusiveEnd
         * @return ViewQuery
         */
        final public function range(mixed $startKey, mixed $endKey, boolean $inclusiveEnd = false) {}

        /**
         * @param string $startKeyDocumentId
         * @param string $endKeyDocumentId
         * @return ViewQuery
         */
        final public function idRange(string $startKeyDocumentId, string $endKeyDocumentId) {}

        /**
         * @param array $custom
         * @return ViewQuery
         */
        final public function custom(array $customParameters) {}
    }

    final class SpatialViewQuery implements ViewQueryEncodable {
        final private function __construct() {}

        /**
         * @param int $limit
         * @return SpatialViewQuery
         */
        final public function limit(int $limit) {}

        /**
         * @param int $skip
         * @return SpatialViewQuery
         */
        final public function skip(int $skip) {}

        /**
         * @param int $consistency
         *     Possible values are:
         *       - UPDATE_BEFORE
         *       - UPDATE_NONE
         *       - UPDATE_AFTER
         * @return SpatialViewQuery
         */
        final public function consistency(int $consistency) {}

        /**
         * @param int $order
         *     Possible values are:
         *       - ORDER_ASCENDING
         *       - ORDER_DESCENDING
         * @return SpatialViewQuery
         */
        final public function order(int $order) {}

        /**
         * @param array $bbox
         * @return SpatialViewQuery
         */
        final public function bbox(boolean $bbox) {}

        /**
         * @param array $range
         * @return SpatialViewQuery
         */
        final public function startRange(array $range) {}

        /**
         * @param array $range
         * @return SpatialViewQuery
         */
        final public function endRange(array $range) {}

        /**
         * @param array $custom
         * @return SpatialViewQuery
         */
        final public function custom(array $customParameters) {}
    }

    final class N1qlQuery {
        define("NOT_BOUNDED", 1);
        define("REQUEST_PLUS", 2);
        define("STATEMENT_PLUS", 3);

        final private function __construct() {}

        /**
         * @param string $statement
         * @return N1qlQuery
         */
        final public static function from(string $statement) {}

        /**
         * @param boolean $adhoc
         * @return N1qlQuery
         */
        final public function adhoc(boolean $adhoc) {}

        /**
         * @param boolean $crossBucket
         * @return N1qlQuery
         */
        final public function crossBucket(boolean $crossBucket) {}

        /**
         * @param array $params
         * @return N1qlQuery
         */
        final public function positionalParams(array $params) {}

        /**
         * @param array $params
         * @return N1qlQuery
         */
        final public function namedParams(array $params) {}

        /**
         * @param int $consistency
         *      Possible values are:
         *        - NOT_BOUNDED
         *        - REQUEST_PLUS
         *        - STATEMENT_PLUS
         * @return N1qlQuery
         */
        final public function consistency(int $consistency) {}

        /**
         * @param MutationState $consistentWith
         * @return N1qlQuery
         */
        final public function consistentWith(MutationState $state) {}
    }

    final class N1qlIndex {
        define("DEFAULT", 0);
        define("GSI", 1);
        define("VIEW", 2);

        final private function __construct() {}

        /**
         * @var string
         */
        public $name;

        /**
         * @var boolean
         */
        public $isPrimary;

        /**
         * @var int
         *      Possible values are:
         *        - DEFAULT
         *        - GSI
         *        - VIEW
         */
        public $type;

        /**
         * @var string
         */
        public $state;

        /**
         * @var string
         */
        public $keyspace;

        /**
         * @var string
         */
        public $namespace;

        /**
         * @var array
         */
        public $fields;

        /**
         * @var string
         */
        public $condition;
    }

    final class LookupInBuilder {
        final private function __construct() {}

        /**
         * @param string $path
         * @return LookupInBuilder
         */
        final public function get(string $path) {}

        /**
         * @param string $path
         * @return LookupInBuilder
         */
        final public function exists(string $path) {}

        /**
         * @return DocumentFragment
         */
        final public function execute() {}
    }

    final class MutateInBuilder {
        final private function __construct() {}

        /**
         * @param string $path
         * @param mixed $value
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function insert(string $path, mixed $value, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param mixed $value
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function upsert(string $path, mixed $value, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param mixed $value
         * @return MutateInBuilder
         */
        final public function replace(string $path, mixed $value) {}

        /**
         * @param string $path
         * @return MutateInBuilder
         */
        final public function remove(string $path) {}

        /**
         * @param string $path
         * @param mixed $value
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function arrayPrepend(string $path, mixed $value, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param array $values
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function arrayPrependAll(string $path, array $values, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param mixed $value
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function arrayAppend(string $path, mixed $value, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param array $values
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function arrayAppendAll(string $path, array $values, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param mixed $value
         * @return MutateInBuilder
         */
        final public function arrayInsert(string $path, mixed $value) {}

        /**
         * @param string $path
         * @param array $values
         * @return MutateInBuilder
         */
        final public function arrayInsertAll(string $path, array $values, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param mixed $value
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function arrayAddUnique(string $path, mixed $value, boolean $createParents = false) {}

        /**
         * @param string $path
         * @param int $delta
         * @param boolean $createParents
         * @return MutateInBuilder
         */
        final public function counter(string $path, int $delta, boolean $createParents = false) {}

        /**
         * @return DocumentFragment
         */
        final public function execute() {}
    }

    final class SearchQuery implements JsonSerializable {
        define("HIGHLIGHT_HTML", 'html');
        define("HIGHLIGHT_ANSI", 'ansi');
        define("HIGHLIGHT_SIMPL", 'simple');

        /**
         * @return BooleanSearchQuery
         */
        final public static function boolean() {}

        /**
         * @return DateRangeSearchQuery
         */
        final public static function dateRange() {}

        /**
         * @param boolean $value
         * @return BooleanFieldSearchQuery
         */
        final public static function booleanField(boolean $value) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return ConjunctionSearchQuery
         */
        final public static function conjuncts(SearchQueryPart ...$queries) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return DisjunctionSearchQuery
         */
        final public static function disjuncts(SearchQueryPart ...$queries) {}

        /**
         * @param string ...$documentIds
         * @return DocIdSearchQuery
         */
        final public static function docId(SearchQueryPart ...$documentIds) {}

        /**
         * @param string $match
         * @return MatchSearchQuery
         */
        final public static function match(string $match) {}

        /**
         * @return MatchAllSearchQuery
         */
        final public static function matchAll() {}

        /**
         * @return MatchNoneSearchQuery
         */
        final public static function matchNone() {}

        /**
         * @param string ...$terms
         * @return MatchPhraseSearchQuery
         */
        final public static function matchPhrase(string ...$terms) {}

        /**
         * @param string $prefix
         * @return PrefixSearchQuery
         */
        final public static function prefix(string $prefix) {}

        /**
         * @param string $queryString
         * @return QueryStringSearchQuery
         */
        final public static function queryString(string $queryString) {}

        /**
         * @param string $regexp
         * @return RegexpSearchQuery
         */
        final public static function regexp(string $regexp) {}

        /**
         * @param string $term
         * @return TermSearchQuery
         */
        final public static function term(string $term) {}

        /**
         * @param string $wildcard
         * @return WildcardSearchQuery
         */
        final public static function wildcard(string $wildcard) {}

        /**
         * @param string $field
         * @param int $limit
         * @return TermSearchFacet
         */
        final public static function termFacet(string $field, int $limit) {}

        /**
         * @param string $field
         * @param int $limit
         * @return DateRangeSearchFacet
         */
        final public static function dateRangeFacet(string $field, int $limit) {}

        /**
         * @param string $field
         * @param int $limit
         * @return NumericRangeSearchFacet
         */
        final public static function numericRangeFacet(string $field, int $limit) {}

        /**
         * @param string $indexName
         * @param SearchQueryPart $queryPart
         */
        final public function __construct(string $indexName, SearchQueryPart $queryPart) {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param int $limit
         * @return SearchQuery
         */
        final public function limit(int $limit) {}

        /**
         * @param int $skip
         * @return SearchQuery
         */
        final public function skip(int $skip) {}

        /**
         * @param boolean $explain
         * @return SearchQuery
         */
        final public function explain(boolean $explain) {}

        /**
         * @param int $serverSideTimeout
         * @return SearchQuery
         */
        final public function serverSideTimeout(int $serverSideTimeout) {}

        /**
         * @param int $consistentWith
         * @return SearchQuery
         */
        final public function consistentWith(MutationState $state) {}

        /**
         * @param string ...$fields
         * @return SearchQuery
         */
        final public function fields(string ...$fields) {}

        /**
         * @param string $style
         *         Possible values:
         *           - HIGHLIGHT_HTML
         *           - HIGHLIGHT_ANSI
         *           - HIGHLIGHT_SIMPLE
         * @param string ...$fields
         * @return SearchQuery
         */
        final public function highlight(string $style, string ...$fields) {}

        /**
         * @param string $name
         * @param SearchFacet $facet
         * @return SearchQuery
         */
        final public function addFacet(string $name, SearchFacet $facet) {}
    }

    interface SearchQueryPart {}

    final class BooleanFieldSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return BooleanFieldSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return BooleanFieldSearchQuery
         */
        final public function field(string $field) {}
    }

    final class BooleanSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return BooleanSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return BooleanSearchQuery
         */
        final public function must(SearchQueryPart ...$queries) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return BooleanSearchQuery
         */
        final public function mustNot(SearchQueryPart ...$queries) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return BooleanSearchQuery
         */
        final public function should(SearchQueryPart ...$queries) {}
    }

    final class ConjunctionSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return ConjunctionSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return ConjunctionSearchQuery
         */
        final public function and(SearchQueryPart ...$queries) {}
    }

    final class DisjunctionSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return DisjunctionSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param SearchQueryPart ...$queries
         * @return DisjunctionSearchQuery
         */
        final public function or(SearchQueryPart ...$queries) {}

        /**
         * @param int $min
         * @return DisjunctionSearchQuery
         */
        final public function min(int $min) {}

    }

    final class DateRangeSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return DateRangeSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return DateRangeSearchQuery
         */
        final public function field(string $field) {}

        /**
         * @param int|string $start The strings will be taken verbatim and supposed to be formatted with custom date
         *      time formatter (see dateTimeParser). Integers interpreted as unix timestamps and represented as RFC3339
         *      strings.
         * @param boolean $inclusive
         * @return DateRangeSearchQuery
         */
        final public function start(mixed $start, boolean $inclusive = true) {}

        /**
         * @param int|string $end The strings will be taken verbatim and supposed to be formatted with custom date
         *      time formatter (see dateTimeParser). Integers interpreted as unix timestamps and represented as RFC3339
         *      strings.
         * @param boolean $inclusive
         * @return DateRangeSearchQuery
         */
        final public function end(mixed $end, boolean $inclusive = false) {}

        /**
         * @param string $dateTimeParser
         * @return DateRangeSearchQuery
         */
        final public function dateTimeParser(string $dateTimeParser) {}
    }

    final class NumericRangeSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return NumericRangeSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return NumericRangeSearchQuery
         */
        final public function field(string $field) {}

        /**
         * @param float $min
         * @param boolean $inclusive
         * @return NumericRangeSearchQuery
         */
        final public function min(float $min, boolean $inclusive = true) {}

        /**
         * @param float $max
         * @param boolean $inclusive
         * @return NumericRangeSearchQuery
         */
        final public function max(float $max, boolean $inclusive = false) {}
    }

    final class DocIdSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return DocIdSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return DocIdSearchQuery
         */
        final public function field(string $field) {}

        /**
         * @param string ...$documentIds
         * @return DocIdSearchQuery
         */
        final public function docIds(string ...$documentIds) {}
    }

    final class MatchAllSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return MatchAllSearchQuery
         */
        final public function boost(float $boost) {}
    }

    final class MatchNoneSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return MatchNoneSearchQuery
         */
        final public function boost(float $boost) {}
    }

    final class MatchPhraseSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return MatchPhraseSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return MatchPhraseSearchQuery
         */
        final public function field(string $field) {}

        /**
         * @param string $analyzer
         * @return MatchPhraseSearchQuery
         */
        final public function analyzer(string $analyzer) {}
    }

    final class MatchSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return MatchSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return MatchSearchQuery
         */
        final public function field(string $field) {}

        /**
         * @param string $analyzer
         * @return MatchSearchQuery
         */
        final public function analyzer(string $analyzer) {}

        /**
         * @param int $prefixLength
         * @return MatchSearchQuery
         */
        final public function prefixLength(int $prefixLength) {}

        /**
         * @param int $fuzziness
         * @return MatchSearchQuery
         */
        final public function fuzziness(int $fuzziness) {}
    }

    final class PhraseSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return PhraseSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return PhraseSearchQuery
         */
        final public function field(string $field) {}
    }

    final class RegexpSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return RegexpSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return RegexpSearchQuery
         */
        final public function field(string $field) {}
    }

    final class WildcardSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return WildcardSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return WildcardSearchQuery
         */
        final public function field(string $field) {}
    }

    final class PrefixSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return PrefixSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return PrefixSearchQuery
         */
        final public function field(string $field) {}
    }

    final class QueryStringSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return QueryStringSearchQuery
         */
        final public function boost(float $boost) {}
    }

    final class TermSearchQuery implements JsonSerializable, SearchQueryPart {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param float $boost
         * @return TermSearchQuery
         */
        final public function boost(float $boost) {}

        /**
         * @param string $field
         * @return TermSearchQuery
         */
        final public function field(string $field) {}

        /**
         * @param int $prefixLength
         * @return TermSearchQuery
         */
        final public function prefixLength(int $prefixLength) {}

        /**
         * @param int $fuzziness
         * @return TermSearchQuery
         */
        final public function fuzziness(int $fuzziness) {}
    }

    interface SearchFacet {}

    final class TermSearchFacet implements JsonSerializable, SearchFacet {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}
    }

    final class DateRangeSearchFacet implements JsonSerializable, SearchFacet {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param string $name
         * @param int|string $start
         * @param int|string $end
         * @return DateSearchFacet
         */
        final public function addRange(string $name, mixed $start, mixed $end) {}
    }

    final class NumericSearchFacet implements JsonSerializable, SearchFacet {
        final private function __construct() {}

        /**
         * @return array
         */
        final public function jsonSerialize() {}

        /**
         * @param string $name
         * @param float $min
         * @param float $max
         * @return NumericSearchFacet
         */
        final public function addRange(string $name, float $min, float $max) {}
    }
}