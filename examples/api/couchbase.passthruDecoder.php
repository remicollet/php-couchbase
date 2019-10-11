<?php
$cluster = new \Couchbase\Cluster("couchbase://localhost");
$bucket = $cluster->bucket("default");
$bucket->setTranscoder("\\Couchbase\\defaultEncoder", "\\Couchbase\\passthruDecoder");
$bucket->upsert("foo", ["bar" => "baz"]);
$value = $bucket->get("foo")->value;
var_dump($value);
// => string(13) "{"bar":"baz"}"