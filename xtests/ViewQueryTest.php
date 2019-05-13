<?php
require_once('CouchbaseTestCase.php');

class ViewQueryTest extends CouchbaseTestCase {
    protected function setUp() {
        parent::setUp();
        $this->cluster = new \Couchbase\Cluster($this->testDsn);
        $this->cluster->authenticate($this->testAuthenticator);
        $this->bucket = $this->cluster->openBucket($this->testBucket);
        $this->manager = $this->bucket->manager();
        $this->setTimeouts($this->bucket);
    }

    function testConsistency() {
        if ($this->usingMock()) {
            $this->markTestSkipped('View consistency is not supported by the CouchbaseMock');
        }
        $ddocName = $this->makeKey('testConsistency');
        $ddoc = [
            'views' => [
                'test' => [
                    'map' => "function(doc, meta) { if (meta.id.startsWith(\"{$ddocName}\")) emit(meta.id); }"
                ],
            ]
        ];
        $this->manager->upsertDesignDocument($ddocName, $ddoc);
        sleep(1); // give design document a second to settle

        $key = $this->makeKey($ddocName);
        $this->bucket->upsert($key, ['foo' => 42]);

        $query = \Couchbase\ViewQuery::from($ddocName, 'test');
        $res = $this->bucket->query($query);
        $this->assertEmpty($res->rows);

        $query->consistency(\Couchbase\ViewQuery::UPDATE_BEFORE);
        $res = $this->bucket->query($query);
        $this->assertCount(1, $res->rows);
        $this->assertEquals($key, $res->rows[0]->id);

        $this->manager->removeDesignDocument($ddocName);
    }

    function testGrouping() {
        $ddocName = $this->makeKey('ViewQueryTest');
        $ddoc = [
            'views' => [
                'test' => [
                    'map' => "function(doc, meta) { if (doc.ddoc == \"{$ddocName}\") emit([doc.country, doc.city]); }",
                    'reduce' => '_count'
                ]
            ]
        ];
        $this->manager->upsertDesignDocument($ddocName, $ddoc);
        sleep(1); // give design document a second to settle

        $this->bucket->upsert($this->makeKey($ddocName), ['ddoc' => $ddocName, 'country' => 'USA', 'city' => 'New York', 'name' => 'John Doe']);
        $this->bucket->upsert($this->makeKey($ddocName), ['ddoc' => $ddocName, 'country' => 'USA', 'city' => 'New York', 'name' => 'Jane Doe']);
        $this->bucket->upsert($this->makeKey($ddocName), ['ddoc' => $ddocName, 'country' => 'USA', 'city' => 'Miami', 'name' => 'Bill Brown']);
        $this->bucket->upsert($this->makeKey($ddocName), ['ddoc' => $ddocName, 'country' => 'France', 'city' => 'Paris', 'name' => 'Jean Bon']);

        $query = \Couchbase\ViewQuery::from($ddocName, 'test');
        $query->consistency(\Couchbase\ViewQuery::UPDATE_BEFORE);
        $res = $this->bucket->query($query);
        $this->assertCount(1, $res->rows);
        $this->assertEquals(4, $res->rows[0]->value);

        $query = \Couchbase\ViewQuery::from($ddocName, 'test');
        $query->consistency(\Couchbase\ViewQuery::UPDATE_BEFORE);
        $query->groupLevel(1);
        $res = $this->bucket->query($query);
        $this->assertCount(2, $res->rows);
        $this->assertEquals(["France"], $res->rows[0]->key);
        $this->assertEquals(1, $res->rows[0]->value);
        $this->assertEquals(["USA"], $res->rows[1]->key);
        $this->assertEquals(3, $res->rows[1]->value);

        $query = \Couchbase\ViewQuery::from($ddocName, 'test');
        $query->consistency(\Couchbase\ViewQuery::UPDATE_BEFORE);
        $query->group(true);
        $res = $this->bucket->query($query);
        $this->assertCount(3, $res->rows);
        $this->assertEquals(["France", "Paris"], $res->rows[0]->key);
        $this->assertEquals(1, $res->rows[0]->value);
        $this->assertEquals(["USA", "Miami"], $res->rows[1]->key);
        $this->assertEquals(1, $res->rows[1]->value);
        $this->assertEquals(["USA", "New York"], $res->rows[2]->key);
        $this->assertEquals(2, $res->rows[2]->value);

        $query = \Couchbase\ViewQuery::from($ddocName, 'test');
        $query->consistency(\Couchbase\ViewQuery::UPDATE_BEFORE);
        $query->reduce(false)->keys(array_values([['USA', 'New York']]));
        $res = $this->bucket->query($query);
        $this->assertCount(2, $res->rows);

        $this->manager->removeDesignDocument($ddocName);
    }
}
