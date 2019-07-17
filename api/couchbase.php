<?php
/**
 * INI entries:
 *
 * * `couchbase.log_level` (string), default: `"WARN"`
 *
 *   controls amount of information, the module will send to PHP error log. Accepts the following values in order of
 *   increasing verbosity: `"FATAL"`, `"ERROR"`, `"WARN"`, `"INFO"`, `"DEBUG"`, `"TRACE"`.
 *
 * * `couchbase.encoder.format` (string), default: `"json"`
 *
 *   selects serialization format for default encoder (\Couchbase\defaultEncoder). Accepts the following values:
 *   * `"json"` - encodes objects and arrays as JSON object (using `json_encode()`), primitives written in stringified form,
 *      which is allowed for most of the JSON parsers as valid values. For empty arrays JSON array preferred, if it is
 *      necessary, use `new stdClass()` to persist empty JSON object. Note, that only JSON format considered supported by
 *      all Couchbase SDKs, everything else is private implementation (i.e. `"php"` format won't be readable by .NET SDK).
 *   * `"php"` - uses PHP serialize() method to encode the document.
 *   * `"igbinary"` - uses pecl/igbinary to encode the document in even more efficient than `"php"` format. Might not be
 *      available, if the Couchbase PHP SDK didn't find it during build phase, in this case constant
 *      \Couchbase\HAVE_IGBINARY will be false.
 *
 * * `couchbase.encoder.compression` (string), default: `"none"`
 *
 *   selects compression algorithm. Also see related compression options below. Accepts the following values:
 *   * `"fastlz"` - uses FastLZ algorithm. The module might be configured to use system fastlz library during build,
 *     othewise vendored version will be used. This algorithm is always available.
 *   * `"zlib"` - uses compression implemented by libz. Might not be available, if the system didn't have libz headers
 *     during build phase. In this case \Couchbase\HAVE_ZLIB will be false.
 *   * `"off"` or `"none"` - compression will be disabled, but the library will still read compressed values.
 *
 * * `couchbase.encoder.compression_threshold` (long), default: `0`
 *
 *   controls minimum size of the document value in bytes to use compression. For example, if threshold 100 bytes,
 *   and the document size is 50, compression will be disabled for this particular document.
 *
 * * `couchbase.encoder.compression_factor` (float), default: `0.0`
 *
 *   controls the minimum ratio of the result value and original document value to proceed with persisting compressed
 *   bytes. For example, the original document consists of 100 bytes. In this case factor 1.0 will require compressor
 *   to yield values not larger than 100 bytes (100/1.0), and 1.5 -- not larger than 66 bytes (100/1.5).
 *
 * * `couchbase.decoder.json_arrays` (boolean), default: `false`
 *
 *   controls the form of the documents, returned by the server if they were in JSON format. When true, it will generate
 *   arrays of arrays, otherwise instances of stdClass.
 *
 * * `couchbase.pool.max_idle_time_sec` (long), default: `60`
 *
 *   controls the maximum interval the underlying connection object could be idle, i.e. without any data/query
 *   operations. All connections which idle more than this interval will be closed automatically. Cleanup function
 *   executed after each request using RSHUTDOWN hook.
 *
 * @package Couchbase
 */

namespace Couchbase {

    use JsonSerializable;

    /**
     * An object which contains meta information of the document needed to enforce query consistency.
     */
    interface MutationToken
    {
        /**
         * Returns bucket name
         *
         * @return string
         */
        public function bucketName();

        /**
         * Returns partition number
         *
         * @return int
         */
        public function partitionId();

        /**
         * Returns UUID of the partition
         *
         * @return string
         */
        public function partitionUuid();

        /**
         * Returns the sequence number inside partition
         *
         * @return string
         */
        public function sequenceNumber();
    }


    interface MetaData
    {
        public function status(): ?string;

        public function requestId(): ?string;

        public function clientContextId(): ?string;

        public function signature(): ?object;

        public function warnings(): ?array;

        public function errors(): ?array;

        public function metrics(): ?object;

        public function profile(): ?object;
    }

    interface SearchMetaData
    {
        public function successCount(): ?int;

        public function errorCount(): ?int;

        public function took(): ?int;

        public function totalHits(): ?int;

        public function maxScore(): ?float;

        public function metrics(): ?object;
    }

    interface ViewMetaData
    {
        public function totalRows(): ?int;
    }

    interface Result
    {
        public function cas(): ?string;

        public function expiration(): ?int;
    }

    interface GetResult extends Result
    {
        public function content(): ?object;
    }

    interface GetReplicaResult extends Result
    {

        public function content(): ?object;

        public function isMaster(): bool;
    }

    interface ExistsResult extends Result
    {
        public function exists(): bool;
    }

    interface MutationResult extends Result
    {
        public function mutationToken(): ?MutationToken;
    }

    interface CounterResult extends Result
    {
        public function content(): int;
    }


    interface LookupInResult extends Result
    {
        public function content(int $index): ?object;

        public function exists(int $index): bool;
    }

    interface MutateInResult extends MutationResult, Result
    {
        public function content(int $index): ?object;
    }

    interface QueryResult
    {
        public function metadata(): ?MetaData;

        public function rows(): ?array;
    }

    interface AnalyticsQueryResult
    {
        public function metadata(): ?MetaData;

        public function rows(): ?array;
    }

    interface SearchResult
    {
        public function metadata(): ?SearchMetaData;

        public function facets(): ?array;

        public function hits(): ?array;
    }

    interface ViewResult
    {
        public function metadata(): ?ViewMetaData;

        public function rows(): ?array;
    }

    class ViewResultEntry
    {
        public function id(): ?string
        {
        }

        public function key(): ?object
        {
        }

        public function value(): ?object
        {
        }

        public function document(): ?object
        {
        }
    }

    class BaseException extends Exception implements Throwable
    {
        public function ref(): ?string
        {
        }

        public function context(): ?object
        {
        }
    }

    class HttpException extends BaseException implements Throwable
    {
    }

    class QueryException extends HttpException implements Throwable
    {
    }

    class QueryErrorException extends QueryException implements Throwable
    {
    }

    class QueryServiceException extends QueryException implements Throwable
    {
    }

    class SearchException extends HttpException implements Throwable
    {
    }

    class AnalyticsException extends HttpException implements Throwable
    {
    }

    class ViewException extends HttpException implements Throwable
    {
    }

    class PartialViewException extends HttpException implements Throwable
    {
    }

    class BindingsException extends BaseException implements Throwable
    {
    }

    class InvalidStateException extends BaseException implements Throwable
    {
    }

    class KeyValueException extends BaseException implements Throwable
    {
    }

    class KeyNotFoundException extends KeyValueException implements Throwable
    {
    }

    class KeyExistsException extends KeyValueException implements Throwable
    {
    }

    class ValueTooBigException extends KeyValueException implements Throwable
    {
    }

    class KeyLockedException extends KeyValueException implements Throwable
    {
    }

    class TempFailException extends KeyValueException implements Throwable
    {
    }

    class PathNotFoundException extends KeyValueException implements Throwable
    {
    }

    class PathExistsException extends KeyValueException implements Throwable
    {
    }

    class InvalidRangeException extends KeyValueException implements Throwable
    {
    }

    class KeyDeletedException extends KeyValueException implements Throwable
    {
    }

    class CasMismatchException extends KeyValueException implements Throwable
    {
    }

    class InvalidConfigurationException extends BaseException implements Throwable
    {
    }

    class ServiceMissingException extends BaseException implements Throwable
    {
    }

    class NetworkException extends BaseException implements Throwable
    {
    }

    class TimeoutException extends BaseException implements Throwable
    {
    }

    class BucketMissingException extends BaseException implements Throwable
    {
    }

    class ScopeMissingException extends BaseException implements Throwable
    {
    }

    class CollectionMissingException extends BaseException implements Throwable
    {
    }

    class AuthenticationException extends BaseException implements Throwable
    {
    }

    class BadInputException extends BaseException implements Throwable
    {
    }

    class DurabilityException extends BaseException implements Throwable
    {
    }

    class SubdocumentException extends BaseException implements Throwable
    {
    }


    class Cluster
    {
        public function __construct(string $connstr)
        {
        }

        public function bucket(string $connstr): Bucket
        {
        }

        public function manager(): ClusterManager
        {
        }

        public function authenticate(Authenticator $authenticator)
        {
        }

        public function authenticateAs(string $username, string $password)
        {
        }

        public function query(string $statement, QueryOptions $options = null)
        {
        }

        public function analyticsQuery(string $statement, AnalyticsQueryOptions $options = null)
        {
        }

        public function searchQuery(string $indexName, SearchQuery $query, SearchQueryOptions $options = null)
        {
        }
    }

    class Collection
    {
        public function __construct(Bucket $bucket, string $scope, string $name)
        {
        }

        public function get(string $id, GetOptions $options = null): GetResult
        {
        }

        public function exists(string $id, ExistsOptions $options = null): ExistsResult
        {
        }

        public function getAndLock(string $id, int $lockTime, GetAndLockOptions $options = null): GetResult
        {
        }

        public function getAndTouch(string $id, int $expiry, GetAndTouchOptions $options = null): GetResult
        {
        }

        public function getAnyReplica(string $id, GetAnyReplicaOptions $options = null): GetReplicaResult
        {
        }

        public function getAllReplica(string $id, GetAllReplicaOptions $options = null): GetReplicaResult
        {
        }

        public function upsert(string $id, $value, UpsertOptions $options = null): StoreResult
        {
        }

        public function insert(string $id, $value, InsertOptions $options = null): StoreResult
        {
        }

        public function replace(string $id, $value, ReplaceOptions $options = null): StoreResult
        {
        }

        public function append(string $id, string $value, AppendOptions $options = null): StoreResult
        {
        }

        public function prepend(string $id, string $value, PrependOptions $options = null): StoreResult
        {
        }

        public function remove(string $id, RemoveOptions $options = null): MutationResult
        {
        }

        public function unlock(string $id, string $cas, UnlockOptions $options = null): Result
        {
        }

        public function touch(string $id, int $expiry, TcouchOptions $options = null): Result
        {
        }

        public function increment(string $id, CounterOptions $options = null): CounterResult
        {
        }

        public function decrement(string $id, CounterOptions $options = null): CounterResult
        {
        }

        public function lookupIn(string $id, array $specs, LookupInOptions $options = null): LookupInResult
        {
        }

        public function mutateIn(string $id, array $specs, MutateInOptions $options = null): MutateInResult
        {
        }
    }

    class Scope
    {
        public function __construct(Bucket $bucket, string $name)
        {
        }

        public function collection(string $name): Collection
        {
        }
    }

    class ClusterManager
    {
        public const RBAC_DOMAIN_LOCAL = 1;
        public const RBAC_DOMAIN_EXTERNAL 2;

        final private function __construct()
        {
        }

        public function listBuckets()
        {
        }

        public function createBucket($name, $options)
        {
        }

        public function removeBucket($name)
        {
        }

        public function listUsers($dodmain)
        {
        }

        public function upsertUser($name, $settings, $domain)
        {
        }

        public function getUser($name, $domain)
        {
        }

        public function remove($name)
        {
        }

        public function info()
        {
        }
    }

    class UserSettings
    {
        public function __construct()
        {
        }

        public function fullName($fullName)
        {
        }

        public function password($password)
        {
        }

        public function role($role)
        {
        }
    }

    class Bucket
    {
        final private function __construct()
        {
        }

        private function __get($name)
        {
        }

        private function __set($name, $value)
        {
        }

        public function defaultCollection(): Collection
        {
        }

        public function scope(string $name): Scope
        {
        }

        public function setTranscoder(callable $encoder, callable $decoder)
        {
        }

        public function name(): string
        {
        }

        public function manager(): BucketManager
        {
        }

        public function viewQuery(string $designDoc, string $viewName, ViewOptions $options = null): ViewResult
        {
        }

        public function ping($services, $reportId)
        {
        }

        public function diagnostics($reportId)
        {
        }
    }

    class BucketManager
    {
        final private function __construct()
        {
        }

        public function info()
        {
        }

        public function flush()
        {
        }

        public function listDesignDocuments()
        {
        }

        public function getDesignDocument($name)
        {
        }

        public function removeDesignDocument($name)
        {
        }

        public function upsertDesignDocument($name, $document)
        {
        }

        public function insertDesignDocument($name, $document)
        {
        }

        public function listN1qlIndexes()
        {
        }

        public function createN1qlPrimaryIndex($customName, $ignoreIfExists, $defer)
        {
        }

        public function createN1qlIndex($indexName, $fields, $whereClause, $ignoreIfExists, $defer)
        {
        }

        public function dropN1qlPrimaryIndex($customName, $ignoreIfNotExists, $defer)
        {
        }

        public function dropN1qlIndex($indexName, $ignoreIfNotExists, $defer)
        {
        }

        public function searchIndexManager()
        {
        }
    }

    /**
     * Interface of authentication containers.
     *
     * @see \Couchbase\Cluster::authenticate()
     * @see \Couchbase\ClassicAuthenticator
     * @see \Couchbase\PasswordAuthenticator
     * @see \Couchbase\CertAuthenticator
     */
    interface Authenticator
    {
    }

    /**
     * Authenticator for Client Certificate authentication feature of Couchbase Server 5+.
     *
     * This authenticator does not have any attributes, but ensures that
     * "certpath" and "keypath" options specified in the connection string
     *
     * @see \Couchbase\Cluster::authenticate()
     * @see \Couchbase\Authenticator
     */
    class CertAuthenticator implements Authenticator
    {
    }


    /**
     * Authenticator based on login/password credentials.
     *
     * This authenticator uses separate credentials for Cluster management interface
     * as well as for each bucket.
     *
     * @example examples/api/couchbase.Authenticator.php Cluster authentication
     *
     * @example examples/api/couchbase.N1qlQuery.crossBucket.php Cross-bucket N1QL query
     *
     * @see \Couchbase\Cluster::authenticate()
     * @see \Couchbase\Authenticator
     */
    class ClassicAuthenticator implements Authenticator
    {
        /**
         * Registers cluster management credentials in the container
         *
         * @param string $username admin username
         * @param string $password admin password
         */
        public function cluster($username, $password)
        {
        }

        /**
         * Registers bucket credentials in the container
         *
         * @param string $name bucket name
         * @param string $password bucket password
         */
        public function bucket($name, $password)
        {
        }
    }

    /**
     * Authenticator based on RBAC feature of Couchbase Server 5+.
     *
     * This authenticator uses single credentials for all operations (data and management).
     *
     * @see \Couchbase\Cluster::authenticate()
     * @see \Couchbase\Authenticator
     */
    class PasswordAuthenticator implements Authenticator
    {
        /**
         * Sets username
         *
         * @param string $username username
         * @return \Couchbase\PasswordAuthenticator
         */
        public function username($username)
        {
        }

        /**
         * Sets password
         *
         * @param string $password password
         * @return \Couchbase\PasswordAuthenticator
         */
        public function password($password)
        {
        }
    }

    class AnalyticsQueryOptions
    {
        public function timeout(int $arg): AnalyticsQueryOptions
        {
        }

        public function namedParameters(array $pairs): AnalyticsQueryOptions
        {
        }

        public function positionalParameters(array $args): AnalyticsQueryOptions
        {
        }

        public function rawParameter(string $key, $value): AnalyticsQueryOptions
        {
        }
    }

    interface LookupInSpec
    {
    }

    class LookupGetSpec implements LookupInSpec
    {
        public function __construct(string $path, bool $isXattr = false)
        {
        }
    }

    class LookupCountSpec implements LookupInSpec
    {
        public function __construct(string $path, bool $isXattr = false)
        {
        }
    }

    class LookupExistsSpec implements LookupInSpec
    {
        public function __construct(string $path, bool $isXattr = false)
        {
        }
    }

    class LookupGetFullSpec implements LookupInSpec
    {
        public function __construct()
        {
        }
    }

    interface MutateInSpec
    {
    }

    class MutateInsertSpec implements MutateInSpec
    {
        public function __construct(string $path, $value, bool $isXattr, bool $createPath, bool $expandMacros)
        {
        }
    }

    class MutateUpsertSpec implements MutateInSpec
    {
        public function __construct(string $path, $value, bool $isXattr, bool $createPath, bool $expandMacros)
        {
        }
    }

    class MutateReplaceSpec implements MutateInSpec
    {
        public function __construct(string $path, $value, bool $isXattr)
        {
        }
    }

    class MutateRemoveSpec implements MutateInSpec
    {
        public function __construct(string $path, bool $isXattr)
        {
        }
    }

    class MutateArrayAppendSpec implements MutateInSpec
    {
        public function __construct(string $path, array $values, bool $isXattr, bool $createPath, bool $expandMacros)
        {
        }
    }

    class MutateArrayPrependSpec implements MutateInSpec
    {
        public function __construct(string $path, array $values, bool $isXattr, bool $createPath, bool $expandMacros)
        {
        }
    }

    class MutateArrayInsertSpec implements MutateInSpec
    {
        public function __construct(string $path, array $values, bool $isXattr, bool $createPath, bool $expandMacros)
        {
        }
    }

    class MutateArrayAddUniqueSpec implements MutateInSpec
    {
        public function __construct(string $path, $value, bool $isXattr, bool $createPath, bool $expandMacros)
        {
        }
    }

    class MutateInsertFullSpec implements MutateInSpec
    {
        public function __construct($value)
        {
        }
    }

    class MutateUpsertFullSpec implements MutateInSpec
    {
        public function __construct($value)
        {
        }
    }

    class MutateReplaceFullSpec implements MutateInSpec
    {
        public function __construct($value)
        {
        }
    }

    class MutateCounterSpec implements MutateInSpec
    {
        public function __construct(string $path, int $delta, bool $isXattr, bool $createPath)
        {
        }
    }


    class GetOptions
    {
        public function timeout(int $arg): GetOptions
        {
        }

        public function withExpiration(bool $arg): GetOptions
        {
        }

        public function project(array $arg): GetOptions
        {
        }
    }

    class GetAndTouchOptions
    {
        public function timeout(int $arg): GetAndTouchOptions
        {
        }
    }

    class GetAndLockOptions
    {
        public function timeout(int $arg): GetAndLockOptions
        {
        }
    }

    class GetAllReplicasOptions
    {
        public function timeout(int $arg): GetAllReplicasOptions
        {
        }
    }

    class GetAnyReplicaOptions
    {
        public function timeout(int $arg): GetAnyReplicasOptions
        {
        }
    }

    class ExistsOptions
    {
        public function timeout(int $arg): ExistsOptions
        {
        }
    }

    class UnlockOptions
    {
        public function timeout(int $arg): UnlockOptions
        {
        }
    }

    class InsertOptions
    {
        public function timeout(int $arg): InsertOptions
        {
        }

        public function expiration(int $arg): InsertOptions
        {
        }

        public function durabilityLevel(int $arg): InsertOptions
        {
        }
    }

    class UpsertOptions
    {
        public function timeout(int $arg): UpsertOptions
        {
        }

        public function expiration(int $arg): UpsertOptions
        {
        }

        public function cas(string $arg): UpsertOptions
        {
        }

        public function durabilityLevel(int $arg): UpsertOptions
        {
        }
    }

    class ReplaceOptions
    {
        public function timeout(int $arg): ReplaceOptions
        {
        }

        public function expiration(int $arg): ReplaceOptions
        {
        }

        public function cas(string $arg): ReplaceOptions
        {
        }

        public function durabilityLevel(int $arg): ReplaceOptions
        {
        }
    }

    class AppendOptions
    {
        public function timeout(int $arg): AppendOptions
        {
        }

        public function expiration(int $arg): AppendOptions
        {
        }

        public function durabilityLevel(int $arg): AppendOptions
        {
        }
    }

    class PrependOptions
    {
        public function timeout(int $arg): PrependOptions
        {
        }

        public function expiration(int $arg): PrependOptions
        {
        }

        public function durabilityLevel(int $arg): PrependOptions
        {
        }
    }

    interface DurabilityLevel
    {
        public const NONE = 0;
        public const MAJORITY = 1;
        public const MAJORITY_AND_PERSIST_ON_MASTER = 2;
        public const PERSIST_TO_MAJORITY = 3;
    }

    class TouchOptions
    {
        public function timeout(int $arg): TouchOptions
        {
        }
    }

    class CounterOptions
    {
        public function timeout(int $arg): CounterOptions
        {
        }

        public function expiration(int $arg): CounterOptions
        {
        }

        public function durabilitLevel(int $arg): CounterOptions
        {
        }

        public function delta(int $arg): CounterOptions
        {
        }

        public function initial(int $arg): CounterOptions
        {
        }
    }

    class RemoveOptions
    {
        public function timeout(int $arg): RemoveOptions
        {
        }

        public function durabilitLevel(int $arg): RemoveOptions
        {
        }

        public function cas(string $arg): RemoveOptions
        {
        }
    }

    class LookupInOptions
    {
        public function timeout(int $arg): LookupInOptions
        {
        }

        public function withExpiration(bool $arg): LookupInOptions
        {
        }
    }

    class MutateInOptions
    {
        public function timeout(int $arg): MutateInOptions
        {
        }

        public function cas(string $arg): MutateInOptions
        {
        }

        public function expiration(int $arg): MutateInOptions
        {
        }

        public function durabilityLevel(int $arg): MutateInOptions
        {
        }
    }

    class ViewOptions
    {
        public function timeout(int $arg): ViewOptions
        {
        }

        public function includeDocs(bool $arg): ViewOptions
        {
        }

        public function maxConcurrentDocs(int $arg): ViewOptions
        {
        }

        public function key($arg): ViewOptions
        {
        }

        public function keys(array $args): ViewOptions
        {
        }

        public function limit(int $arg): ViewOptions
        {
        }

        public function skip(int $arg): ViewOptions
        {
        }

        public function consistency(int $arg): ViewOptions
        {
        }

        public function order(int $arg): ViewOptions
        {
        }

        public function reduce(bool $arg): ViewOptions
        {
        }

        public function group(bool $arg): ViewOptions
        {
        }

        public function groupLevel(int $arg): ViewOptions
        {
        }

        public function range($start, $end, $inclusiveEnd = false): ViewOptions
        {
        }

        public function idRange($start, $end, $inclusiveEnd = false): ViewOptions
        {
        }

        public function rawParameter(string $key, $value): ViewOptions
        {
        }
    }

    interface ViewConsistency
    {
        public const NONE = 0;
        public const UPDATE_BEFORE = 1;
        public const UPDATE_AFTER = 2;
    }

    interface ViewOrder
    {
        public const ASCENDING = 0;
        public const DESCENDING = 1;
    }

    class QueryOptions
    {
        public function timeout(int $arg): QueryOptions
        {
        }

        public function consistency(int $arg): QueryOptions
        {
        }

        public function scanCap(int $arg): QueryOptions
        {
        }

        public function pipelineCap(int $arg): QueryOptions
        {
        }

        public function pipelineBatch(int $arg): QueryOptions
        {
        }

        public function maxParallelism(int $arg): QueryOptions
        {
        }

        public function profile(int $arg): QueryOptions
        {
        }

        public function readonly(bool $arg): QueryOptions
        {
        }

        public function adhoc(bool $arg): QueryOptions
        {
        }

        public function namedParameters(array $pairs): QueryOptions
        {
        }

        public function positionalParameters(array $args): QueryOptions
        {
        }

        public function rawParameter(string $key, $value): QueryOptions
        {
        }
    }

    interface QueryConsistency
    {
        public const NOT_BOUNDED = 1;
        public const REQUEST_PLUS = 2;
        public const STATEMENT_PLUS = 3;
    }

    interface QueryProfile
    {
        public const NONE = 1;
        public const PHASES = 2;
        public const TIMINGS = 3;
    }


    /**
     * Represents full text search query
     *
     * @see https://developer.couchbase.com/documentation/server/4.6/sdk/php/full-text-searching-with-sdk.html
     *   Searching from the SDK
     */
    class SearchQuery implements JsonSerializable
    {
        const HIGHLIGHT_HTML = 'html';
        const HIGHLIGHT_ANSI = 'ansi';
        const HIGHLIGHT_SIMPLE = 'simple';

        /**
         * Prepare boolean search query
         *
         * @return BooleanSearchQuery
         */
        public static function boolean()
        {
        }

        /**
         * Prepare date range search query
         *
         * @return DateRangeSearchQuery
         */
        public static function dateRange()
        {
        }

        /**
         * Prepare numeric range search query
         *
         * @return NumericRangeSearchQuery
         */
        public static function numericRange()
        {
        }

        /**
         * Prepare term range search query
         *
         * @return TermRangeSearchQuery
         */
        public static function termRange()
        {
        }

        /**
         * Prepare boolean field search query
         *
         * @param bool $value
         * @return BooleanFieldSearchQuery
         */
        public static function booleanField($value)
        {
        }

        /**
         * Prepare compound conjunction search query
         *
         * @param SearchQueryPart ...$queries list of inner query parts
         * @return ConjunctionSearchQuery
         */
        public static function conjuncts(...$queries)
        {
        }

        /**
         * Prepare compound disjunction search query
         *
         * @param SearchQueryPart ...$queries list of inner query parts
         * @return DisjunctionSearchQuery
         */
        public static function disjuncts(...$queries)
        {
        }

        /**
         * Prepare document ID search query
         *
         * @param string ...$documentIds
         * @return DocIdSearchQuery
         */
        public static function docId(...$documentIds)
        {
        }

        /**
         * Prepare match search query
         *
         * @param string $match
         * @return MatchSearchQuery
         */
        public static function match($match)
        {
        }

        /**
         * Prepare match all search query
         *
         * @return MatchAllSearchQuery
         */
        public static function matchAll()
        {
        }

        /**
         * Prepare match non search query
         *
         * @return MatchNoneSearchQuery
         */
        public static function matchNone()
        {
        }

        /**
         * Prepare phrase search query
         *
         * @param string ...$terms
         * @return MatchPhraseSearchQuery
         */
        public static function matchPhrase(...$terms)
        {
        }

        /**
         * Prepare prefix search query
         *
         * @param string $prefix
         * @return PrefixSearchQuery
         */
        public static function prefix($prefix)
        {
        }

        /**
         * Prepare query string search query
         *
         * @param string $queryString
         * @return QueryStringSearchQuery
         */
        public static function queryString($queryString)
        {
        }

        /**
         * Prepare regexp search query
         *
         * @param string $regexp
         * @return RegexpSearchQuery
         */
        public static function regexp($regexp)
        {
        }

        /**
         * Prepare term search query
         *
         * @param string $term
         * @return TermSearchQuery
         */
        public static function term($term)
        {
        }

        /**
         * Prepare wildcard search query
         *
         * @param string $wildcard
         * @return WildcardSearchQuery
         */
        public static function wildcard($wildcard)
        {
        }

        /**
         * Prepare geo distance search query
         *
         * @param float $longitude
         * @param float $latitude
         * @param string $distance e.g. "10mi"
         * @return GeoDistanceSearchQuery
         */
        public static function geoDistance($longitude, $latitude, $distance)
        {
        }

        /**
         * Prepare geo bounding box search query
         *
         * @param float $topLeftLongitude
         * @param float $topLeftLatitude
         * @param float $bottomRightLongitude
         * @param float $bottomRightLatitude
         * @return GeoBoundingBoxSearchQuery
         */
        public static function geoBoundingBox($topLeftLongitude, $topLeftLatitude, $bottomRightLongitude, $bottomRightLatitude)
        {
        }

        /**
         * Prepare term search facet
         *
         * @param string $field
         * @param int $limit
         * @return TermSearchFacet
         */
        public static function termFacet($field, $limit)
        {
        }

        /**
         * Prepare date range search facet
         *
         * @param string $field
         * @param int $limit
         * @return DateRangeSearchFacet
         */
        public static function dateRangeFacet($field, $limit)
        {
        }

        /**
         * Prepare numeric range search facet
         *
         * @param string $field
         * @param int $limit
         * @return NumericRangeSearchFacet
         */
        public static function numericRangeFacet($field, $limit)
        {
        }

        /**
         * Prepare an FTS SearchQuery on an index.
         *
         * Top level query parameters can be set after that by using the fluent API.
         *
         * @param string $indexName the FTS index to search in
         * @param SearchQueryPart $queryPart the body of the FTS query (e.g. a match phrase query)
         */
        public function __construct($indexName, $queryPart)
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * Add a limit to the query on the number of hits it can return
         *
         * @param int $limit the maximum number of hits to return
         * @return SearchQuery
         */
        public function limit($limit)
        {
        }

        /**
         * Set the number of hits to skip (eg. for pagination).
         *
         * @param int $skip the number of results to skip
         * @return SearchQuery
         */
        public function skip($skip)
        {
        }

        /**
         * Activates the explanation of each result hit in the response
         *
         * @param bool $explain
         * @return SearchQuery
         */
        public function explain($explain)
        {
        }

        /**
         * Sets the server side timeout in milliseconds
         *
         * @param int $serverSideTimeout the server side timeout to apply
         * @return SearchQuery
         */
        public function serverSideTimeout($serverSideTimeout)
        {
        }

        /**
         * Sets the consistency to consider for this FTS query to AT_PLUS and
         * uses the MutationState to parameterize the consistency.
         *
         * This replaces any consistency tuning previously set.
         *
         * @param MutationState $state the mutation state information to work with
         * @return SearchQuery
         */
        public function consistentWith($state)
        {
        }

        /**
         * Configures the list of fields for which the whole value should be included in the response.
         *
         * If empty, no field values are included. This drives the inclusion of the fields in each hit.
         * Note that to be highlighted, the fields must be stored in the FTS index.
         *
         * @param string ...$fields
         * @return SearchQuery
         */
        public function fields(...$fields)
        {
        }

        /**
         * Configures the highlighting of matches in the response
         *
         * @param string $style highlight style to apply. Use constants HIGHLIGHT_HTML,
         *   HIGHLIGHT_ANSI, HIGHLIGHT_SIMPLE.
         * @param string ...$fields the optional fields on which to highlight.
         *   If none, all fields where there is a match are highlighted.
         * @return SearchQuery
         *
         * @see \Couchbase\SearchQuery::HIGHLIGHT_HTML
         * @see \Couchbase\SearchQuery::HIGHLIGHT_ANSI
         * @see \Couchbase\SearchQuery::HIGHLIGHT_SIMPLE
         */
        public function highlight($style, ...$fields)
        {
        }

        /**
         * Configures the list of fields (including special fields) which are used for sorting purposes.
         * If empty, the default sorting (descending by score) is used by the server.
         *
         * The list of sort fields can include actual fields (like "firstname" but then they must be stored in the
         * index, configured in the server side mapping). Fields provided first are considered first and in a "tie" case
         * the next sort field is considered. So sorting by "firstname" and then "lastname" will first sort ascending by
         * the firstname and if the names are equal then sort ascending by lastname. Special fields like "_id" and
         * "_score" can also be used. If prefixed with "-" the sort order is set to descending.
         *
         * If no sort is provided, it is equal to sort("-_score"), since the server will sort it by score in descending
         * order.
         *
         * @param sort the fields that should take part in the sorting.
         * @return SearchQuery
         */
        public function sort(...$sort)
        {
        }

        /**
         * Adds one SearchFacet to the query
         *
         * This is an additive operation (the given facets are added to any facet previously requested),
         * but if an existing facet has the same name it will be replaced.
         *
         * Note that to be faceted, a field's value must be stored in the FTS index.
         *
         * @param string $name
         * @param SearchFacet $facet
         * @return SearchQuery
         *
         * @see \Couchbase\SearchFacet
         * @see \Couchbase\TermSearchFacet
         * @see \Couchbase\NumericRangeSearchFacet
         * @see \Couchbase\DateRangeSearchFacet
         */
        public function addFacet($name, $facet)
        {
        }
    }

    /**
     * Common interface for all classes, which could be used as a body of SearchQuery
     *
     * @see \Couchbase\SearchQuery::__construct()
     */
    interface SearchQueryPart
    {
    }

    /**
     * A FTS query that queries fields explicitly indexed as boolean.
     */
    class BooleanFieldSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return BooleanFieldSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return BooleanFieldSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * A compound FTS query that allows various combinations of sub-queries.
     */
    class BooleanSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return BooleanSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param SearchQueryPart ...$queries
         * @return BooleanSearchQuery
         */
        public function must(...$queries)
        {
        }

        /**
         * @param SearchQueryPart ...$queries
         * @return BooleanSearchQuery
         */
        public function mustNot(...$queries)
        {
        }

        /**
         * @param SearchQueryPart ...$queries
         * @return BooleanSearchQuery
         */
        public function should(...$queries)
        {
        }
    }

    /**
     * A compound FTS query that performs a logical AND between all its sub-queries (conjunction).
     */
    class ConjunctionSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return ConjunctionSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param SearchQueryPart ...$queries
         * @return ConjunctionSearchQuery
         */
        public function every(...$queries)
        {
        }
    }


    /**
     * A compound FTS query that performs a logical OR between all its sub-queries (disjunction). It requires that a
     * minimum of the queries match. The minimum is configurable (default 1).
     */
    class DisjunctionSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return DisjunctionSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param SearchQueryPart ...$queries
         * @return DisjunctionSearchQuery
         */
        public function either(...$queries)
        {
        }

        /**
         * @param int $min
         * @return DisjunctionSearchQuery
         */
        public function min($min)
        {
        }

    }

    /**
     * A FTS query that matches documents on a range of values. At least one bound is required, and the
     * inclusiveness of each bound can be configured.
     */
    class DateRangeSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return DateRangeSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return DateRangeSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param int|string $start The strings will be taken verbatim and supposed to be formatted with custom date
         *      time formatter (see dateTimeParser). Integers interpreted as unix timestamps and represented as RFC3339
         *      strings.
         * @param bool $inclusive
         * @return DateRangeSearchQuery
         */
        public function start($start, $inclusive = true)
        {
        }

        /**
         * @param int|string $end The strings will be taken verbatim and supposed to be formatted with custom date
         *      time formatter (see dateTimeParser). Integers interpreted as unix timestamps and represented as RFC3339
         *      strings.
         * @param bool $inclusive
         * @return DateRangeSearchQuery
         */
        public function end($end, $inclusive = false)
        {
        }

        /**
         * @param string $dateTimeParser
         * @return DateRangeSearchQuery
         */
        public function dateTimeParser($dateTimeParser)
        {
        }
    }

    /**
     * A FTS query that matches documents on a range of values. At least one bound is required, and the
     * inclusiveness of each bound can be configured.
     */
    class NumericRangeSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return NumericRangeSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return NumericRangeSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param float $min
         * @param bool $inclusive
         * @return NumericRangeSearchQuery
         */
        public function min($min, $inclusive = true)
        {
        }

        /**
         * @param float $max
         * @param bool $inclusive
         * @return NumericRangeSearchQuery
         */
        public function max($max, $inclusive = false)
        {
        }
    }

    /**
     * A FTS query that matches on Couchbase document IDs. Useful to restrict the search space to a list of keys (by using
     * this in a compound query).
     */
    class DocIdSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return DocIdSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return DocIdSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param string ...$documentIds
         * @return DocIdSearchQuery
         */
        public function docIds(...$documentIds)
        {
        }
    }

    /**
     * A FTS query that matches all indexed documents (usually for debugging purposes).
     */
    class MatchAllSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return MatchAllSearchQuery
         */
        public function boost($boost)
        {
        }
    }

    /**
     * A FTS query that matches 0 document (usually for debugging purposes).
     */
    class MatchNoneSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return MatchNoneSearchQuery
         */
        public function boost($boost)
        {
        }
    }

    /**
     * A FTS query that matches several given terms (a "phrase"), applying further processing
     * like analyzers to them.
     */
    class MatchPhraseSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return MatchPhraseSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return MatchPhraseSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param string $analyzer
         * @return MatchPhraseSearchQuery
         */
        public function analyzer($analyzer)
        {
        }
    }

    /**
     * A FTS query that matches a given term, applying further processing to it
     * like analyzers, stemming and even #fuzziness(int).
     */
    class MatchSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return MatchSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return MatchSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param string $analyzer
         * @return MatchSearchQuery
         */
        public function analyzer($analyzer)
        {
        }

        /**
         * @param int $prefixLength
         * @return MatchSearchQuery
         */
        public function prefixLength($prefixLength)
        {
        }

        /**
         * @param int $fuzziness
         * @return MatchSearchQuery
         */
        public function fuzziness($fuzziness)
        {
        }
    }

    /**
     * A FTS query that matches several terms (a "phrase") as is. The order of the terms mater and no further processing is
     * applied to them, so they must appear in the index exactly as provided.  Usually for debugging purposes, prefer
     * MatchPhraseQuery.
     */
    class PhraseSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return PhraseSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return PhraseSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * A FTS query that allows for simple matching of regular expressions.
     */
    class RegexpSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return RegexpSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return RegexpSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * A FTS query that allows for simple matching using wildcard characters (* and ?).
     */
    class WildcardSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return WildcardSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return WildcardSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * A FTS query that allows for simple matching on a given prefix.
     */
    class PrefixSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return PrefixSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return PrefixSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * A FTS query that performs a search according to the "string query" syntax.
     */
    class QueryStringSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return QueryStringSearchQuery
         */
        public function boost($boost)
        {
        }
    }

    /**
     * A facet that gives the number of occurrences of the most recurring terms in all hits.
     */
    class TermSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return TermSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return TermSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param int $prefixLength
         * @return TermSearchQuery
         */
        public function prefixLength($prefixLength)
        {
        }

        /**
         * @param int $fuzziness
         * @return TermSearchQuery
         */
        public function fuzziness($fuzziness)
        {
        }
    }

    /**
     * A FTS query that matches documents on a range of values. At least one bound is required, and the
     * inclusiveness of each bound can be configured.
     */
    class TermRangeSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return TermRangeSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return TermRangeSearchQuery
         */
        public function field($field)
        {
        }

        /**
         * @param string $min
         * @param bool $inclusive
         * @return TermRangeSearchQuery
         */
        public function min($min, $inclusive = true)
        {
        }

        /**
         * @param string $max
         * @param bool $inclusive
         * @return TermRangeSearchQuery
         */
        public function max($max, $inclusive = false)
        {
        }
    }

    /**
     * A FTS query that finds all matches from a given location (point) within the given distance.
     *
     * Both the point and the distance are required.
     */
    class GeoDistanceSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return GeoDistanceSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return GeoDistanceSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * A FTS query which allows to match geo bounding boxes.
     */
    class GeoBoundingBoxSearchQuery implements JsonSerializable, SearchQueryPart
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param float $boost
         * @return GeoBoundingBoxSearchQuery
         */
        public function boost($boost)
        {
        }

        /**
         * @param string $field
         * @return GeoBoundingBoxSearchQuery
         */
        public function field($field)
        {
        }
    }

    /**
     * Common interface for all search facets
     *
     * @see \Couchbase\SearchQuery::addFacet()
     * @see \Couchbase\TermSearchFacet
     * @see \Couchbase\DateRangeSearchFacet
     * @see \Couchbase\NumericRangeSearchFacet
     */
    interface SearchFacet
    {
    }

    /**
     * A facet that gives the number of occurrences of the most recurring terms in all hits.
     */
    class TermSearchFacet implements JsonSerializable, SearchFacet
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }
    }

    /**
     * A facet that categorizes hits inside date ranges (or buckets) provided by the user.
     */
    class DateRangeSearchFacet implements JsonSerializable, SearchFacet
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param string $name
         * @param int|string $start
         * @param int|string $end
         * @return DateSearchFacet
         */
        public function addRange($name, $start, $end)
        {
        }
    }

    /**
     * A facet that categorizes hits into numerical ranges (or buckets) provided by the user.
     */
    class NumericRangeSearchFacet implements JsonSerializable, SearchFacet
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * @return array
         * @ignore
         */
        public function jsonSerialize()
        {
        }

        /**
         * @param string $name
         * @param float $min
         * @param float $max
         * @return NumericSearchFacet
         */
        public function addRange($name, $min, $max)
        {
        }
    }

    /**
     * Base class for all FTS sort options in querying.
     */
    class SearchSort
    {
        /** @ignore */
        private function __construct()
        {
        }

        /**
         * Sort by the document identifier.
         *
         * @return SearchSortId
         */
        public static function id()
        {
        }

        /**
         * Sort by the hit score.
         *
         * @return SearchSortScore
         */
        public static function score()
        {
        }

        /**
         * Sort by a field in the hits.
         *
         * @param string $field the field name
         *
         * @return SearchSortField
         */
        public static function field($field)
        {
        }

        /**
         * Sort by geo location.
         *
         * @param string $field the field name
         * @param float $longitude the longitude of the location
         * @param float $latitude the latitude of the location
         *
         * @return SearchSortGeoDistance
         */
        public static function geoDistance($field, $longitude, $latitude)
        {
        }
    }

    /**
     * Sort by the document identifier.
     */
    class SearchSortId extends SearchSort implements JsonSerializable
    {
        /** @ignore */
        private function __construct()
        {
        }

        /**
         * Direction of the sort
         *
         * @param bool $descending
         *
         * @return SearchSortId
         */
        public function descending($descending)
        {
        }

        public function jsonSerialize()
        {
        }
    }

    /**
     * Sort by the hit score.
     */
    class SearchSortScore extends SearchSort implements JsonSerializable
    {
        /** @ignore */
        private function __construct()
        {
        }

        /**
         * Direction of the sort
         *
         * @param bool $descending
         *
         * @return SearchSortScore
         */
        public function descending($descending)
        {
        }

        public function jsonSerialize()
        {
        }
    }

    /**
     * Sort by a field in the hits.
     */
    class SearchSortField extends SearchSort implements JsonSerializable
    {
        const TYPE_AUTO = "auto";
        const TYPE_STRING = "string";
        const TYPE_NUMBER = "number";
        const TYPE_DATE = "date";

        const MODE_DEFAULT = "default";
        const MODE_MIN = "min";
        const MODE_MAX = "max";

        const MISSING_FIRST = "first";
        const MISSING_LAST = "last";

        /** @ignore */
        private function __construct()
        {
        }

        /**
         * Direction of the sort
         *
         * @param bool $descending
         *
         * @return SearchSortField
         */
        public function descending($descending)
        {
        }

        /**
         * Set type of the field
         *
         * @param string type the type
         *
         * @see SearchSortField::TYPE_AUTO
         * @see SearchSortField::TYPE_STRING
         * @see SearchSortField::TYPE_NUMBER
         * @see SearchSortField::TYPE_DATE
         */
        public function type($type)
        {
        }

        /**
         * Set mode of the sort
         *
         * @param string mode the mode
         *
         * @see SearchSortField::MODE_MIN
         * @see SearchSortField::MODE_MAX
         */
        public function mode($mode)
        {
        }

        /**
         * Set where the hits with missing field will be inserted
         *
         * @param string missing strategy for hits with missing fields
         *
         * @see SearchSortField::MISSING_FIRST
         * @see SearchSortField::MISSING_LAST
         */
        public function missing($missing)
        {
        }

        public function jsonSerialize()
        {
        }
    }

    /**
     * Sort by a location and unit in the hits.
     */
    class SearchSortGeoDistance extends SearchSort implements JsonSerializable
    {
        /** @ignore */
        private function __construct()
        {
        }

        /**
         * Direction of the sort
         *
         * @param bool $descending
         *
         * @return SearchSortGeoDistance
         */
        public function descending($descending)
        {
        }

        /**
         * Name of the units
         *
         * @param string $unit
         *
         * @return SearchSortGeoDistance
         */
        public function unit($unit)
        {
        }

        public function jsonSerialize()
        {
        }
    }

    /**
     * Interface for working with Full Text Search indexes.
     */
    class SearchIndexManager
    {
        /** @ignore */
        final private function __construct()
        {
        }

        /**
         * Returns list of currently defined search indexes.
         *
         * @return array of index definitions
         */
        public function listIndexDefinitions()
        {
        }

        /**
         * Retrieves search index definition by its name.
         *
         * @param string $name index name
         *
         * @return array representing index
         */
        public function getIndexDefinition($name)
        {
        }

        /**
         * Retrieves number of the documents currently covered by the index
         *
         * @param string $name index name
         *
         * @return int
         */
        public function getIndexDocumentsCount($name)
        {
        }

        /**
         * Creates search index with specified name and definition
         *
         * @param string $name index name
         * @param string $definition JSON-encoded index definition
         */
        public function createIndex($name, $definition)
        {
        }

        /**
         * Deletes search index by its name.
         *
         * @param string $name index name
         */
        public function deleteIndex($name)
        {
        }
    }
}

