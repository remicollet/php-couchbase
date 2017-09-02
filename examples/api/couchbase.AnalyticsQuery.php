<?php
$cluster = new \Couchbase\Cluster('couchbase://localhost?detailed_errcodes=1');
$cluster->authenticateAs('Administrator', 'password');
$bucket = $cluster->openBucket('default');

$query = \Couchbase\AnalyticsQuery::fromString('SELECT "Hello, beer!" AS greeting;');
var_dump($res->rows[0]);
//=> object(stdClass)#4 (1) {
//     ["greeting"]=>
//     string(12) "Hello, beer!"
//   }
