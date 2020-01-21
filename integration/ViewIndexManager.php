<?php

$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster('couchbase://localhost', $options);

$bucket = $cluster->bucket('beer-sample');
$design_documents = $bucket->viewIndexes()->getAllDesignDocuments();
printf("There are %d design documents in the bucket\n", count($design_documents));
foreach ($design_documents as $ddoc) {
  printf("  * %s\n", $ddoc->name());
  foreach ($ddoc->views() as $view) {
    printf("    * %s\n", $view->name());
  }
}

if (count($design_documents) > 0) {
  $design_document_name = $design_documents[0]->name();
  $design_document = $bucket->viewIndexes()->getDesignDocument($design_document_name);
  printf("Design document \"%s\" contains %d views\n", $design_document->name(), count($design_document->views()));

  $start = microtime(true);
  $bucket->viewindexes()->dropDesignDocument($design_document_name);
  printf("Design document \"%s\" removed, in %f seconds\n", $design_document_name, microtime(true)-$start);
}

$view = new \Couchbase\View();
$view->setName("example");
$view->setMap("function (doc, meta) { emit(meta.id, null); }");
$view->setReduce("_count");
$design_document = new \Couchbase\DesignDocument();
$design_document->setName('_design/test');
$design_document->setViews([$view->name() => $view]);

$start = microtime(true);
$bucket->viewindexes()->upsertDesignDocument($design_document);
printf("Design document \"%s\" created, in %f seconds\n", $design_document->name(), microtime(true)-$start);
