<?php
$cluster = new \Couchbase\Cluster('couchbase://localhost?detailed_errcodes=1');
$cluster->authenticateAs('Administrator', 'password');
$bucket = $cluster->bucket('default');

$query = \Couchbase\AnalyticsQuery::fromString('
    SELECT "Hello, " || $name || "!" AS greeting,
           "¡Hola, " || ? || "!" AS saludo
');
$query->namedParams(['name' => 'Beer']);
$query->positionalParams(['Cerveza']);
$res = $bucket->query($query);
var_dump($res->rows[0]);
//=> object(stdClass)#5 (2) {
//     ["greeting"]=>
//     string(12) "Hello, Beer!"
//     ["saludo"]=>
//     string(16) "¡Hola, Cerveza!"
//   }

