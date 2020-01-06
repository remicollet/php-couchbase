<?php
require_once('CouchbaseTestCase.php');

class DatastructuresTest extends CouchbaseTestCase {
    private $cluster;
    private $bucket;

    protected function setUp() {
        parent::setUp();
        $this->cluster = new \Couchbase\Cluster($this->testDsn);
        $this->cluster->authenticate($this->testAuthenticator);
        $this->bucket = $this->cluster->bucket($this->testBucket);
        $this->setTimeouts($this->bucket);
    }

    function testMap() {
        $key = $this->makeKey("datastructuresMap");
        $this->bucket->upsert($key, ["name" => "John"]);

        $this->assertEquals(1, $this->bucket->mapSize($key));
        $this->assertEquals("John", $this->bucket->mapGet($key, "name"));

        $this->bucket->mapAdd($key, "age", 42);
        $this->assertEquals(2, $this->bucket->mapSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals("John", $doc->value->name);
        $this->assertEquals(42, $doc->value->age);

        $this->bucket->mapRemove($key, "name");
        $this->assertEquals(1, $this->bucket->mapSize($key));
        $doc = $this->bucket->get($key);
        $this->assertObjectNotHasAttribute("name", $doc);
        $this->assertEquals(42, $doc->value->age);
    }

    function testList() {
        $key = $this->makeKey("datastructuresList");
        $this->bucket->upsert($key, [1, 2]);
        $this->assertEquals(2, $this->bucket->listSize($key));

        $this->bucket->listPush($key, 3);
        $this->assertEquals(3, $this->bucket->listSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals([1, 2, 3], $doc->value);

        $this->bucket->listShift($key, 42);
        $this->assertEquals(4, $this->bucket->listSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals([42, 1, 2, 3], $doc->value);

        $this->bucket->listRemove($key, 0);
        $this->assertEquals(3, $this->bucket->listSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals([1, 2, 3], $doc->value);

        $res = $this->bucket->listGet($key, 2);
        $this->assertEquals(3, $res);

        $this->bucket->listSet($key, 1, 42);
        $this->assertEquals(3, $this->bucket->listSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals([1, 42, 3], $doc->value);

    }

    function testSet() {
        $key = $this->makeKey("datastructuresSet");
        $this->bucket->upsert($key, ["hello"]);
        $this->assertEquals(1, $this->bucket->setSize($key));

        $this->bucket->setAdd($key, "world");
        $this->assertEquals(2, $this->bucket->setSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals(["hello", "world"], $doc->value, 'Sets must be equal', 0.0, 1, true);

        $this->bucket->setAdd($key, "hello");
        $this->assertEquals(2, $this->bucket->setSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals(["hello", "world"], $doc->value, 'Sets must be equal', 0.0, 1, true);

        $res = $this->bucket->setExists($key, "hello");
        $this->assertTrue($res, "Expected set to contain \"hello\"");

        $res = $this->bucket->setRemove($key, "hello");
        $this->assertTrue($res, "Expected successful removal of \"hello\"");

        $res = $this->bucket->setExists($key, "hello");
        $this->assertFalse($res, "Expected set not to contain \"hello\"");

        $res = $this->bucket->setRemove($key, "hello");
        $this->assertFalse($res, "Expected failure removal of \"hello\"");
    }

    function testQueue() {
        $key = $this->makeKey("datastructuresQueue");
        $this->bucket->upsert($key, ["hello"]);
        $this->assertEquals(1, $this->bucket->queueSize($key));

        $this->bucket->queueAdd($key, "world");
        $this->assertEquals(2, $this->bucket->queueSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals(["world", "hello"], $doc->value);

        $res = $this->bucket->queueRemove($key);
        $this->assertEquals("hello", $res);
        $this->assertEquals(1, $this->bucket->queueSize($key));
        $doc = $this->bucket->get($key);
        $this->assertEquals(["world"], $doc->value);
    }

    function testMissingKeys() {
        $key = $this->makeKey("datastructuresMissingKey");

        $this->wrapException(function() use($key) {
            $res = $this->bucket->queueSize($key);
        }, '\Couchbase\Exception', COUCHBASE_ERR_DOCUMENT_NOT_FOUND);

        $this->wrapException(function() use($key) {
            $res = $this->bucket->setExists($key, 42);
        }, '\Couchbase\Exception', COUCHBASE_ERR_DOCUMENT_NOT_FOUND);

        $this->wrapException(function() use($key) {
            $res = $this->bucket->setRemove($key, 42);
        }, '\Couchbase\Exception', COUCHBASE_ERR_DOCUMENT_NOT_FOUND);
    }
}
