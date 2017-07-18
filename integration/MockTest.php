<?php

const ERRCODE_CONSTANT = 0x7ff0;
const ERRCODE_LINEAR = 0x7ff1;
const ERRCODE_EXPONENTIAL = 0x7ff2;

class MockTest extends \PHPUnit_Framework_TestCase {

    protected $mock;

    protected function setUp() {
        $this->mock = new Mock();
        $this->mock->start();
        $this->mock->setCCCP(true);
    }

    protected function tearDown() {
        $this->mock->stop();
    }

    protected function checkRetryVerify($errorCode) {
        $cluster = new \Couchbase\Cluster($this->mock->connectionString(['enable_errmap' => true]));
        $bucket = $cluster->openBucket('default');

        // Establish data connection
        $bucket->upsert('key', 'var');

        $serverIdx = $this->mock->masterForKey('key');
        $this->mock->startRetryVerify($serverIdx);
        $this->mock->operationFailure($errorCode, -1, $serverIdx);

        $seenException = false;
        try {
            $bucket->upsert('key', 'var');
        } catch (\Couchbase\Exception $ex) {
            if ($ex->getCode() == COUCHBASE_GENERIC_TMPERR) {
                $seenException = true;
            }
        }
        $this->assertTrue($seenException, 'Expected upsert operation fail with COUCHBASE_GENERIC_TMPERR');

        $opcode = 0x01; // PROTOCOL_BINARY_CMD_SET
        $this->mock->checkRetryVerify($errorCode, $serverIdx, $opcode, 20);
    }

    public function testRetryVerifyConstant() {
        $this->checkRetryVerify(ERRCODE_CONSTANT);
    }

    public function testRetryVerifyLinear() {
        $this->checkRetryVerify(ERRCODE_LINEAR);
    }

    public function testRetryVerifyExponential() {
        $this->checkRetryVerify(ERRCODE_EXPONENTIAL);
    }
}

class Mock {
    private $jarPath;
    private $ctlServer;
    private $ctl;

    public function __construct() {
        $this->jarPath = join(DIRECTORY_SEPARATOR, [sys_get_temp_dir(), "CouchbaseMock.jar"]);
        $this->download();
    }

    public function download() {
        if (!file_exists($this->jarPath)) {
            $data = file_get_contents("http://packages.couchbase.com/clients/c/mock/CouchbaseMock-LATEST.jar");
            file_put_contents($this->jarPath, $data);
        }
    }

    public function start() {
        $this->ctlServer = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        socket_bind($this->ctlServer, '127.0.0.1');
        socket_listen($this->ctlServer);
        socket_getsockname($this->ctlServer, $addr, $port);

        $this->pid = pcntl_fork();
        if ($this->pid) {
            $this->ctl = socket_accept($this->ctlServer);
        } else {
            $rc = pcntl_exec("/usr/bin/java", [
                "-jar", $this->jarPath,
                "--harakiri-monitor", "{$addr}:{$port}",
                "--port", "0",
                "--replicas", "2",
                "--nodes", "4",
                "--buckets", "default::"
            ]);
            if (!$rc) {
                exit(0);
            }
        }

    }

    public function stop() {
        socket_close($this->ctl);
        socket_close($this->ctlServer);
    }

    protected function send($payload) {
        socket_write($this->ctl, json_encode($payload) . "\n");
        $response = socket_read($this->ctl, 100000, PHP_NORMAL_READ);
        $json = strstr($response, '{');
        if ($json && count($json)) {
            $decoded = json_decode($json, TRUE);
            if (array_key_exists('error', $decoded)) {
                throw new Exception($decoded['error']);
            }
            return $decoded;
        }
        return [];
    }

    public function info() {
        return $this->send(['command' => 'MOCKINFO']);
    }

    public function keyInfo($key, $bucket = 'default') {
        return $this->send([
            'command' => 'KEYINFO',
            'payload' => [
                'Key' => $key,
                'Bucket' => $bucket
            ]
        ])['payload'];
    }

    public function masterForKey($key, $bucket = 'default') {
        $payload = $this->keyInfo($key, $bucket);
        for ($idx = 0; $idx < count($payload); $idx++) {
            if ($payload[$idx] && $payload[$idx]['Conf']['Type'] == 'master') {
                return $idx;
            }
        }
        return -1;
    }

    public function setCCCP($enabled = true) {
        $this->send([
            'command' => 'SET_CCCP',
            'payload' => [
                'enabled' => boolval($enabled)
            ]
        ]);
    }

    public function connectionString($options = []) {
        $resp = $this->send(['command' => 'GET_MCPORTS']);
        $connstr = "";
        foreach ($resp["payload"] as $port) {
            if ($connstr == "") {
                $connstr = "couchbase://127.0.0.1:{$port}";
            } else {
                $connstr .= ",127.0.0.1:{$port}";
            }
        }
        if (count($options)) {
            $connstr .= '?' . http_build_query($options);
        }
        return $connstr;
    }

    public function operationFailure($protocolCode, $count = 1, $bucket = 'default', $serverIndexes = []) {
        $this->send([
            'command' => 'OPFAIL',
            'payload' => [
                'code' => $protocolCode,
                'count' => $count,
                'bucket' => $bucket,
                'servers' => $serverIndexes
            ]
        ]);
    }

    public function operationFailureClear($bucket = 'default', $serverIndexes = []) {
        $this->operationFailure(0, -1, $bucket, $serverIndexes);
    }

    public function startRetryVerify($serverIndex, $bucket = 'default') {
        $this->send([
            'command' => 'START_RETRY_VERIFY',
            'payload' => [
                'bucket' => $bucket,
                'idx' => $serverIndex
            ]
        ]);
    }

    public function checkRetryVerify($protocolCode, $serverIndex, $protocolOpcode, $fuzzMs, $bucket = 'default') {
        $this->send([
            'command' => 'START_RETRY_VERIFY',
            'payload' => [
                'opcode' => $protocolOpcode,
                'errcode' => $protocolCode,
                'fuzz_ms' => $fuzzMs,
                'bucket' => $bucket,
                'idx' => $serverIndex
            ]
        ]);
    }
}
