<?php

function displayIndexes($indexes, $bucket_name) {
    printf("There are %d query indexes in the bucket \"%s\":\n", count($indexes), $bucket_name);
    foreach ($indexes as $index) {
        printf("  * [%s] %s", $index->state(), $index->name());
        if ($index->isPrimary()) {
            printf(" (primary)");
        }
        if (count($index->indexKey()) > 0) {
            printf(" on [%s]", join(", ", $index->indexKey()));
        }
        if ($index->condition()) {
            printf(" where \"%s\"", $index->condition());
        }
        printf("\n");
    }
}

$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster('couchbase://localhost', $options);

$bucket_name = 'beer-sample';
displayIndexes($cluster->queryIndexes()->getAllIndexes($bucket_name), $bucket_name);

$index_name = 'demo_index';

$options = new \Couchbase\DropQueryIndexOptions();
$options->ignoreIfNotExists(true);
$start = microtime(true);
$cluster->queryIndexes()->dropIndex($bucket_name, $index_name, $options);
printf("Index \"%s\" has been dropped in %f seconds\n", $index_name, microtime(true) - $start);

$options = new \Couchbase\CreateQueryIndexOptions();
$options->ignoreIfExists(true);
$options->condition('abv > 2');
$start = microtime(true);
$cluster->queryIndexes()->createIndex($bucket_name, $index_name, ['type', 'name'], $options);
printf("Index \"%s\" has been created in %f seconds\n", $index_name, microtime(true) - $start);

$options = new \Couchbase\DropQueryPrimaryIndexOptions();
$options->ignoreIfNotExists(true);
$start = microtime(true);
$cluster->queryIndexes()->dropPrimaryIndex($bucket_name, $options);
printf("Primary index has been dropped in %f seconds\n", microtime(true) - $start);

$options = new \Couchbase\CreateQueryPrimaryIndexOptions();
$options->deferred(true);
$start = microtime(true);
$cluster->queryIndexes()->createPrimaryIndex($bucket_name, $options);
printf("Primary index on \"%s\" has been created in %f seconds\n", $bucket_name, microtime(true) - $start);

displayIndexes($cluster->queryIndexes()->getAllIndexes($bucket_name), $bucket_name);

$start = microtime(true);
$cluster->queryIndexes()->buildDeferredIndexes($bucket_name);
printf("Build of indexes for \"%s\" has been triggered in %f seconds\n", $bucket_name, microtime(true) - $start);
displayIndexes($cluster->queryIndexes()->getAllIndexes($bucket_name), $bucket_name);

$options = new \Couchbase\WatchQueryIndexesOptions();
$options->watchPrimary(true);
$timeout = 10000000; // in microseconds (= 10 seconds)
$start = microtime(true);
$cluster->queryIndexes()->watchIndexes($bucket_name, [], $timeout, $options);
printf("Watching for of primary index build completion on \"%s\" has been finished in %f seconds\n", $bucket_name, microtime(true) - $start);
displayIndexes($cluster->queryIndexes()->getAllIndexes($bucket_name), $bucket_name);
