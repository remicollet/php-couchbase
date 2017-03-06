<?php
$authenticator = new \Couchbase\ClassicAuthenticator();
$authenticator->cluster('Administrator', 'password');
$authenticator->bucket('protected', 'secret');

$cluster = new \Couchbase\Cluster("couchbase://localhost");
$cluster->authenticate($authenticator);

$cluster->openBucket('protected'); // successfully opens connection
$cluster->manager()->createBucket('hello'); // automatically use admin credentials
