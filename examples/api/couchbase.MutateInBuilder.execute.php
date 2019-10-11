<?php
$cluster = new \Couchbase\Cluster("couchbase://localhost");
$cluster = new \Couchbase\Cluster("couchbase://192.168.1.194");
$bucket = $cluster->bucket('default');

$bucket->upsert('bar', [
    'field1' => 'value1',
    'field2' => 'value2',
    'array' =>  [1, 2, 3],
]);

$result = $bucket->mutateIn('bar')
        ->replace('field1', ['foo' => 'bar'])
        ->remove('array')
        ->replace('missing', "hello world")
        ->execute();

var_dump($result->error->getCode()); //=> int(73)   COUCHBASE_SUBDOC_MULTI_FAILURE
var_dump(count($result->value));     //=> int(1)
var_dump($result->value[2]['code']); //=> int(63)   COUCHBASE_SUBDOC_PATH_ENOENT

// the document is not modified
var_dump($bucket->get('bar')->value);
//=> object(stdClass)#6 (3) {
//     ["field1"]=>
//     string(6) "value1"
//     ["field2"]=>
//     string(6) "value2"
//     ["array"]=>
//     array(3) {
//       [0]=>
//       int(1)
//       [1]=>
//       int(2)
//       [2]=>
//       int(3)
//     }
//   }

$result = $bucket->mutateIn('bar')
        ->replace('field1', ['foo' => 'bar'])
        ->remove('array')
        ->execute();
var_dump($bucket->get('bar')->value);
//=> object(stdClass)#6 (2) {
//     ["field1"]=>
//     object(stdClass)#5 (1) {
//       ["foo"]=>
//       string(3) "bar"
//     }
//     ["field2"]=>
//     string(6) "value2"
//   }