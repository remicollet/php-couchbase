<?php

require_once('CouchbaseTestCase.php');

class AliasesTest extends CouchbaseTestCase {
    function testAliasesForPublicAPI() {
        $this->assertClassAlias("Couchbase\Cluster", "CouchbaseCluster");
        $this->assertClassAlias("Couchbase\Bucket", "CouchbaseBucket");
        $this->assertClassAlias("Couchbase\MutationToken", "CouchbaseMutationToken");
        $this->assertClassAlias("Couchbase\MutationState", "CouchbaseMutationState");
        $this->assertClassAlias("Couchbase\BucketManager", "CouchbaseBucketManager");
        $this->assertClassAlias("Couchbase\ClusterManager", "CouchbaseClusterManager");
        $this->assertClassAlias("Couchbase\LookupInBuilder", "CouchbaseLookupInBuilder");
        $this->assertClassAlias("Couchbase\MutateInBuilder", "CouchbaseMutateInBuilder");
        $this->assertClassAlias("Couchbase\N1qlQuery", "CouchbaseN1qlQuery");
        $this->assertClassAlias("Couchbase\SearchQuery", "CouchbaseSearchQuery");
        $this->assertClassAlias("Couchbase\SearchQueryPart", "CouchbaseAbstractSearchQuery");
        $this->assertClassAlias("Couchbase\QueryStringSearchQuery", "CouchbaseStringSearchQuery");
        $this->assertClassAlias("Couchbase\MatchSearchQuery", "CouchbaseMatchSearchQuery");
        $this->assertClassAlias("Couchbase\MatchPhraseSearchQuery", "CouchbaseMatchPhraseSearchQuery");
        $this->assertClassAlias("Couchbase\PrefixSearchQuery", "CouchbasePrefixSearchQuery");
        $this->assertClassAlias("Couchbase\RegexpSearchQuery", "CouchbaseRegexpSearchQuery");
        $this->assertClassAlias("Couchbase\NumericRangeSearchQuery", "CouchbaseNumericRangeSearchQuery");
        $this->assertClassAlias("Couchbase\DisjunctionSearchQuery", "CouchbaseDisjunctionSearchQuery");
        $this->assertClassAlias("Couchbase\DateRangeSearchQuery", "CouchbaseDateRangeSearchQuery");
        $this->assertClassAlias("Couchbase\ConjunctionSearchQuery", "CouchbaseConjunctionSearchQuery");
        $this->assertClassAlias("Couchbase\BooleanSearchQuery", "CouchbaseBooleanSearchQuery");
        $this->assertClassAlias("Couchbase\WildcardSearchQuery", "CouchbaseWildcardSearchQuery");
        $this->assertClassAlias("Couchbase\DocIdSearchQuery", "CouchbaseDocIdSearchQuery");
        $this->assertClassAlias("Couchbase\BooleanFieldSearchQuery", "CouchbaseBooleanFieldSearchQuery");
        $this->assertClassAlias("Couchbase\TermSearchQuery", "CouchbaseTermSearchQuery");
        $this->assertClassAlias("Couchbase\PhraseSearchQuery", "CouchbasePhraseSearchQuery");
        $this->assertClassAlias("Couchbase\MatchAllSearchQuery", "CouchbaseMatchAllSearchQuery");
        $this->assertClassAlias("Couchbase\MatchNoneSearchQuery", "CouchbaseMatchNoneSearchQuery");
        $this->assertClassAlias("Couchbase\DateRangeSearchFacet", "CouchbaseDateRangeSearchFacet");
        $this->assertClassAlias("Couchbase\NumericRangeSearchFacet", "CouchbaseNumericRangeSearchFacet");
        $this->assertClassAlias("Couchbase\TermSearchFacet", "CouchbaseTermSearchFacet");
        $this->assertClassAlias("Couchbase\SearchFacet", "CouchbaseSearchFacet");
        $this->assertClassAlias("Couchbase\ViewQuery", "CouchbaseViewQuery");
        $this->assertClassAlias("Couchbase\DocumentFragment", "CouchbaseDocumentFragment");
        $this->assertClassAlias("Couchbase\Document", "CouchbaseMetaDoc");
        $this->assertClassAlias("Couchbase\Exception", "CouchbaseException");
        $this->assertClassAlias("Couchbase\ClassicAuthenticator", "CouchbaseAuthenticator");
    }

    private function assertClassAlias($className, $aliasName) {
        $alias = new ReflectionClass($aliasName);
        $this->assertEquals("couchbase", $alias->getExtensionName());
        $this->assertEquals($className, $alias->getName());
    }
}
