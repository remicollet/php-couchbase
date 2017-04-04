<?php
$cluster = new \Couchbase\Cluster('couchbase://localhost?detailed_errcodes=1');
$bucket = $cluster->openBucket('beer-sample');

$query = \Couchbase\AnalyticsQuery::fromString('SELECT "Hello, beer!" AS greeting;');
$query->hostname('localhost:8095/query/service'); // while the feature in experimental mode
$res = $bucket->query($query);
var_dump($res->rows[0]);
//=> object(stdClass)#4 (1) {
//     ["greeting"]=>
//     string(12) "Hello, beer!"
//   }
