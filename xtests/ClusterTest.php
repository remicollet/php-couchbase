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

    function testCertAuthenticator() {
        $h = new \Couchbase\Cluster($this->testDsn);
        $h->authenticate(new \Couchbase\CertAuthenticator());
        $this->wrapException(function() use($h) {
            $h->openBucket('default');
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/Mixed authentication detected/');

        $h = new \Couchbase\Cluster($this->testDsn . '?keypath=/tmp/foo.key');
        $h->authenticate(new \Couchbase\CertAuthenticator());
        $this->wrapException(function() use($h) {
            $h->openBucket('default');
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/Mixed authentication detected/');

        $h = new \Couchbase\Cluster($this->testDsn . '?certpath=/tmp/foo.cert');
        $h->authenticate(new \Couchbase\CertAuthenticator());
        $this->wrapException(function() use($h) {
            $h->openBucket('default');
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/Mixed authentication detected/');

        $h = new \Couchbase\Cluster($this->testDsn . '?keypath=/tmp/foo.key&certpath=/tmp/foo.cert');
        $h->authenticate(new \Couchbase\CertAuthenticator());
        $this->wrapException(function() use($h) {
            $h->openBucket('default', 'mypassword');
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/Mixed authentication detected/');

        $h = new \Couchbase\Cluster($this->testDsn . '?keypath=/tmp/foo.key&certpath=/tmp/foo.cert');
        $this->wrapException(function() use($h) {
            $h->openBucket('default');
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/Mixed authentication detected/');

        $h = new \Couchbase\Cluster($this->testDsn . '?keypath=/tmp/foo.key&certpath=/tmp/foo.cert');
        $h->authenticateAs($this->testAdminUser, $this->testAdminPassword);
        $this->wrapException(function() use($h) {
            $h->openBucket('default');
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/Mixed authentication detected/');



        $h = new \Couchbase\Cluster($this->testDsn . '?keypath=/tmp/foo.key&certpath=/tmp/foo.cert');
        $h->authenticate(new \Couchbase\CertAuthenticator());
        $this->wrapException(function() use($h) {
            $h->openBucket('default');
            /* lcb complains about paths are not being real files */
        }, '\Couchbase\Exception', COUCHBASE_EINVAL, '/LCB_EINVAL/');
    }
}
