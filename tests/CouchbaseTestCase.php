<?php

class CouchbaseTestCase extends \PHPUnit_Framework_TestCase {
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
            $this->testBucket = 'default';
        }

        $this->testUser = getenv('CPUSER');
        if ($this->testUser === FALSE) {
            $this->testUser = 'Administrator';
        }

        $this->testPass = getenv('CPPASS');
        if ($this->testPass === FALSE) {
            $this->testPass = 'password';
        }
    }

    function setTimeouts($bucket) {
        $val = getenv("CB_OPERATION_TIMEOUT");
        if ($val !== FALSE) {
            $bucket->operationTimeout = intval($val);
        } else {
            $bucket->operationTimeout = 5000000;
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
        if (getenv("REPORT_TIMEOUT_SETTINGS")) {
            printf("\n[TIMEOUTS] OT=%d, VT=%d, DI=%d, DT=%d, HT=%d, CT=%d, CD=%d, CNT=%d, HCIT=%d\n",
                   $bucket->operationTimeout,
                   $bucket->viewTimeout,
                   $bucket->durabilityInterval,
                   $bucket->durabilityTimeout,
                   $bucket->httpTimeout,
                   $bucket->configTimeout,
                   $bucket->configDelay,
                   $bucket->configNodeTimeout,
                   $bucket->htconfigIdleTimeout);
        }
    }

    function makeKey($prefix) {
        return uniqid($prefix);
    }

    function assertValidMetaDoc($metadoc) {
        $this->assertInstanceOf('\Couchbase\Document', $metadoc);

        // Check it has all the fields it should.
        for ($i = 1; $i < func_num_args(); ++$i) {
            $attr = func_get_arg($i);
            $this->assertObjectHasAttribute($attr, $metadoc);
            $this->assertNotNull($metadoc->{$attr}, "Expected document to have not NULL $attr");
        }
    }

    function assertErrorMetaDoc($metadoc, $type, $code) {
        $this->assertValidMetaDoc($metadoc, 'error');

        $this->assertInstanceOf($type, $metadoc->error);
        $this->assertEquals($code, $metadoc->error->getCode());
    }

    function wrapException($cb, $type = NULL, $code = NULL, $message = NULL) {
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
        if ($message !== NULL) {
            $this->assertErrorMessage($message, $exOut);
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

    function assertErrorMessage($msg, $ex) {
        $this->assertRegexp($msg, $ex->getMessage());
    }

    function assertErrorCode($code, $ex) {
        $this->assertEquals($code, $ex->getCode(), "Exception code does not match: {$ex->getCode()} != {$code}, exception message: '{$ex->getMessage()}'");
    }
}
