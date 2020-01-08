<?php
$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster("couchbase://localhost", $options);

$cluster->bucket('protected'); // successfully opens connection
$cluster->manager()->createBucket('hello'); // automatically use admin credentials
