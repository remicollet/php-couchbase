<?php
require_once('CouchbaseTestCase.php');

class ClusterTest extends CouchbaseTestCase {
    /**
     * @test
     * Test that connections with invalid details fail.
     *
     * @expectedException \Couchbase\Exception
     * @expectedExceptionMessageRegExp /UNKNOWN_HOST/
     */
    function testBadHost() {
        $h = new \Couchbase\Cluster('couchbase://999.99.99.99');
        $h->openBucket('default');
    }

    /**
     * @test
     * Test that connections with invalid details fail.
     *
     * @expectedException \Couchbase\Exception
     * @expectedExceptionMessageRegExp /AUTH_ERROR/
     */
    function testBadPass() {
        $h = new \Couchbase\Cluster($this->testDsn);
        $h->openBucket('default', 'badpass');
    }
}
