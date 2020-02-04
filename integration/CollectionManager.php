<?php

$options = new \Couchbase\ClusterOptions();
$options->credentials('Administrator', 'password');
$cluster = new \Couchbase\Cluster('couchbase://localhost', $options);
$bucket = $cluster->bucket('travel-sample');

$scope_name = "myapp";

$scopes = $bucket->collections()->getAllScopes();
printf("There are %d scopes on the cluster:\n", count($scopes));
foreach ($scopes as $scope) {
    printf("  * \"%s\" (%d collections)\n", $scope->name(), count($scope->collections()));
    foreach ($scope->collections() as $collection) {
        printf("    * \"%s\"\n", $collection->name());
    }
    if ($scope->name() == $scope_name) {
        $has_scope = true;
    }
}

if ($has_scope) {
    $start = microtime(true);
    $bucket->collections()->dropScope($scope_name);
    printf("Scope \"%s\" has been removed in %fus\n", $scope_name, microtime(true)-$start);
}
$start = microtime(true);
$bucket->collections()->createScope($scope_name);
printf("Scope \"%s\" has been created in %fus\n", $scope_name, microtime(true)-$start);

$scope = $bucket->collections()->getScope($scope_name);
printf("Scope \"%s\" has %d collections\n", $scope_name, count($scope->collections()));

$collection = new \Couchbase\CollectionSpec();
$collection->setScopeName($scope_name);
$collection->setName('users');

$start = microtime(true);
$bucket->collections()->createCollection($collection);
printf("Collection \"%s\" on scope \"%s\" has been created in %fus\n",
    $collection->name(), $collection->scopeName(), microtime(true)-$start);

$scope = $bucket->collections()->getScope($scope_name);
printf("Scope \"%s\" has %d collections\n", $scope_name, count($scope->collections()));
foreach ($scope->collections() as $collection) {
    printf("    * \"%s\"\n", $collection->name());
}

$start = microtime(true);
$bucket->collections()->dropCollection($collection);
printf("Collection \"%s\" on scope \"%s\" has been dropped in %fus\n",
    $collection->name(), $collection->scopeName(), microtime(true)-$start);

$scope = $bucket->collections()->getScope($scope_name);
printf("Scope \"%s\" has %d collections\n", $scope_name, count($scope->collections()));
foreach ($scope->collections() as $collection) {
    printf("    * \"%s\"\n", $collection->name());
}
