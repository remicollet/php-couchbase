<?php
/**
 * File for the CouchbaseCluster class.
 *
 * @author Brett Lawson <brett19@gmail.com>
 */

/**
 * Represents a cluster connection.
 *
 * @package Couchbase
 */
class CouchbaseCluster {
    /**
     * @var _CouchbaseCluster
     * @ignore
     *
     * Pointer to a manager instance if there is one.
     */
    private $_manager = NULL;

    /**
     * @var string
     * @ignore
     *
     * A cluster connection string to connect with.
     */
    private $_dsn;

    private $authenticator;

    /**
     * Sets a logger instance for the extension.
     *
     * @param Psr\Log\LoggerInterface $logger PSR-3 compatible logger
     */
    public static function setLogger($logger)
    {
        pcbc__internal_set_logger($logger);
    }

    /**
     * Creates a connection to a cluster.
     *
     * Creates a CouchbaseCluster object and begins the bootstrapping
     * process necessary for communications with the Couchbase Server.
     *
     * @param string $connstr A cluster connection string to connect with.
     * @param string $username The username for the cluster.
     * @param string $password The password for the cluster.
     *
     * @throws CouchbaseException
     */
    public function __construct($connstr = 'http://127.0.0.1/', $username = '', $password = '') {
        $this->_dsn = cbdsn_parse($connstr);
    }

    public function authenticate($authenticator) {
        $this->authenticator = $authenticator;
    }

    /**
     * Constructs a connection to a bucket.
     *
     * @param string $name The name of the bucket to open.
     * @param string $password The bucket password to authenticate with.
     * @return CouchbaseBucket A bucket object.
     *
     * @throws CouchbaseException
     *
     * @see CouchbaseBucket CouchbaseBucket
     */
    public function openBucket($name = 'default', $password = '') {
        $bucketDsn = cbdsn_normalize($this->_dsn);
        $bucketDsn['bucket'] = $name;
        if (!$password && $this->authenticator) {
            $password = $this->authenticator->getCredentials('bucket-kv', $name);
        }
        $dsnStr = cbdsn_stringify($bucketDsn);
        return new CouchbaseBucket($dsnStr, $name, $password, $this->authenticator);
    }

    /**
     * Creates a manager allowing the management of a Couchbase cluster.
     *
     * @param $username The administration username.
     * @param $password The administration password.
     * @return CouchbaseClusterManager
     */
    public function manager($username = '', $password = '') {
        if (!$this->_manager) {
            if (!($username && $password) && $this->authenticator) {
                $credentials = $this->authenticator->getCredentials('cluster-mgmt');
                $username = $credentials[0];
                $password = $credentials[1];
            }
            if (!($username && $password)) {
                throw InvalidArgumentException('invalid credentials for cluster manager');
            }
            $this->_manager = new CouchbaseClusterManager(
                cbdsn_stringify($this->_dsn), $username, $password);
        }
        return $this->_manager;
    }
}

class CouchbaseAuthenticator {
    private $adminUsername;
    private $adminPassword;
    private $buckets = array();

    public function setBucketCredentials($bucketName, $password) {
        $this->buckets[$bucketName] = $password;
    }

    public function setClusterCredentials($username, $password) {
        $this->adminUsername = $username;
        $this->adminPassword = $password;
    }

    public function getCredentials($context, $specific = null) {
        switch ($context) {
        case 'bucket-kv':
            return $this->buckets[$specific];
        case 'bucket-n1ql':
            return $this->buckets[$specific];
        case 'bucket-cbft':
            return $this->buckets;
        case 'cluster-n1ql':
            return $this->buckets;
        case 'cluster-mgmt':
            return array($this->adminUsername, $this->adminPassword);
        }
        throw new InvalidArgumentException("unable to get '$context' credentials, specific: $specific");
    }
}
