<?php
require_once('CouchbaseTestCase.php');

class N1qlQueryTest extends CouchbaseTestCase {
    private $cluster;
    private $bucket;

    protected function setUp() {
        parent::setUp();
        $this->cluster = new \Couchbase\Cluster($this->testDsn);
        $this->cluster->authenticate($this->testAuthenticator);
        $this->bucket = $this->cluster->openBucket($this->testBucket);
        $this->setTimeouts($this->bucket);
    }

    function testAlreadyHaveCreatedIndex() {
        if ($this->usingMock()) {
            $this->markTestSkipped('N1QL indexes are not supported by the CouchbaseMock');
        }
        $this->bucket->manager()->dropN1qlPrimaryIndex('', true);
        $this->bucket->manager()->createN1qlPrimaryIndex();
    }

    function testResponseProperties() {
        if ($this->usingMock()) {
            $this->markTestSkipped('N1QL queries are not supported by the CouchbaseMock');
        }
        $key = $this->makeKey("n1qlResponseProperties");
        $this->bucket->upsert($key, ["bar" => 42]);
        $query = \Couchbase\N1qlQuery::fromString("SELECT * FROM `{$this->testBucket}` USE KEYS \"$key\"");
        $query->consistency(\Couchbase\N1qlQuery::REQUEST_PLUS);
        $res = $this->bucket->query($query);
        $this->assertNotEmpty($res->rows);
        $this->assertEquals(42, $res->rows[0]->{$this->testBucket}->bar);
        $this->assertNotNull($res->requestId);
        $this->assertNotNull($res->metrics);
        $this->assertNotNull($res->signature);
        $this->assertEquals("success", $res->status);

        $res = $this->bucket->query($query, true); // with arrays instead of stdObject
        $this->assertNotEmpty($res->rows);
        $this->assertEquals(42, $res->rows[0][$this->testBucket]['bar']);
    }

    function testParameters() {
        if ($this->usingMock()) {
            $this->markTestSkipped('N1QL queries are not supported by the CouchbaseMock');
        }
        $key = $this->makeKey("n1qlParameters");
        $bucketName = $this->testBucket;
        $this->bucket->upsert($key, ["bar" => 42]);

        $query = \Couchbase\N1qlQuery::fromString("SELECT * FROM `$bucketName` USE KEYS \$1");
        $query->consistency(\Couchbase\N1qlQuery::REQUEST_PLUS);
        $query->positionalParams([$key]);
        $res = $this->bucket->query($query);
        $this->assertNotEmpty($res->rows);
        $this->assertEquals(42, $res->rows[0]->{$bucketName}->bar);

        $query = \Couchbase\N1qlQuery::fromString("SELECT * FROM `$bucketName` USE KEYS \$key");
        $query->consistency(\Couchbase\N1qlQuery::REQUEST_PLUS);
        $query->namedParams(["key" => $key]);
        $res = $this->bucket->query($query);
        $this->assertNotEmpty($res->rows);
        $this->assertEquals(42, $res->rows[0]->{$bucketName}->bar);

        // it will use PHP interpolation, and actually breaks query
        $this->wrapException(function() use($bucketName, $key) {
            $query = \Couchbase\N1qlQuery::fromString("SELECT * FROM `$bucketName` USE KEYS $key");
            $query->consistency(\Couchbase\N1qlQuery::REQUEST_PLUS);
            $query->namedParams(["key" => $key]);
            $res = $this->bucket->query($query);
        }, '\Couchbase\Exception', 3000, '/Ambiguous reference to field/');
    }

    function testAtPlus() {
        if ($this->usingMock()) {
            $this->markTestSkipped('N1QL queries are not supported by the CouchbaseMock');
        }
        $bucketName = $this->testBucket;
        $cluster = new \Couchbase\Cluster($this->testDsn . '?fetch_mutation_tokens=true');
        $cluster->authenticate($this->testAuthenticator);
        $bucket = $cluster->openBucket($bucketName);
        $this->setTimeouts($bucket);

        $key = $this->makeKey("atPlus");
        $random = rand(0, 1000000);
        $result = $bucket->upsert($key, array(
            "name" => array("Brass", "Doorknob"),
            "email" => "brass.doorknob@example.com",
            "random" => $random)
        );
        // construct mutation state from the list of mutation results
        $mutationState = \Couchbase\MutationState::from(array($result));
        $query = \Couchbase\N1qlQuery::fromString("SELECT name, random, META($bucketName).id FROM `$bucketName` WHERE \$1 IN name");
        $query->positionalParams(['Brass']);
        $query->consistentWith($mutationState);
        $result = $bucket->query($query);
        $found = false;
        foreach ($result->rows as $row) {
            if ($row->random == $random) {
                $found = true;
            }
        }
        $this->assertTrue($found, "The record \"$key\" is missing in the result set");
    }
}
