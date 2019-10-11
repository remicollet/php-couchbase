<?php

$cluster = new \Couchbase\Cluster('couchbase://localhost');
$cluster->authenticateAs("Administrator", "password");

$bucket = $cluster->bucket("default");
$bucket->upsert("foo", ["bar" => 42]);
var_dump($bucket->ping(\Couchbase\Bucket::PINGSVC_KV));
//=> array(5) {
//     ["config_rev"]=>
//     int(39)
//     ["id"]=>
//     string(9) "0x16c1290"
//     ["sdk"]=>
//     string(46) "libcouchbase/2.8.4 PCBC/2.4.2 (PHP/5.6.32 ZTS)"
//     ["services"]=>
//     array(1) {
//       ["kv"]=>
//       array(1) {
//         [0]=>
//         array(6) {
//           ["id"]=>
//           string(9) "0x16c58b0"
//           ["latency_us"]=>
//           int(94)
//           ["local"]=>
//           string(15) "127.0.0.1:59876"
//           ["remote"]=>
//           string(15) "localhost:11210"
//           ["scope"]=>
//           string(7) "default"
//           ["status"]=>
//           string(2) "ok"
//         }
//       }
//     }
//     ["version"]=>
//     int(1)
//   }
