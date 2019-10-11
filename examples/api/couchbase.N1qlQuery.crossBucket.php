<?php
$authenticator = new \Couchbase\ClassicAuthenticator();
$authenticator->bucket('people', 'secret');
$authenticator->bucket('orders', '123456');

$cluster = new \Couchbase\Cluster("couchbase://localhost");
$cluster->authenticate($authenticator);

$bucket = $cluster->bucket('orders');

$query = \Couchbase\N1qlQuery::fromString(
    "SELECT * FROM `orders` JOIN `people` ON KEYS `orders`.person_id ORDER BY `orders`.name");
$query->consistency(\Couchbase\N1qlQuery::REQUEST_PLUS);
$query->crossBucket(true);

$res = $bucket->query($query);
// $res inludes rows from orders and people buckets