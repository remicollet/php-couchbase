<?php
$cluster = new \Couchbase\Cluster('couchbase://localhost?fetch_mutation_tokens=true');
$bucket = $cluster->bucket('default');

$doc = $bucket->upsert("foo", ['answer' => 42]);
$mutationState = \Couchbase\MutationState::from($doc);

$query = \Couchbase\N1qlQuery::fromString("SELECT * FROM `default` WHERE answer=42");
$result = $query->consistentWith($mutationState);
// the $result should contain information from "foo"