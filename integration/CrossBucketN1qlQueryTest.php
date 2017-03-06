<?php

/**
 * The tests are relying on two buckets created:
 *
 *   - `people` with password 'secret'
 *   - `orders` with password '123456'
 *
 * Also primary indexes have to be created on them.
 */
class SearchTest extends PHPUnit_Framework_TestCase {
    public function __construct() {
        $testDsn = getenv('CPDSN');
        if ($testDsn === FALSE) {
            $testDsn = 'couchbase://localhost/';
        }
        $this->cluster = new \Couchbase\Cluster($testDsn);
    }

    protected function populatePeople() {
        $bucket = $this->cluster->openBucket('people', 'secret');
        $bucket->upsert("john", ['name' => 'John Doe', 'city' => 'New York']);
        $bucket->upsert("jane", ['name' => 'Jane Doe', 'city' => 'Miami']);
    }

    protected function populateOrders() {
        $bucket = $this->cluster->openBucket('orders', '123456');
        $bucket->upsert("soap-12", ['name' => 'Green Soap', 'person_id' => 'john']);
        $bucket->upsert("soap-42", ['name' => 'Red Soap', 'person_id' => 'jane']);
        $bucket->upsert("rope-3",  ['name' => 'Rope 5m', 'person_id' => 'john']);
    }


    function testCrossBucketQuery() {
        $this->populatePeople();
        $this->populateOrders();

        $authenticator = new \Couchbase\ClassicAuthenticator();
        $authenticator->bucket('people', 'secret');
        $authenticator->bucket('orders', '123456');
        $this->cluster->authenticate($authenticator);

        $bucket = $this->cluster->openBucket('orders');

        $query = \Couchbase\N1qlQuery::fromString(
            "SELECT * FROM `orders` JOIN `people` ON KEYS `orders`.person_id ORDER BY `orders`.name");
        $query->consistency(\Couchbase\N1qlQuery::REQUEST_PLUS);
        $query->crossBucket(true);

        $res = $bucket->query($query);

        $this->assertEquals('Green Soap', $res->rows[0]->orders->name);
        $this->assertEquals('John Doe', $res->rows[0]->people->name);

        $this->assertEquals('Red Soap', $res->rows[1]->orders->name);
        $this->assertEquals('Jane Doe', $res->rows[1]->people->name);

        $this->assertEquals('Rope 5m', $res->rows[2]->orders->name);
        $this->assertEquals('John Doe', $res->rows[2]->people->name);
    }
}