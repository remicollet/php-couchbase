<?php

$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster('couchbase://localhost', $options);

$bucket_name = 'newbucket';

$need_cleanup = false;
$all_buckets = $cluster->buckets()->getAllBuckets();
printf("There are %d buckets on the cluster\n", count($all_buckets));
foreach ($all_buckets as $bucket) {
  printf(" * %s (%d MB)\n", $bucket->name(), $bucket->ramQuotaMb());
  if ($bucket->name() == $bucket_name) {
    $need_cleanup = true;
  }
}

if ($need_cleanup) {
  $start = microtime(true);
  $cluster->buckets()->removeBucket($bucket_name);
  printf("Bucket \"%s\" removed in %f seconds\n", $bucket_name, microtime(true) - $start);
}

$settings = new \Couchbase\BucketSettings();
$settings->setName($bucket_name);
$settings->setRamQuotaMb(100);
$settings->enableFlush(true);
$start = microtime(true);
$cluster->buckets()->createBucket($settings);
printf("New bucket \"%s\" created in %f seconds\n", $bucket_name, microtime(true) - $start);

sleep(1);

$settings = $cluster->buckets()->getBucket($bucket_name);
printf("Bucket \"%s\" settings:\n", $bucket_name);
printf(" * RAM quota: %d\n", $settings->ramQuotaMb());
printf(" * number of replicas: %d\n", $settings->numReplicas());
printf(" * flush enabled: %s\n", $settings->flushEnabled() ? "yes" : "no");
printf(" * max TTL: %s\n", $settings->maxTtl());
printf(" * compression mode: %s\n", $settings->compressionMode());
printf(" * replicas indexed: %s\n", $settings->replicaIndexes() ? "yes" : "no");
printf(" * ejection method: %s\n", $settings->ejectionMethod());

$start = microtime(true);
$cluster->buckets()->flush($bucket_name);
printf("Bucket \"%s\" flushed in %f seconds\n", $bucket_name, microtime(true) - $start);
