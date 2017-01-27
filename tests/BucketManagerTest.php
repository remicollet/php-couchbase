<?php
require_once('CouchbaseTestCase.php');

class BucketManagerTest extends CouchbaseTestCase {
    /**
     * @test
     * Test that a connection works and return manager instance.
     */
    function testConnect() {
        $h = new \Couchbase\Cluster($this->testDsn);
        $m = $h->openBucket($this->testBucket)->manager();
        return $m;
    }

    /**
     * @depends testConnect
     * @expectedException \Couchbase\Exception
     * @expectedExceptionMessageRegExp /EEXISTS/
     */
    function testIndexCreateDuplicates($m) {
        $m->createN1qlPrimaryIndex();
        $m->createN1qlPrimaryIndex();
    }

    /**
     * @depends testConnect
     * @expectedException \Couchbase\Exception
     * @expectedExceptionMessageRegExp /ENOENT/
     */
    function testIndexDropMissing($m) {
        $m->dropN1qlPrimaryIndex();
        $m->dropN1qlPrimaryIndex();
    }

    /**
     * @depends testConnect
     */
    function testBasicIndexLifecycle($m) {
        $name = $this->makeKey('user_name');
        $m->createN1qlIndex($name, array('`user.first_name`', '`user.last_name`'),
                            '`state` = "active"', false, false);

        $indexes = $m->listN1qlIndexes();
        $this->assertNotEmpty($indexes);

        $newIndex = NULL;
        foreach ($indexes as $index) {
            if ($index->name == $name) {
                $newIndex = $index;
            }
        }
        $this->assertNotNull($newIndex);
        $this->assertEquals(array('`user.first_name`', '`user.last_name`'),
                            $newIndex->fields);
        $this->assertEquals('(`state` = "active")', $newIndex->condition);
        $this->assertEquals(COUCHBASE_N1XSPEC_T_GSI, $newIndex->type);

        $m->dropN1qlIndex($name);
    }

    /**
     * @depends testConnect
     */
    function testBasicDesignDocumentLifecycle($m) {
        $name = $this->makeKey('user_name');
        $ddoc = [
            "views" => [
                "test" => [
                    "map" => 'function(doc, meta) { emit(meta.id); }',
                    "reduce" => "_count"
                ]
            ]
        ];
        $m->upsertDesignDocument($name, $ddoc);

        $documents = $m->listDesignDocuments();
        $this->assertNotEmpty($documents);

        $newDocument = NULL;
        foreach ($documents['rows'] as $document) {
            if ($document['doc']['meta']['id'] == "_design/$name") {
                $newDocument = $document;
            }
        }
        $this->assertNotNull($newDocument);
        $this->assertNotNull($newDocument['doc']['json']['views']['test']);

        $newDocument = $m->getDesignDocument($name);
        $this->assertNotNull($newDocument);
        $this->assertNotNull($newDocument['views']['test']);

        $m->removeDesignDocument($name);
    }

    /**
     * @depends testConnect
     * @expectedException \Couchbase\Exception
     * @expectedExceptionMessageRegExp /already exists/
     */
    function testInsertDesignDocumentDuplicate($m) {
        $name = $this->makeKey('user_name');
        $ddoc = [
            "views" => [
                "test" => [
                    "map" => 'function(doc, meta) { emit(meta.id); }',
                    "reduce" => "_count"
                ]
            ]
        ];
        $m->insertDesignDocument($name, $ddoc);
        $m->insertDesignDocument($name, $ddoc);
    }
}
