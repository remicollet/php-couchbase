<?php

$cluster = new \Couchbase\Cluster('couchbase://192.168.1.194');
$cluster->authenticateAs('Administrator', 'password');

$bucket = $cluster->bucket('travel-sample');

// Add key-value pairs to hotel_10138, representing traveller-Ids and associated discount percentages
$bucket->mutateIn('hotel_10138')
    ->upsert('discounts.jsmith123', '20', ['xattr' => true, 'createPath' => true])
    ->upsert('discounts.pjones356', '30', ['xattr' => true, 'createPath' => true])
    // The following lines, "insert" and "remove", simply demonstrate insertion and
    // removal of the same path and value
    ->insert('discounts.jbrown789', '25', ['xattr' => true, 'createPath' => true])
    ->remove('discounts.jbrown789', ['xattr' => true])
    ->execute();

// Add key - value pairs to hotel_10142, again representing traveller - Ids and associated discount percentages
$bucket->mutateIn('hotel_10142')
    ->upsert('discounts.jsmith123', '15', ['xattr' => true, 'createPath' => true])
    ->upsert('discounts.pjones356', '10', ['xattr' => true, 'createPath' => true])
    ->execute();

// Create a user and assign roles. This user will search for their available discounts.
$userSettings = new \Couchbase\UserSettings();
$userSettings
    ->password('jsmith123pwd')
    ->role('data_reader', 'travel-sample')
    ->role('query_select', 'travel-sample');
$cluster->manager()->upsertUser('cbtestuser', $userSettings);

// reconnect using new user
$cluster = new \Couchbase\Cluster('couchbase://192.168.1.194');
$cluster->authenticateAs('jsmith123', 'jsmith123pwd');

$bucket = $cluster->bucket('travel-sample');

// Perform a N1QL Query to return document IDs from the bucket. These IDs will be
// used to reference each document in turn, and check for extended attributes
// corresponding to discounts.
$query = \Couchbase\N1qlQuery::fromString('SELECT id, meta(`travel-sample`).id AS docID FROM `travel-sample`');
$result = $bucket->query($query);
$results = '';

foreach ($result->rows as $row) {
    // get row document ID
    $docID = $row->docID;

    // Determine whether a hotel-discount has been applied to this user.
    $res = $bucket->lookupIn($docID)
         ->exists('discounts.jsmith123', ['xattr' => true])
         ->execute();
    if ($res->value[0]['code'] == COUCHBASE_SUCCESS) {
        // If so, get the discount-percentage.
        $res = $bucket->lookupIn($docID)
             ->get('discounts.jsmith123', ['xattr' => true])
             ->execute();
        $discount = $res->value[0]['value'];

        // If the percentage - value is greater than 15, include the document in the
        // results to be returned.
        $results = sprintf("%s\n%s - %s", $results, $discount, $docID);
    }
}
printf("Results returned are: %s\n", $results);