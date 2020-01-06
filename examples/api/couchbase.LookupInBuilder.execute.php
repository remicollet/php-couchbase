<?php
$cluster = new \Couchbase\Cluster("couchbase://localhost");
$bucket = $cluster->bucket('default');

$bucket->upsert('foo', ['path1' => 'value1']);

$result = $bucket->lookupIn('foo')
        ->get('path1')
        ->exists('path2')
        ->execute();

var_dump(count($result->value));      //=> int(2)

var_dump($result->value[0]['code']);  //=> int(0)    COUCHBASE_SUCCESS
var_dump($result->value[0]['value']); //=> string(6) "value1"

var_dump($result->value[1]['code']);  //=> int(63)  COUCHBASE_ERR_SUBDOC_PATH_NOT_FOUND
var_dump($result->value[1]['value']); //=> NULL