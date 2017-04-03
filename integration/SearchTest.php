<?php

/**
 * The tests are relying on FTS index for beer-sample bucket exists with the following mapping for type 'beer':
 *
 *    name | text | index | store | include in _all field | include term vectors
 *    description | text | index | store | include in _all field | include term vectors
 *    updated | datetime | index | store | include in _all field
 *    abv | number | index | store | include in _all field
 *    style | text | index | store | include in _all field | include term vectors
 *    category | text | index | store | include in _all field | include term vectors
 */
class SearchTest extends PHPUnit_Framework_TestCase {
    public function __construct() {
        $this->testDsn = getenv('CPDSN');
        if ($this->testDsn === FALSE) {
            $this->testDsn = 'couchbase://localhost/';
        }
    }

    protected function setUp() {
        $this->cluster = new \Couchbase\Cluster($this->testDsn);
        $this->bucket = $this->cluster->openBucket('beer-sample');
    }

    function testSearchWithLimit() {
        $queryPart = \Couchbase\SearchQuery::matchPhrase("hop beer");
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->limit(3);

        $result = $this->bucket->query($query);

        $this->assertNotNull($result);
        $this->assertNotEmpty($result->hits);
        $this->assertCount(2, $result->hits);
        $this->assertEquals(2, $result->metrics['total_hits']);

        foreach ($result->hits as $hit) {
            $this->assertNotNull($hit->id);
            $this->assertStringStartsWith("beer-search", $hit->index);
            $this->assertGreaterThan(0, $hit->score);
        }
    }

    function testSearchWithNoHits() {
        $queryPart = \Couchbase\SearchQuery::matchPhrase("doesnotexistintheindex");
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->limit(3);

        $result = $this->bucket->query($query);

        $this->assertNotNull($result);
        $this->assertEmpty($result->hits);
        $this->assertEquals(0, $result->metrics['total_hits']);
    }

    function testSearchWithConsistency() {
        $cluster = new \Couchbase\Cluster($this->testDsn . '?fetch_mutation_tokens=true');
        $bucket = $cluster->openBucket('beer-sample');

        $id = uniqid('testSearchWithConsistency');
        $queryPart = \Couchbase\SearchQuery::matchPhrase($id);
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->limit(3);

        $result = $bucket->query($query);

        $this->assertNotNull($result);
        $this->assertEmpty($result->hits);
        $this->assertEquals(0, $result->metrics['total_hits']);

        $result = $bucket->upsert($id, ["type" => "beer", "name" => $id]);
        $mutationState = \Couchbase\MutationState::from($result);

        $query->consistentWith($mutationState);
        $result = $bucket->query($query);

        $this->assertNotNull($result);
        $this->assertNotEmpty($result->hits);
        $this->assertEquals(1, $result->metrics['total_hits']);
        $this->assertEquals($id, $result->hits[0]->id);
    }

    function testSearchWithFields() {
        $queryPart = \Couchbase\SearchQuery::matchPhrase("hop beer");
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->limit(3)->fields("name");

        $result = $this->bucket->query($query);

        $this->assertNotNull($result);
        $this->assertNotEmpty($result->hits);
        $this->assertCount(2, $result->hits);
        $this->assertEquals(2, $result->metrics['total_hits']);

        foreach ($result->hits as $hit) {
            $this->assertNotNull($hit->id);
            $this->assertStringStartsWith("beer-search", $hit->index);
            $this->assertGreaterThan(0, $hit->score);
            $this->assertObjectHasAttribute('fields', $hit);
            $this->assertObjectHasAttribute('name', $hit->fields);
            $this->assertNotNull($hit->fields->name);
        }
    }

    function testSearchWithRanges() {
        $queryPart = \Couchbase\SearchQuery::numericRange()->field("abv")->min(2.0)->max(3.2);
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->fields("abv");
        $result = $this->bucket->query($query);

        $this->assertNotNull($result);
        $this->assertNotEmpty($result->hits);
        $this->assertEquals(count($result->hits), $result->metrics['total_hits']);

        foreach ($result->hits as $hit) {
            $this->assertNotNull($hit->id);
            $this->assertStringStartsWith("beer-search", $hit->index);
            $this->assertGreaterThan(0, $hit->score);
            $this->assertObjectHasAttribute('fields', $hit);
            $this->assertObjectHasAttribute('abv', $hit->fields);
            $this->assertGreaterThanOrEqual(2.0, $hit->fields->abv);
            $this->assertLessThan(3.2, $hit->fields->abv);
        }

        $startDate = new DateTime('2010-11-01 10:00:00');
        $startStr = $startDate->format(DATE_RFC3339);
        $endInt = mktime(20, 0, 0, 12, 1, 2010);
        $endDate = DateTime::createFromFormat("U", $endInt);
        $queryPart = \Couchbase\SearchQuery::conjuncts(
            \Couchbase\SearchQuery::term("beer")->field("type"),
            \Couchbase\SearchQuery::dateRange()->field("updated")->start($startStr)->end($endInt)
        );
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->fields("updated", "type");
        $result = $this->bucket->query($query);

        $this->assertNotNull($result);
        $this->assertNotEmpty($result->hits);
        $this->assertEquals(count($result->hits), $result->metrics['total_hits']);

        foreach ($result->hits as $hit) {
            $this->assertNotNull($hit->id);
            $this->assertStringStartsWith("beer-search", $hit->index);
            $this->assertGreaterThan(0, $hit->score);
            $this->assertObjectHasAttribute('fields', $hit);
            $this->assertObjectHasAttribute('updated', $hit->fields);
            $hitDate = new DateTime($hit->fields->updated);
            $diff = $startDate->diff($hitDate);
            $this->assertEquals(0, $diff->invert,
                              "The hit->update date ({$hitDate->format(DATE_RFC3339)}) should go after start date ({$startDate->format(DATE_RFC3339)})");
            $diff = $endDate->diff($hitDate);
            $this->assertEquals(1, $diff->invert,
                              "The hit->update date ({$hitDate->format(DATE_RFC3339)}) should go before or equals to end date ({$startDate->format(DATE_RFC3339)})");
        }
    }

    function testCompoundSearchQueries() {
        $nameQuery = \Couchbase\SearchQuery::match("green")->field("name")->boost(3.4);
        $descriptionQuery = \Couchbase\SearchQuery::match("hop")->field("description")->fuzziness(1);

        $disjunctionQuery = \Couchbase\SearchQuery::disjuncts($nameQuery, $descriptionQuery);
        $query = new \Couchbase\SearchQuery("beer-search", $disjunctionQuery);
        $query->fields("type", "name", "description");
        $result = $this->bucket->query($query);
        $this->assertGreaterThan(1000, $result->metrics['total_hits']);
        $this->assertNotEmpty($result->hits);
        $this->assertRegexp('/green/i', $result->hits[0]->fields->name);
        $this->assertNotRegexp('/hop/i', $result->hits[0]->fields->name);
        $this->assertRegexp('/hop/i', $result->hits[0]->fields->description);
        $this->assertNotRegexp('/green/i', $result->hits[0]->fields->description);

        $disjunctionQuery->min(2);
        $query = new \Couchbase\SearchQuery("beer-search", $disjunctionQuery);
        $query->fields("type", "name", "description");
        $result = $this->bucket->query($query);
        $this->assertNotEmpty($result->hits);
        $this->assertLessThan(10, $result->metrics['total_hits']);
        $disjunction2Result = $result;

        $conjunctionQuery = \Couchbase\SearchQuery::conjuncts($nameQuery, $descriptionQuery);
        $query = new \Couchbase\SearchQuery("beer-search", $conjunctionQuery);
        $query->fields("type", "name", "description");
        $result = $this->bucket->query($query);
        $this->assertNotEmpty($result->hits);
        $this->assertEquals(count($disjunction2Result->hits), count($result->hits));
        $this->assertEquals($disjunction2Result->hits[0]->fields->name,
                            $result->hits[0]->fields->name);
        $this->assertEquals($disjunction2Result->hits[0]->fields->description,
                            $result->hits[0]->fields->description);
    }

    function testSearchWithFragments() {
        $queryPart = \Couchbase\SearchQuery::match("hop beer");
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query->limit(3)->highlight(\Couchbase\SearchQuery::HIGHLIGHT_HTML, "name");

        $result = $this->bucket->query($query);
        $this->assertNotEmpty($result->hits);

        foreach ($result->hits as $hit) {
            $this->assertNotNull($hit->id);
            $this->assertObjectHasAttribute('fragments', $hit);
            $this->assertObjectHasAttribute('name', $hit->fragments);
            foreach ($hit->fragments->name as $fragment) {
                $this->assertRegexp('/<mark>/', $fragment);
            }
        }
    }

    function testSearchWithFacets() {
        $queryPart = \Couchbase\SearchQuery::term("beer")->field("type");
        $query = new \Couchbase\SearchQuery("beer-search", $queryPart);
        $query
            ->addFacet("foo", \Couchbase\SearchQuery::termFacet("name", 3))
            ->addFacet("bar", \Couchbase\SearchQuery::dateRangeFacet("updated", 1)
                       //->addRange("old", NULL, mktime(0, 0, 0, 1, 1, 2014)))
                       ->addRange("old", NULL, "2014-01-01T00:00:00"))
            ->addFacet("baz", \Couchbase\SearchQuery::numericRangeFacet("abv", 2)
                       ->addRange("strong", 4.9, NULL)
                       ->addRange("light", NULL, 4.89));

        $result = $this->bucket->query($query);
        $this->assertNotEmpty($result->hits);
        $this->assertObjectHasAttribute('facets', $result);
        $this->assertNotEmpty($result->facets);

        $this->assertNotNull($result->facets['foo']);
        $this->assertEquals('name', $result->facets['foo']['field']);
        $this->assertEquals('ale', $result->facets['foo']['terms'][0]['term']);
        $this->assertGreaterThan(1000, $result->facets['foo']['terms'][0]['count']);

        $this->assertNotNull($result->facets['bar']);
        $this->assertEquals('updated', $result->facets['bar']['field']);
        $this->assertEquals('old', $result->facets['bar']['date_ranges'][0]['name']);
        $this->assertGreaterThan(5000, $result->facets['bar']['date_ranges'][0]['count']);

        $this->assertNotNull($result->facets['baz']);
        $this->assertEquals('abv', $result->facets['baz']['field']);
        $this->assertEquals('light', $result->facets['baz']['numeric_ranges'][0]['name']);
        $this->assertGreaterThan(100, $result->facets['baz']['numeric_ranges'][0]['count']);
    }
}
