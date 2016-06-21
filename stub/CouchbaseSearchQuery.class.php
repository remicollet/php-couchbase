<?php
/**
 * File for the CouchbaseN1qlQuery class.
 */

/**
 * Represents a Full Text Search query to be executed against a Couchbase bucket.
 *
 * @package Couchbase
 */
class CouchbaseSearchQuery {
    /**
     * @var string
     */
    private $indexName;

    /**
     * @var CouchbaseAbstractSearchQuery
     */
    private $queryPart;

    /**
     * @var integer
     */
    private $limit;

    /**
     * @var integer
     */
    private $skip;

    /**
     * @var boolean
     */
    private $explain;

    /**
     * @var string
     */
    private $highlightStyle;

    /**
     * @var array
     */
    private $highlightFields;

    /**
     * @var integer
     */
    private $serverSideTimeout;

    /**
     * @var array
     */
    private $fields;

    /**
     * @var array
     */
    private $facets = array();

    public function __construct($indexName, $queryPart) {
        $this->indexName = $indexName;
        $this->queryPart = $queryPart;
    }

    /**
     * Add a limit to the query on the number of hits it can return.
     *
     * @param integer $limit the maximum number of hits to return.
     * @return $this
     */
    public function limit($limit) {
        $this->limit = $limit;
        return $this;
    }

    /**
     * Set the number of hits to skip (eg. for pagination).
     *
     * @param integer $skip the number of results to skip.
     * @return $this
     */
    public function skip($skip) {
        $this->skip = $skip;
        return $this;
    }

    /**
     * Activates or deactivates the explanation of each result hit in the response, according to the parameter.
     *
     * @param boolean $explain should the response include an explanation of each hit (true) or not (false)?
     * @return $this
     */
    public function explain($explain) {
        $this->explain = $explain;
        return $this;
    }

    /**
     * Configures the highlighting of matches in the response.
     *
     * This drives the inclusion of the fragments in each CouchbaseSearchQueryRow hit.
     *
     * Note that to be highlighted, the fields must be stored in the FTS index.
     *
     * @param string $style the style to apply ('ascii', 'html' or null).
     * @param array $fields the optional fields on which to highlight. If none, all fields where there is a match are highlighted.
     * @return $this
     */
    public function highlight($style, $fields = array()) {
        if ($style != 'ascii' && $style != 'html') {
            $this->highlightStyle = null;
            $this->highlightFields = null;
        } else {
            $this->highlightStyle = $style;
            if (count($fields) > 0) {
                $this->highlightFields = $fields;
            }
        }
        return $this;
    }

    /**
     * Configures the list of fields for which the whole value should be included in the response. If empty, no field
     * values are included.
     *
     * This drives the inclusion of the CouchbaseSearchQueryRow#fields() in each CouchbaseSearchQueryRow hit.
     *
     * Note that to be included, the fields must be stored in the FTS index.
     *
     * @param array fields
     * @return $this
     */
    public function fields($fields = null) {
        if ($fields) {
            $this->fields = $fields;
        }
        return $this;
    }

    /**
     * Sets the server side timeout.
     *
     * @param $timeout the server side timeout in milliseconds to apply.
     * @return $this
     */
    public function serverSideTimeout($timeout) {
        $this->serverSideTimeout = $timeout;
        return $this;
    }

    /**
     * Adds one search facet to the query.
     *
     * This is an additive operation (the given facets are added to any facet previously requested),
     * but if an existing facet has the same name it will be replaced.
     *
     * This drives the inclusion of the  facets in the CouchbaseSearchQueryResult.
     *
     * Note that to be faceted, a field's value must be stored in the FTS index.
     *
     * @param string $facetName the name of the facet to add (or replace if one already exists with same name).
     * @param CouchbaseSearchFacet $facet the facet to add.
     *
     * @return $this
     */
    public function addFacet($facetName, $facet) {
        if (facet != null && facetName != null) {
            $this->facets[$facetName] = $facet;
        }
        return $this;
    }

    /**
     * Clears all previously added CouchbaseSearchFacet
     *
     * @return $this
     */
    public function clearFacets() {
        $this->facets = array();
        return $this;
    }

    public function export() {
        $result = array('indexName' => $this->indexName);
        $this->injectParams($result);

        $result['query'] = array();
        $this->queryPart->injectParamsAndBoost($result['query']);
        return $result;
    }

    public function injectParams(&$input) {
        if ($this->limit !== null && $this->limit >= 0) {
            $input['size'] = $this->limit;
        }
        if ($this->skip !== null && $this->skip >= 0) {
            $input['from'] = $this->skip;
        }
        if ($this->explain !== null) {
            $input['explain'] = $this->explain;
        }
        if ($this->highlightStyle !== null) {
            $input['highlight'] = array('style' => $this->highlightStyle);
            if (count($this->highlightFields) > 0) {
                $input['highlight']['fields'] = $this->highlightFields;
            }
        }
        if (count($this->fields) > 0) {
            $input['fields'] = $this->fields;
        }
        if (count($this->facets) > 0) {
            $facets = array();
            foreach ($this->facets as $key => $value) {
                $facets[$key] = array();
                $value->injectParams($facets[$key]);
            }
            $input['facets'] = $facets;
        }

        $control = array();
        //check need for timeout
        if($this->serverSideTimeout !== null) {
            $control['timeout'] = $this->serverSideTimeout;
        }
        //if any control was set, inject it
        if (count($control) > 0) {
            $input['ctl'] = $control;
        }
    }

    /* ===============================
     * Factory methods for FTS queries
     * =============================== */

    /**
     * Prepare a CouchbaseStringSearchQuery body.
     *
     * @param string $query
     */
    public static function string($query) {
        return new CouchbaseStringSearchQuery($query);
    }

    /**
     * Prepare a CouchbaseMatchSearchQuery body.
     *
     * @param string $match
     */
    public static function match($match) {
        return new CouchbaseMatchSearchQuery($match);
    }

    /**
     * Prepare a CouchbaseMatchPhraseSearchQuery body.
     *
     * @param string $matchPhrase
     */
    public static function matchPhrase($matchPhrase) {
        return new CouchbaseMatchPhraseSearchQuery($matchPhrase);
    }

    /**
     * Prepare a CouchbasePrefixSearchQuery body.
     *
     * @param string $prefix
     */
    public static function prefix($prefix) {
        return new CouchbasePrefixSearchQuery($prefix);
    }

    /**
     * Prepare a CouchbaseRegexpSearchQuery body.
     *
     * @param string $regexp
     */
    public static function regexp($regexp) {
        return new CouchbaseRegexpSearchQuery($regexp);
    }

    /** Prepare a CouchbaseNumericRangeSearchQuery body. */
    public static function numericRange() {
        return new CouchbaseNumericRangeSearchQuery();
    }

    /** Prepare a CouchbaseDateRangeSearchQuery body. */
    public static function dateRange() {
        return new CouchbaseDateRangeSearchQuery();
    }

    /**
     * Prepare a CouchbaseDisjunctionSearchQuery body.
     *
     * @param array $queries array of CouchbaseAbstractSearchQuery
     */
    public static function disjuncts($queries = array()) {
        return new CouchbaseDisjunctionSearchQuery($queries);
    }

    /**
     * Prepare a CouchbaseConjunctionSearchQuery body.
     *
     * @param array $queries array of CouchbaseAbstractSearchQuery
     */
    public static function conjuncts($queries = array()) {
        return new CouchbaseConjunctionSearchQuery($queries);
    }

    /** Prepare a CouchbaseBooleanSearchQuery body. */
    public static function booleans() {
        return new CouchbaseBooleanSearchQuery();
    }

    /**
     * Prepare a CouchbaseWildcardSearchQuery body.
     *
     * @param string $wildcard
     */
    public static function wildcard($wildcard) {
        return new CouchbaseWildcardSearchQuery($wildcard);
    }

    /**
     * Prepare a CouchbaseDocIdSearchQuery body.
     *
     * @param array $docIds array of strings
     */
    public static function docId($docIds = array()) {
        return new CouchbaseDocIdSearchQuery($docIds);
    }

    /**
     * Prepare a CouchbaseBooleanFieldSearchQuery body.
     *
     * @param boolean $value
     */
    public static function booleanField($value) {
        return new CouchbaseBooleanFieldSearchQuery($value);
    }

    /**
     * Prepare a CouchbaseTermSearchQuery body.
     *
     * @param string $term
     */
    public static function term($term) {
        return new CouchbaseTermSearchQuery($term);
    }

    /**
     * Prepare a CouchbasePhraseSearchQuery body.
     *
     * @param array $terms array of strings
     */
    public static function phrase($terms) {
        return new CouchbasePhraseSearchQuery($terms);
    }

    /** Prepare a CouchbaseMatchAllSearchQuery body. */
    public static function matchAll() {
        return new CouchbaseMatchAllSearchQuery();
    }

    /** Prepare a CouchbaseMatchNoneSearchQuery body. */
    public static function matchNone() {
        return new CouchbaseMatchNoneSearchQuery();
    }
}

/**
 * A base class for all FTS query classes. Exposes the common FTS query parameters.
 * In order to instantiate various flavors of queries, look at concrete classes or
 * static factory methods in CouchbaseSearchQuery.
 */
abstract class CouchbaseAbstractSearchQuery {
    /**
     * @var double
     */
    private $boost;

    protected function __construct() {
        $this->boost = null;
    }

    public function boost($boost) {
        $this->boost = $boost;
        return $this;
    }

    public function injectParamsAndBoost(&$input) {
        if ($this->boost !== null) {
            $input['boost'] = $this->boost;
        }
        $this->injectParams($input);
    }
}

/**
 * A FTS query that performs a search according to the "string query" syntax.
 */
class CouchbaseStringSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $query;

    public function __construct($query) {
        $this->query = $query;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function injectParams(&$input) {
        $input['query'] = $this->query;
    }
}

/**
 * A FTS query that matches a given term, applying further processing to it
 * like analyzers, stemming and even #fuzziness(int).
 */
class CouchbaseMatchSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $match;

    /**
     * @var string
     */
    private $field;

    /**
     * @var string
     */
    private $analyzer;

    /**
     * @var integer
     */
    private $prefixLength;

    /**
     * @var integer
     */
    private $fuzziness;

    public function __construct($match) {
        $this->match = $match;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function analyzer($analyzer) {
        $this->analyzer = $analyzer;
        return $this;
    }

    public function prefixLength($prefixLength) {
        $this->prefixLength = $prefixLength;
        return $this;
    }

    public function fuzziness($fuzziness) {
        $this->fuzziness = $fuzziness;
        return $this;
    }

    public function injectParams(&$input) {
        $input['match'] = $this->match;
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
        if ($this->analyzer !== null) {
            $input['analyzer'] = $this->analyzer;
        }
        if ($this->prefixLength !== null) {
            $input['prefix_length'] = $this->prefixLength;
        }
        if ($this->fuzziness !== null) {
            $input['fuzziness'] = $this->fuzziness;
        }
    }
}

/**
 * A FTS query that matches terms (without further analysis). Usually for debugging purposes.
 * Prefer using CouchbaseMatchSearchQuery.
 */
class CouchbaseTermSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $term;

    /**
     * @var string
     */
    private $field;

    /**
     * @var integer
     */
    private $prefixLength;

    /**
     * @var integer
     */
    private $fuzziness;

    public function __construct($term) {
        $this->term = $term;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function prefixLength($prefixLength) {
        $this->prefixLength = $prefixLength;
        return $this;
    }

    public function fuzziness($fuzziness) {
        $this->fuzziness = $fuzziness;
        return $this;
    }

    public function injectParams(&$input) {
        $input['term'] = $this->term;
        if ($field != null) {
            $input['field'] = $this->field;
        }
        if ($fuzziness > 0) {
            $input['fuzziness'] = $this->fuzziness;
            if (prefixLength > 0) {
                $input['prefix_length'] = $this->prefixLength;
            }
        }
    }
}

/**
 * A FTS query that matches several terms (a "phrase") as is. The order of the terms mater and no
 * further processing is applied to them, so they must appear in the index exactly as provided.
 * Usually for debugging purposes. Prefer CouchbaseMatchPhraseSearchQuery.
 */
class CouchbasePhraseSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $terms;

    /**
     * @var string
     */
    private $field;

    public function __construct($terms = array()) {
        $this->terms = $terms;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function injectParams(&$input) {
        if (count($this->terms) < 1) {
            throw new InvalidArgumentException("Phrase query must at least have one term");
        }
        $input['terms'] = $this->terms;

        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * A FTS query that matches several given terms (a "phrase"), applying further processing
 * like analyzers to them.
 */
class CouchbaseMatchPhraseSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $matchPhrase;

    /**
     * @var string
     */
    private $field;

    /**
     * @var string
     */
    private $analyzer;

    public function __construct($matchPhrase) {
        $this->matchPhrase = $matchPhrase;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function analyzer($analyzer) {
        $this->analyzer = $analyzer;
        return $this;
    }

    public function injectParams(&$input) {
        $input['match_phrase'] = $this->matchPhrase;
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
        if ($this->analyzer !== null) {
            $input['analyzer'] = $this->analyzer;
        }
    }
}

/**
 * A FTS query that allows for simple matching on a given prefix.
 */
class CouchbasePrefixSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $prefix;

    /**
     * @var string
     */
    private $field;

    public function __construct($prefix) {
        $this->prefix = $prefix;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function injectParams(&$input) {
        $input['prefix'] = $this->prefix;
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * A FTS query that allows for simple matching of regular expressions.
 */
class CouchbaseRegexpSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $regexp;

    /**
     * @var string
     */
    private $field;

    public function __construct($regexp) {
        $this->regexp = $regexp;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function injectParams(&$input) {
        $input['regexp'] = $this->regexp;
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * An FTS query that allows for simple matching using wildcard characters (* and ?).
 */
class CouchbaseWildcardSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $wildcard;

    /**
     * @var string
     */
    private $field;

    public function __construct($wildcard) {
        $this->wildcard = $wildcard;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function injectParams(&$input) {
        $input['wildcard'] = $this->wildcard;
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * A FTS query that queries fields explicitly indexed as boolean.
 */
class CouchbaseBooleanFieldSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var boolean
     */
    private $value;

    /**
     * @var string
     */
    private $field;

    public function __construct($value) {
        $this->value = $value;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function injectParams(&$input) {
        $input['bool'] = $this->value;
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * A FTS query that matches on Couchbase document IDs. Useful to restrict the search space to a list of keys
 * (by using this in a compound query).
 */
class CouchbaseDocIdSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var array
     */
    private $docIds;

    public function __construct($docIds = array()) {
        $this->docIds = $docIds;
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function docIds($docIds) {
        foreach ($docIds as $docId) {
            array_push($this->docIds, $field);
        }
        return $this;
    }

    public function injectParams(&$input) {
        if (count($this->docIds) < 1) {
            throw new InvalidArgumentException("DocID query needs at least one document ID");
        }

        $input['ids'] = $this->docIds;
    }
}

/**
 * A FTS query that matches all indexed documents (usually for debugging purposes).
 */
class CouchbaseMatchAllSearchQuery extends CouchbaseAbstractSearchQuery {
    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function injectParams(&$input) {
        $input['match_all'] = null;
    }
}

/**
 * A FTS query that matches 0 document (usually for debugging purposes).
 */
class CouchbaseMatchNoneSearchQuery extends CouchbaseAbstractSearchQuery {
    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function injectParams(&$input) {
        $input['match_none'] = null;
    }
}

/**
 * A FTS query that matches documents on a range of values. At least one bound is required, and the
 * inclusiveness of each bound can be configured.
 */
class CouchbaseNumericRangeSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var float
     */
    private $min;

    /**
     * @var float
     */
    private $max;

    /**
     * @var boolean
     */
    private $inclusiveMin;

    /**
     * @var boolean
     */
    private $inclusiveMax;

    /**
     * @var string
     */
    private $field;

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function min($min, $inclusive = true) {
        $this->min = $min;
        $this->inclusiveMin = $inclusive;
        return $this;
    }

    public function max($max, $inclusive = false) {
        $this->max = $max;
        $this->inclusiveMax = $inclusive;
        return $this;
    }

    public function injectParams(&$input) {
        if ($this->min === null && $this->max === null) {
            throw new InvalidArgumentException("CouchbaseNumericRangeQuery needs at least one of min or max");
        }
        if ($this->min !== null) {
            $input['min'] = $this->min;
            if ($this->inclusiveMin !== null) {
                $input['inclusive_min'] = $this->inclusiveMin;
            }
        }
        if ($this->max !== null) {
            $input['max'] = $this->max;
            if ($this->inclusiveMax !== null) {
                $input['inclusive_max'] = $this->inclusiveMax;
            }
        }
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * A FTS query that matches documents on a range of dates. At least one bound is required, and the parser
 * to use for the date (in string form) can be customized (see #dateTimeParser(string)).
 */
class CouchbaseDateRangeSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var string
     */
    private $start;

    /**
     * @var string
     */
    private $end;

    /**
     * @var boolean
     */
    private $inclusiveStart;

    /**
     * @var boolean
     */
    private $inclusiveEnd;

    /**
     * @var string
     */
    private $dateTimeParser;

    /**
     * @var string
     */
    private $field;

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function field($field) {
        $this->field = $field;
        return $this;
    }

    public function start($start, $inclusive = true) {
        $this->start = $start;
        $this->inclusiveStart = $inclusive;
        return $this;
    }

    public function end($end, $inclusive = false) {
        $this->end = $end;
        $this->inclusiveEnd = $inclusive;
        return $this;
    }

    /**
     * The name of the date/time parser to use to interpret #start(string) and #end(string).
     */
    public function dateTimeParser($parser) {
        $this->dateTimeParser = $parser;
        return $this;
    }

    public function injectParams(&$input) {
        if ($this->start === null && $this->end === null) {
            throw new InvalidArgumentException("CouchbaseDateRangeQuery needs at least one of start or end");
        }
        if ($this->start !== null) {
            $input['start'] = $this->start;
            if ($this->inclusiveStart !== null) {
                $input['inclusive_start'] = $this->inclusiveStart;
            }
        }
        if ($this->end !== null) {
            $input['end'] = $this->end;
            if ($this->inclusiveEnd !== null) {
                $input['inclusive_end'] = $this->inclusiveEnd;
            }
        }
        if ($this->dateTimeParser != null) {
            $input['datetime_parser'] = $this->dateTimeParser;
        }
        if ($this->field !== null) {
            $input['field'] = $this->field;
        }
    }
}

/**
 * Base class for FTS queries that are composite, compounding several other CouchbaseAbstractSearchQuery.
 */
abstract class CouchbaseAbstractCompoundSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var array
     */
    protected $childQueries;

    protected function __construct($childQueries) {
        $this->childQueries = $childQueries;
    }

    protected function addAll($queries) {
        foreach ($queries as $query) {
            array_push($this->childQueries, $query);
        }
    }
}

/**
 * A compound FTS query that performs a logical OR between all its sub-queries (disjunction).
 * It requires that a minimum of the queries match. The minimum is configurable via #min(int) (default 1).
 */
class CouchbaseDisjunctionSearchQuery extends CouchbaseAbstractCompoundSearchQuery {
    /**
     * @var int
     */
    private $min;

    public function __construct($queries = array()) {
        parent::__construct($queries);
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function min($min) {
        $this->min = $min;
        return $this;
    }

    public function either($queries) {
        $this->addAll($queries);
        return $this;
    }

    public function injectParams(&$input) {
        if (count($this->childQueries) < 1) {
            throw new InvalidArgumentException("Compound query has no child query");
        }
        if (count($this->childQueries) < $this->min) {
            throw new InvalidArgumentException("Disjunction query as fewer children than the configured minimum " + $this->min);
        }

        if ($this->min > 0) {
            $input['min'] = $this->min;
        }

        $disjuncts = array();
        foreach ($this->childQueries as $childQuery) {
            $child = array();
            $childQuery->injectParamsAndBoost($child);
            array_push($disjuncts, $child);
        }
        $input['disjuncts'] = $disjuncts;
    }
}

/**
 * A compound FTS query that performs a logical AND between all its sub-queries (conjunction).
 */
class CouchbaseConjunctionSearchQuery extends CouchbaseAbstractCompoundSearchQuery {
    public function __construct($queries = array()) {
       parent::__construct($queries);
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function every($queries) {
        $this->addAll($queries);
        return $this;
    }

    public function injectParams(&$input) {
        if (count($this->childQueries) < 1) {
            throw new InvalidArgumentException("Compound query has no child query");
        }

        $conjuncts = array();
        foreach ($this->childQueries as $childQuery) {
            $child = array();
            $childQuery->injectParamsAndBoost($child);
            array_push($conjuncts, $child);
        }
        $input['conjuncts'] = $conjuncts;
    }
}

/**
 * A compound FTS query that allows various combinations of sub-queries.
 */
class CouchbaseBooleanSearchQuery extends CouchbaseAbstractSearchQuery {
    /**
     * @var CouchbaseConjunctionSearchQuery
     */
    private $must;

    /**
     * @var CouchbaseDisjunctionSearchQuery
     */
    private $mustNot;

    /**
     * @var CouchbaseDisjunctionSearchQuery
     */
    private $should;

    public function __construct() {
        $this->must = new CouchbaseConjunctionSearchQuery();
        $this->mustNot = new CouchbaseDisjunctionSearchQuery();
        $this->should = new CouchbaseDisjunctionSearchQuery();
    }

    public function boost($boost) {
        parent::boost($boost);
        return $this;
    }

    public function shouldMin($minForShould) {
        $this->should->min($minForShould);
        return $this;
    }

    public function must($mustQueries) {
        $this->must->every($mustQueries);
        return $this;
    }

    public function mustNot($mustNotQueries) {
        $this->mustNot->either($mustNotQueries);
        return $this;
    }

    public function should($shouldQueries) {
        $this->should->either($shouldQueries);
        return $this;
    }

    public function injectParams(&$input) {
        $mustIsEmpty = must === null || count($this->must->childQueries()) < 1;
        $mustNotIsEmpty = mustNot === null || count($this->mustNot->childQueries()) < 1;
        $shouldIsEmpty = should === null || count($this->should->childQueries()) < 1;

        if ($mustIsEmpty && $mustNotIsEmpty && $shouldIsEmpty) {
            throw new InvalidArgumentException("Boolean query needs at least one of must, mustNot and should");
        }

        if (!$mustIsEmpty) {
            $child = array();
            $this->must->injectParamsAndBoost($child);
            $input['must'] = $child;
        }

        if (!$mustNotIsEmpty) {
            $child = array();
            $this->mustNot->injectParamsAndBoost($child);
            $input['must_not'] = $child;
        }

        if (!$shouldIsEmpty) {
            $child = array();
            $this->should->injectParamsAndBoost($child);
            $input['should'] = $child;
        }
    }
}

abstract class CouchbaseSearchFacet {
    /**
     * @var string
     */
    private $field;

    /**
     * @var integer
     */
    private $limit;

    protected function __construct($field, $limit) {
        $this->field = $field;
        $this->limit = $limit;
    }

    public function injectParams(&$input) {
        $input['size'] = $this->limit;
        $input['field'] = $this->field;
    }

    public static function term($field, $limit) {
        return new CouchbaseTermSearchFacet($field, $limit);
    }

    public static function numeric($field, $limit) {
        return new CouchbaseNumericSearchFacet($field, $limit);
    }

    public static function dataRange($field, $limit) {
        return new CouchbaseDateRangeSearchFacet($field, $limit);
    }
}

class CouchbaseTermSearchFacet extends CouchbaseSearchFacet {
    public function __construct($field, $limit) {
        parent::__construct($field, $limit);
    }
}

class CouchbaseDateRangeSearchFacet extends CouchbaseSearchFacet {
    /**
     * @var array
     */
    private $dateRanges;

    public function __construct($field, $limit) {
        parent::__construct($field, $limit);
        $this->dateRanges = array();
    }

    public function addRange($rangeName, $start, $end) {
        $range = array();
        if ($start) {
            $range['start'];
        }
        if ($end) {
            $range['end'];
        }
        $this->dateRanges[$rangeName] = $range;
        return $this;
    }

    public function injectParams(&$input) {
        parent::injectParams($input);
        foreach ($this->dateRanges as $name => $value) {
            $input['date_ranges'][$name] = $value;
        }
    }
}

class CouchbaseNumericRangeSearchFacet extends CouchbaseSearchFacet {
    /**
     * @var array
     */
    private $numericRanges;

    public function __construct($field, $limit) {
        parent::__construct($field, $limit);
        $this->numericRanges = array();
    }

    public function addRange($rangeName, $min, $max) {
        $range = array();
        if ($min) {
            $range['min'];
        }
        if ($max) {
            $range['max'];
        }
        $this->numericRanges[$rangeName] = $range;
        return $this;
    }

    public function injectParams(&$input) {
        parent::injectParams($input);
        foreach ($this->numericRanges as $name => $value) {
            $input['numeric_ranges'][$name] = $value;
        }
    }
}
