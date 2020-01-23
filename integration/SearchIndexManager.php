<?php

$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster('couchbase://localhost', $options);

$search_indexes = $cluster->searchIndexes()->getAllIndexes();
printf("There are %d search indexes on the cluster\n", count($search_indexes));
if (count($search_indexes) > 0) {
  $search_index_name = current($search_indexes)->name();
  $search_index = $cluster->searchIndexes()->getIndex($search_index_name);
  $number_docs_indexed = $cluster->searchIndexes()->getIndexedDocumentsCount($search_index_name);
  printf("Index \"%s\" contains %d indexed documents\n", $search_index->name(), $number_docs_indexed);
}

$search_index_name = 'myIndex';
if (array_key_exists($search_index_name, $search_indexes)) {
  $start = microtime(true);
  $cluster->searchIndexes()->dropIndex($search_index_name);
  printf("Index \"%s\" has been dropped in %f seconds\n", $search_index_name, microtime(true) - $start);
}

$index = new \Couchbase\SearchIndex();
$index->setType('fulltext-index');
$index->setName($search_index_name);
$index->setSourceType('couchbase');
$index->setSourceName('travel-sample');
$start = microtime(true);
$cluster->searchIndexes()->upsertIndex($index);
printf("Index \"%s\" has been created in %f seconds\n", $search_index_name, microtime(true) - $start);
$indexManager = $cluster->searchIndexes();
for ($i = 0; $i < 3; $i++) {
  sleep(1);
  printf("    index \"%s\" contains %d indexed documents\n", $search_index_name,
    $indexManager->getIndexedDocumentsCount($search_index_name));
}

$document = [
  'title' => 'Hello world',
  'content' => 'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.'
];
$analysis = $cluster->searchIndexes()->analyzeDocument($search_index_name, $document);
printf("Analysis of document \n%s\n using definition of the index \"%s\"\n%s\n",
  json_encode($document), $search_index_name, json_encode($analysis));
