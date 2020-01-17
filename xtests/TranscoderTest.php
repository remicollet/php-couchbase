<?php
require_once('CouchbaseTestCase.php');

class TranscoderTest extends CouchbaseTestCase {
    function testPassthruEncoder() {
        $res = \Couchbase\passthruEncoder("foobar");
        $this->assertEquals(["foobar", 0, 0], $res);

        $res = \Couchbase\passthruEncoder('{"foo": 1}');
        $this->assertEquals(['{"foo": 1}', 0, 0], $res);

        $res = \Couchbase\passthruEncoder(NULL);
        $this->assertEquals([NULL, 0, 0], $res);
    }

    function testPassthruDecoder() {
        $res = \Couchbase\passthruDecoder("foobar", 0, 0);
        $this->assertEquals("foobar", $res);

        $res = \Couchbase\passthruDecoder("foobar", 0xdeadbeef, 42);
        $this->assertEquals("foobar", $res);

        $res = \Couchbase\passthruDecoder(["foo" => 1], 0, 0);
        $this->assertEquals(["foo" => 1], $res);

        $res = \Couchbase\passthruDecoder(NULL, 0, 0);
        $this->assertEquals(NULL, $res);
    }

    /**
     * For Common Flags encoding see RFC-20 at
     * https://github.com/couchbaselabs/sdk-rfcs
     */
    function testCouchbaseBasicEncoderV1() {
        $options = array(
            'sertype' => COUCHBASE_SERTYPE_JSON,
            'cmprtype' => COUCHBASE_CMPRTYPE_NONE,
            'cmprthresh' => 0,
            'cmprfactor' => 0
        );

        $res = \Couchbase\basicEncoderV1("foo", $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_STRING, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_STRING, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["foo", 0x04000000, 0], $res);

        $res = \Couchbase\basicEncoderV1(1, $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_LONG, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["1", 0x02000001, 0], $res);

        $res = \Couchbase\basicEncoderV1(0xabcdef0123456789, $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_DOUBLE, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["1.2379813738877E+19", 0x02000002, 0], $res);

        $res = \Couchbase\basicEncoderV1(1.0, $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_DOUBLE, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["1.0", 0x02000002, 0], $res);

        $res = \Couchbase\basicEncoderV1(true, $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_BOOL, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["true", 0x02000003, 0], $res);

        $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        if (phpversion() >= 7.1) {
            $this->assertEquals(['{"foo":1.2379813738877118e+19}', 0x02000006, 0], $res);
        } else {
            $this->assertEquals(['{"foo":1.2379813738877e+19}', 0x02000006, 0], $res);
        }

        $res = \Couchbase\basicEncoderV1([1, 2, 3], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(['[1,2,3]', 0x02000006, 0], $res);

        $res = \Couchbase\basicEncoderV1([], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(['[]', 0x02000006, 0], $res);

        $res = \Couchbase\basicEncoderV1((object)[], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(['{}', 0x02000006, 0], $res);

        $options['sertype'] = COUCHBASE_SERTYPE_PHP;
        $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_PRIVATE, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_SERIALIZED, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(['a:1:{s:3:"foo";d:1.2379813738877118E+19;}', 0x01000004, 0], $res);

        // serializer type does not affect serialization of the primitives.
        // they are always JSON-like strings
        $res = \Couchbase\basicEncoderV1("foo", $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_STRING, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_STRING, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["foo", 0x04000000, 0], $res);

        $res = \Couchbase\basicEncoderV1(1.0, $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_DOUBLE, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(["1.0", 0x02000002, 0], $res);

        $options['sertype'] = COUCHBASE_SERTYPE_JSON;
        if (\Couchbase\HAVE_ZLIB) {
            $options['cmprtype'] = COUCHBASE_CMPRTYPE_ZLIB;

            $options['cmprthresh'] = 40;

            $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options);
            $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
            $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
            $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
            if (phpversion() >= 7.1) {
                $this->assertEquals(30, strlen($res[0]));
                $this->assertEquals(['{"foo":1.2379813738877118e+19}', 0x02000006, 0], $res);
            } else {
                $this->assertEquals(27, strlen($res[0]));
                $this->assertEquals(['{"foo":1.2379813738877e+19}', 0x02000006, 0], $res);
            }

            $options['cmprthresh'] = 20;

            $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options);
            $this->assertEquals(COUCHBASE_COMPRESSION_ZLIB, $res[1] & COUCHBASE_COMPRESSION_MASK);
            $this->assertEquals(COUCHBASE_COMPRESSION_MCISCOMPRESSED, $res[1] & COUCHBASE_COMPRESSION_MCISCOMPRESSED);
            $this->assertEquals(COUCHBASE_CFFMT_PRIVATE, $res[1] & COUCHBASE_CFFMT_MASK);
            $this->assertEquals(COUCHBASE_VAL_IS_JSON | COUCHBASE_COMPRESSION_MCISCOMPRESSED, $res[1] & COUCHBASE_VAL_MASK);
            $this->assertEquals(COUCHBASE_VAL_IS_JSON, ($res[1] & ~COUCHBASE_COMPRESSION_MCISCOMPRESSED) & COUCHBASE_VAL_MASK);
            if (phpversion() >= 7.1) {
                $this->assertEquals(42, strlen($res[0]));
                $this->assertEquals([hex2bin('1e000000789cab564acbcf57b232d4333236b7b430343637b6b030373734b448d536b4ac050076a00767'), 0x01000036, 0], $res);
            } else {
                $this->assertEquals(39, strlen($res[0]));
                $this->assertEquals([hex2bin('1b000000789cab564acbcf57b232d4333236b7b430343637b6b030374fd536b4ac0500626f06cd'), 0x01000036, 0], $res);
            }

            $res = \Couchbase\basicEncoderV1('{"foo": 12379813738877118345}', $options);
            $this->assertEquals(COUCHBASE_COMPRESSION_ZLIB, $res[1] & COUCHBASE_COMPRESSION_MASK);
            $this->assertEquals(COUCHBASE_COMPRESSION_MCISCOMPRESSED, $res[1] & COUCHBASE_COMPRESSION_MCISCOMPRESSED);
            $this->assertEquals(COUCHBASE_CFFMT_PRIVATE, $res[1] & COUCHBASE_CFFMT_MASK);
            $this->assertEquals(COUCHBASE_VAL_IS_STRING | COUCHBASE_COMPRESSION_MCISCOMPRESSED, $res[1] & COUCHBASE_VAL_MASK);
            $this->assertEquals(COUCHBASE_VAL_IS_STRING, ($res[1] & ~COUCHBASE_COMPRESSION_MCISCOMPRESSED) & COUCHBASE_VAL_MASK);
            $this->assertEquals(41, strlen($res[0]));
            $this->assertEquals([hex2bin('1d000000789cab564acbcf57b25230343236b7b430343637b6b030373734b4303631ad05006da106fb'), 0x01000030, 0], $res);
        }

        $options['cmprtype'] = COUCHBASE_CMPRTYPE_FASTLZ;
        $options['cmprthresh'] = 40;

        $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        if (phpversion() >= 7.1) {
            $this->assertEquals(30, strlen($res[0]));
            $this->assertEquals(['{"foo":1.2379813738877118e+19}', 0x02000006, 0], $res);
        } else {
            $this->assertEquals(27, strlen($res[0]));
            $this->assertEquals(['{"foo":1.2379813738877e+19}', 0x02000006, 0], $res);
        }

        $options['cmprthresh'] = 20;

        $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options);
        $this->assertEquals(COUCHBASE_COMPRESSION_FASTLZ, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_COMPRESSION_MCISCOMPRESSED, $res[1] & COUCHBASE_COMPRESSION_MCISCOMPRESSED);
        $this->assertEquals(COUCHBASE_CFFMT_PRIVATE, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON | COUCHBASE_COMPRESSION_MCISCOMPRESSED, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, ($res[1] & ~COUCHBASE_COMPRESSION_MCISCOMPRESSED) & COUCHBASE_VAL_MASK);
        if (phpversion() >= 7.1) {
            $this->assertEquals(35, strlen($res[0]));
            $this->assertEquals([hex2bin('1e0000001d7b22666f6f223a312e32333739383133373338383737313138652b31397d'), 0x01000056, 0], $res);
        } else {
            $this->assertEquals(32, strlen($res[0]));
            $this->assertEquals([hex2bin('1b0000001a7b22666f6f223a312e32333739383133373338383737652b31397d'), 0x01000056, 0], $res);
        }

        $options['cmprfactor'] = 1.0; // it should be at least 1 byte less than source
        $res = \Couchbase\basicEncoderV1(["foo" => 0xabcdef0123456789], $options); // 27 < 32 * 1.0
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        if (phpversion() >= 7.1) {
            $this->assertEquals(30, strlen($res[0]));
            $this->assertEquals(['{"foo":1.2379813738877118e+19}', 0x02000006, 0], $res);
        } else {
            $this->assertEquals(27, strlen($res[0]));
            $this->assertEquals(['{"foo":1.2379813738877e+19}', 0x02000006, 0], $res);
        }
    }

    function testCouchbaseBasicDecoderV1() {
        $options = [
            'jsonassoc' => false
        ];

        $res = \Couchbase\basicDecoderV1("foo", 0x04000000, 0, $options);
        $this->assertEquals("foo", $res);

        $res = \Couchbase\basicDecoderV1("1", 0x02000001, 0, $options);
        $this->assertEquals(1, $res);

        $original = 0xabcdef0123456789;

        $encoder_options = [
            'sertype' => COUCHBASE_SERTYPE_JSON,
            'cmprtype' => COUCHBASE_CMPRTYPE_NONE,
            'cmprthresh' => 0,
            'cmprfactor' => 0
        ];
        $res = \Couchbase\basicEncoderV1($original, $encoder_options);
        $serialized = $res[0];
        $this->assertEquals("1.2379813738877E+19", $serialized);
        $deserialized = \Couchbase\basicDecoderV1($serialized, 0x02000002, 0, $options);
        $this->assertNotEquals($deserialized, $original);
        $this->assertEquals(0xabcdef0123439800, $deserialized);

        $res = \Couchbase\basicDecoderV1("1.0", 0x02000002, 0, $options);
        $this->assertEquals(1.0, $res);

        $res = \Couchbase\basicDecoderV1("true", 0x02000003, 0, $options);
        $this->assertEquals(true, $res);

        $res = \Couchbase\basicDecoderV1('{"foo":1.2379813738877e+19}', 0x02000006, 0, $options);
        $this->assertEquals((object)["foo" => 0xabcdef0123439800], $res);

        // decodes strings from PHP7 as well
        $res = \Couchbase\basicDecoderV1('{"foo":1.2379813738877118e+19}', 0x02000006, 0, $options);
        $this->assertEquals((object)["foo" => 0xabcdef0123456789], $res);

        // legacy JSON document with zero flags should be detected
        $res = \Couchbase\basicDecoderV1('{"foo":1.2379813738877e+19}', 0x0000000, 0, $options);
        $this->assertEquals((object)["foo" => 0xabcdef0123439800], $res);

        $res = \Couchbase\basicDecoderV1('{"foo":1.2379813738877e+19', 0x0000000, 0, $options);
        $this->assertEquals('{"foo":1.2379813738877e+19', $res);

        $res = \Couchbase\basicDecoderV1('[1,2,3]', 0x02000006, 0, $options);
        $this->assertEquals([1, 2, 3], $res);

        $res = \Couchbase\basicDecoderV1('[]', 0x02000006, 0, $options);
        $this->assertEquals([], $res);

        $res = \Couchbase\basicDecoderV1('{}', 0x02000006, 0, $options);
        $this->assertEquals((object)[], $res);

        $options['jsonassoc'] = true;
        $res = \Couchbase\basicDecoderV1('{"foo":1.2379813738877e+19}', 0x02000006, 0, $options);
        $this->assertEquals(["foo" => 0xabcdef0123439800], $res);

        $res = \Couchbase\basicDecoderV1('a:1:{s:3:"foo";d:1.2379813738877118E+19;}', 0x01000004, 0, $options);
        $this->assertEquals(["foo" => 0xabcdef0123456789], $res);

        $res = \Couchbase\basicDecoderV1(hex2bin("0000000214011103666f6f0c43e579bde02468ad"), 0x01000005, 0, $options);
        $this->assertEquals(NULL, $res);

        $options['jsonassoc'] = false;  // jsonassoc does not affect "binary" encoders
        $res = \Couchbase\basicDecoderV1('a:1:{s:3:"foo";d:1.2379813738877118E+19;}', 0x01000004, 0, $options);
        $this->assertEquals(["foo" => 0xabcdef0123456789], $res);

        $res = \Couchbase\basicDecoderV1(hex2bin("0000000214011103666f6f0c43e579bde02468ad"), 0x01000005, 0, $options);
        $this->assertEquals(NULL, $res);

        $res = \Couchbase\basicDecoderV1("1.0", 0x02000002, 0, $options);
        $this->assertEquals(1.0, $res);

        $options['jsonassoc'] = true;
        if (\Couchbase\HAVE_ZLIB) {
            $res = \Couchbase\basicDecoderV1(hex2bin('1b000000789cab564acbcf57b232d4333236b7b430343637b6b030374fd536b4ac0500626f06cd'), 0x01000036, 0, $options);
            $this->assertEquals(["foo" => 0xabcdef0123439800], $res);

            $res = \Couchbase\basicDecoderV1(hex2bin('1d000000789cab564acbcf57b25230343236b7b430343637b6b030373734b4303631ad05006da106fb'), 0x01000030, 0, $options);
            $this->assertEquals('{"foo": 12379813738877118345}', $res);
        }

        $res = \Couchbase\basicDecoderV1(hex2bin('1b0000001a7b22666f6f223a312e32333739383133373338383737652b31397d'), 0x01000056, 0, $options);
        $this->assertEquals(["foo" => 0xabcdef0123439800], $res);

        $res = \Couchbase\basicDecoderV1(hex2bin('1e0000001d7b22666f6f223a312e32333739383133373338383737313138652b31397d'), 0x01000056, 0, $options);
        $this->assertEquals(["foo" => 0xabcdef0123456789], $res);

        $res = \Couchbase\basicDecoderV1('{"foo":"bar"', 0x04000000, 0, $options);
        $this->assertEquals('{"foo":"bar"', $res);

        $res = \Couchbase\basicDecoderV1('{"foo":"bar"', 0x02000000, 0, $options);
        $this->assertEquals(null, $res);
        $this->assertEquals(JSON_ERROR_SYNTAX, json_last_error());

        $res = \Couchbase\basicDecoderV1('{"foo":"bar"}{"baz":42}', 0x02000000, 0, $options);
        $this->assertEquals(null, $res);
        $this->assertEquals(JSON_ERROR_SYNTAX, json_last_error());
    }

    function testInvalidInputForPhpSerialize() {
        $res = \Couchbase\basicDecoderV1('foobar', 0x01000004, 0, []);
        $this->assertNull($res);
    }

    function testCouchbaseDefaultEncoder() {
        $orig = ini_get('couchbase.encoder.format');
        ini_set('couchbase.encoder.format', 'json');
        $res = \Couchbase\defaultEncoder(["foo" => 0xabcdef0123456789]);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_JSON, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_JSON, $res[1] & COUCHBASE_VAL_MASK);
        if (phpversion() >= 7.1) {
            $this->assertEquals(['{"foo":1.2379813738877118e+19}', 0x02000006, 0], $res);
        } else {
            $this->assertEquals(['{"foo":1.2379813738877e+19}', 0x02000006, 0], $res);
        }

        ini_set('couchbase.encoder.format', 'php');
        $res = \Couchbase\defaultEncoder(["foo" => 0xabcdef0123456789]);
        $this->assertEquals(COUCHBASE_COMPRESSION_NONE, $res[1] & COUCHBASE_COMPRESSION_MASK);
        $this->assertEquals(COUCHBASE_CFFMT_PRIVATE, $res[1] & COUCHBASE_CFFMT_MASK);
        $this->assertEquals(COUCHBASE_VAL_IS_SERIALIZED, $res[1] & COUCHBASE_VAL_MASK);
        $this->assertEquals(['a:1:{s:3:"foo";d:1.2379813738877118E+19;}', 0x01000004, 0], $res);
        ini_set('couchbase.encoder.format', $orig);
    }

    function testCouchbaseDefaultDecoder() {
        $orig = ini_get('couchbase.decoder.json_arrays');
        ini_set('couchbase.decoder.json_array', false);
        $res = \Couchbase\defaultDecoder('{"foo":1.2379813738877e+19}', 0x02000006, 0);
        $this->assertEquals($res, (object)["foo" => 0xabcdef0123439800]);

        ini_set('couchbase.decoder.json_arrays', true);
        $res = \Couchbase\defaultDecoder('{"foo":1.2379813738877e+19}', 0x02000006, 0);
        $this->assertEquals(["foo" => 0xabcdef0123439800], $res);

        ini_set('couchbase.decoder.json_arrays', $orig);
    }
}
