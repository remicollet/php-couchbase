<?php
require_once('CouchbaseTestCase.php');

class TranscoderEncodingTest extends CouchbaseTestCase {
    function testBrokenEncoding() {
        if (phpversion() >= 7.0) {
            $options = array(
                'sertype' => COUCHBASE_SERTYPE_JSON,
                'cmprtype' => COUCHBASE_CMPRTYPE_NONE,
                'cmprthresh' => 0,
                'cmprfactor' => 0
            );
            $res = \Couchbase\basicEncoderV1(['ps¤cy'], $options);
            $this->assertEquals([null, 0x2000006, 0], $res);
            $this->assertEquals(JSON_ERROR_UTF8, json_last_error());
        }
    }
}
