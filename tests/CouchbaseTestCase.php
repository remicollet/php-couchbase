<?php
class CouchbaseTestCase extends PHPUnit_Framework_TestCase {
    public $testDsn;
    public $testBucket;
    public $testUser;
    public $testPass;

    public function __construct() {
        $this->testDsn = getenv('CPDSN');
        if ($this->testDsn === FALSE) {
            $this->testDsn = 'couchbase://localhost/default';
        }

        $this->testBucket = getenv('CPBUCKET');
        if ($this->testBucket === FALSE) {
            $this->testBucket = '';
        }

        $this->testUser = getenv('CPUSER');
        if ($this->testUser === FALSE) {
            $this->testUser = '';
        }

        $this->testPass = getenv('CPPASS');
        if ($this->testPass === FALSE) {
            $this->testPass = '';
        }
    }

    function setTimeouts($bucket) {
        $val = getenv("CB_OPERATION_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->operationTimeout = intval($val);
        }
        $val = getenv("CB_VIEW_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->viewTimeout = intval($val);
        }
        $val = getenv("CB_DURABILITY_INTERVAL");
        if ($val !== FALSE) {
            $bucket->durabilityInterval = intval($val);
        }
        $val = getenv("CB_DURABILITY_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->durabilityTimeout = intval($val);
        }
        $val = getenv("CB_HTTP_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->httpTimeout = intval($val);
        }
        $val = getenv("CB_CONFIG_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->configTimeout = intval($val);
        }
        $val = getenv("CB_CONFIG_DELAY");
        if ($val !== FALSE) {
            $bucket->configDelay = intval($val);
        }
        $val = getenv("CB_CONFIG_NODE_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->configNodeTimeout = intval($val);
        }
        $val = getenv("CB_HTTP_CONFIG_IDLE_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->htconfigIdleTimeout = intval($val);
        }
    }

    function makeKey($prefix) {
        return uniqid($prefix);
    }

    function assertValidMetaDoc($metadoc) {
        // Note that this is only valid at the moment, in the future
        //   a MetaDoc might be a simple array, or other.
        $this->assertInstanceOf('CouchbaseMetaDoc', $metadoc);

        // Check it has all the fields it should.
        for ($i = 1; $i < func_num_args(); ++$i) {
            $attr = func_get_arg($i);
            $this->assertObjectHasAttribute($attr, $metadoc);
        }
    }

    function assertErrorMetaDoc($metadoc, $type, $code) {
        $this->assertValidMetaDoc($metadoc, 'error');

        $this->assertInstanceOf($type, $metadoc->error);
        $this->assertEquals($code, $metadoc->error->getCode());
    }

    function wrapException($cb, $type = NULL, $code = NULL) {
        PHPUnit_Framework_Error_Notice::$enabled = false;
        $exOut = NULL;
        try {
            $cb();
        } catch (Exception $ex) {
            $exOut = $ex;
        }
        PHPUnit_Framework_Error_Notice::$enabled = true;

        if ($type !== NULL) {
            $this->assertErrorType($type, $exOut);
        }
        if ($code !== NULL) {
            $this->assertErrorCode($code, $exOut);
        }

        return $exOut;
    }

    function assertError($type, $code, $ex) {
        $this->assertErrorType($type, $ex);
        $this->assertErrorCode($code, $ex);
    }

    function assertErrorType($type, $ex) {
        $this->assertInstanceOf($type, $ex);
    }

    function assertErrorCode($code, $ex) {
        $this->assertEquals($code, $ex->getCode());
    }
}
