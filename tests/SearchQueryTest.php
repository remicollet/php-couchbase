<?php
require_once('CouchbaseTestCase.php');

class SearchQueryTest extends CouchbaseTestCase {
    function testNullInNumericRangeFacet() {
        $facet = \Couchbase\SearchQuery::numericRangeFacet("abv", 2)->addRange("light", NULL, 4.89);
        $this->assertNotNull(json_encode($facet));
    }
}
