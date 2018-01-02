<?php

$cluster = new \Couchbase\Cluster('couchbase://localhost');
$cluster->authenticateAs("Administrator", "password");

$bucket = $cluster->openBucket("default");
$bucket->upsert("foo", ["bar" => 42]);
var_dump($bucket->diag("my_app"));
//=> array(4) {
//     ["id"]=>
//     string(21) "0x55e6c5977950/my_app"
//     ["kv"]=>
//     array(1) {
//       [0]=>
//       array(5) {
//         ["id"]=>
//         string(14) "0x55e6c597bd70"
//         ["last_activity_us"]=>
//         int(19)
//         ["local"]=>
//         string(15) "127.0.0.1:60136"
//         ["remote"]=>
//         string(15) "localhost:11210"
//         ["status"]=>
//         string(9) "connected"
//       }
//     }
//     ["sdk"]=>
//     string(46) "libcouchbase/2.8.4 PCBC/2.4.2 (PHP/7.1.12 NTS)"
//     ["version"]=>
//     int(1)
//   }
