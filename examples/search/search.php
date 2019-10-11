<?php

/**
   The example assumes the existence of "travel-sample" bucket and three
   specific Full Text Indexes, defined for it. These are:
   - "travel-sample-index-unstored": Uses only the default settings.
   - "travel-sample-index-stored": Uses default settings, with one exception:
     dynamic fields are stored, for the whole index.
   - "travel-sample-index-hotel-description": Indexes only the description
     fields of hotel documents, and disables the default type mapping. The index
     has a custom analyzer named myUnicodeAnalyzer defined on it: the analyzer's
     main characteristic is that it uses the unicode tokenizer.
 */

use \Couchbase\Cluster;
use \Couchbase\Bucket;
use \Couchbase\SearchOptions;

function printResult($label, $resultObject) {
    echo("\n");
    echo("= = = = = = = = = = = = = = = = = = = = = = =\n");
    echo("= = = = = = = = = = = = = = = = = = = = = = =\n");
    echo("\n");
    echo($label);
    echo(' (total hits: ' . $resultObject->metrics['total_hits'] . ')');
    echo("\n");

    foreach ($resultObject->hits as $row) {
        echo("id=" . $row->id . ", score=" . $row->score);
        if (property_exists($row, 'fields')) {
            echo(", fields=" . json_encode($row->fields));
        }
        if (property_exists($row, 'locations')) {
            echo(", locations=" . json_encode($row->locations));
        }
        if (property_exists($row, 'fragments')) {
            echo(", fragments=" . json_encode($row->fragments));
        }
        echo("\n");
    }
}

// Simple Text Query on a single word, targeting an index with dynamic fields
// unstored.
function simpleTextQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-unstored";
    $query = new SearchOptions($indexName,
                             SearchOptions::match("swanky"));
    $query->limit(10);
    $result = $bucket->query($query);
    printResult("Simple Text Query", $result);
}

// Simple Text Query on Stored Field, specifying the field to be searched;
// targeting an index with dynamic fields stored, to ensure that field-content
// is included in the return object.
function simpleTextQueryOnStoredField(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::match("MDG")->field("destinationairport"));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Simple Text Query on Stored Field", $result);
}

// Simple Text Query on Non-Default Index, specifying an index that consists
// only of content derived from a specific field from a specific document-type.
function simpleTextQueryOnNonDefaultIndex(Bucket $bucket) {
    $indexName = "travel-sample-index-hotel-description";
    $query = new SearchOptions($indexName,
                             SearchOptions::match("swanky"));
    $query->limit(10);
    $result = $bucket->query($query);
    printResult("Simple Text Query on Non-Default Index", $result);
}

// Match Query with Facet, showing how query-results can be displayed either by
// row or by hits; and demonstrating use of a facet, which provides
// aggregation-data.
function textQueryOnStoredFieldWithFacet(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::match("La Rue Saint Denis!!")->field("reviews.content"));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $query->addFacet('Countries Referenced', SearchOptions::termFacet('country', 5));
    $result = $bucket->query($query);
    printResult("Match Query with Facet, Result by Row", $result);

    echo("Match Query with Facet, Result by facet:\n");
    echo(json_encode($result->facets) . "\n");
}

// DocId Query, showing results of a query on two document IDs.
function docIdQueryMethod(Bucket $bucket) {
    $indexName = "travel-sample-index-unstored";
    $query = new SearchOptions($indexName,
                             SearchOptions::docId("hotel_26223", "hotel_28960"));
    $result = $bucket->query($query);
    printResult("DocId Query", $result);
}

// Unanalyzed Term Query with Fuzziness Level, demonstrating how to query on a
// term with no analysis. Zero fuzziness is specified, to ensure that matches
// are exact. With a fuzziness factor of 2, allowing partial matches to be made.
function unAnalyzedTermQuery(Bucket $bucket, int $fuzzinessLevel) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::term("sushi")->field("reviews.content")->fuzziness($fuzzinessLevel));
    $query->limit(50)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Unanalyzed Term Query with Fuzziness Level of " . $fuzzinessLevel . ":", $result);
}

// Match Phrase Query, using Analysis, for searching on a phrase.
function matchPhraseQueryOnStoredField(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::matchPhrase("Eiffel Tower")->field("description"));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Match Phrase Query, using Analysis", $result);
}

// Phrase Query, without Analysis, for searching on a phrase without analysis supported.
function unAnalyzedPhraseQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::phrase("dorm", "rooms")->field("description"));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Phrase Query, without Analysis", $result);
}

// Conjunction Query, whereby two separate queries are defined and then run as
// part of the search, with only the matches returned by both included in the
// result-object.
function conjunctionQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $firstQuery = SearchOptions::match("La Rue Saint Denis!!")->field("reviews.content");
    $secondQuery = SearchOptions::match("boutique")->field("description");
    $query = new SearchOptions($indexName,
                             SearchOptions::conjuncts($firstQuery, $secondQuery));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Conjunction Query", $result);
}

// Query String Query, showing how a query string is specified as search-input.
function queryStringQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-unstored";
    $query = new SearchOptions($indexName,
                             SearchOptions::queryString("description: Imperial"));
    $query->limit(10);
    $result = $bucket->query($query);
    printResult("Query String Query", $result);
}

// Wild Card Query, whereby a wildcard is used in the string submitted for the
// search.
function wildCardQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::wildcard("bouti*ue")->field("description"));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Wild Card Query", $result);
}

// Numeric Range Query, whereby minimum and maximum numbers are specified, and
// matches within the range returned.
function numericRangeQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-unstored";
    $query = new SearchOptions($indexName,
                             SearchOptions::numericRange()->min(10100)->max(10200)->field("id"));
    $query->limit(10);
    $result = $bucket->query($query);
    printResult("Numeric Range Query", $result);
}

// Regexp Query, whereby a regular expression is submitted, to generate the
// conditions for successful matches.
function regexpQuery(Bucket $bucket) {
    $indexName = "travel-sample-index-stored";
    $query = new SearchOptions($indexName,
                             SearchOptions::regexp("[a-z]")->field("description"));
    $query->limit(10)->highlight(SearchOptions::HIGHLIGHT_HTML);
    $result = $bucket->query($query);
    printResult("Regexp Query", $result);
}

$cluster = new Cluster('couchbase://localhost');
$cluster->authenticateAs('Administrator', 'password');
$bucket = $cluster->bucket('travel-sample');

simpleTextQuery($bucket);
simpleTextQueryOnStoredField($bucket);
simpleTextQueryOnNonDefaultIndex($bucket);
textQueryOnStoredFieldWithFacet($bucket);
docIdQueryMethod($bucket);
unAnalyzedTermQuery($bucket, 0);
unAnalyzedTermQuery($bucket, 2);
matchPhraseQueryOnStoredField($bucket);
unAnalyzedPhraseQuery($bucket);
conjunctionQuery($bucket);
queryStringQuery($bucket);
wildCardQuery($bucket);
numericRangeQuery($bucket);
regexpQuery($bucket);
