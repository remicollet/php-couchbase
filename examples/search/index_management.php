<?php

$cluster = new \Couchbase\Cluster('couchbase://localhost');
$cluster->authenticateAs('Administrator', 'password');

$bucket = $cluster->openBucket('travel-sample');

$manager = $bucket->manager()->searchIndexManager();

var_dump($manager->listIndexDefinitions());

var_dump($manager->getIndexDefinition('travel-sample-index-hotel-description'));

var_dump($manager->getIndexedDocumentsCount('travel-sample-index-hotel-description'));

$indexDef = '
{
  "type": "fulltext-index",
  "name": "testindex",
  "sourceType": "couchbase",
  "sourceName": "travel-sample",
  "planParams": {
    "maxPartitionsPerPIndex": 171
  },
  "params": {
    "doc_config": {
      "mode": "type_field",
      "type_field": "type"
    },
    "mapping": {
      "default_analyzer": "standard",
      "default_datetime_parser": "dateTimeOptional",
      "default_field": "_all",
      "default_mapping": {
        "dynamic": true,
        "enabled": true
      },
      "default_type": "_default",
      "index_dynamic": true,
      "store_dynamic": false
    },
    "store": {
      "kvStoreName": "mossStore"
    }
  },
  "sourceParams": {}
}
';

var_dump($manager->createIndex('testindex', $indexDef));

var_dump($manager->deleteIndex('testindex'));
