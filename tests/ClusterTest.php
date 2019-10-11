<?php
require_once('CouchbaseTestCase.php');

class ClusterTest extends CouchbaseTestCase {
    /**
     * @test
     * Test that connections with invalid details fail.
     *
     * @expectedException \Couchbase\BaseException
     * @expectedExceptionMessageRegExp /UNKNOWN_HOST/
     */
    function testBadHost() {
        ini_set('couchbase.log_level', 'FATAL');
        $options = new \Couchbase\ClusterOptions();
        $options->credentials($this->testUser, $this->testPassword);
        new \Couchbase\Cluster('couchbase://999.99.99.99', $options);
    }
}
