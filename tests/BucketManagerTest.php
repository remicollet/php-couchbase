<?php
require_once('CouchbaseTestCase.php');

class BucketManagerTest extends CouchbaseTestCase {
    /**
     * @test
     * Test that a connection works and return manager instance.
     */
    function testConnect() {
        $h = new CouchbaseCluster($this->testDsn);
        $m = $h->openBucket()->manager();
        return $m;
    }

    /**
     * @depends testConnect
     * @expectedException CouchbaseException
     * @expectedExceptionMessageRegExp /EEXISTS/
     */
    function testIndexCreateDuplicates($m) {
        $m->createN1qlPrimaryIndex();
        $m->createN1qlPrimaryIndex();
    }

    /**
     * @depends testConnect
     * @expectedException CouchbaseException
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
}
