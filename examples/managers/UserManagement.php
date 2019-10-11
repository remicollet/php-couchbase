<?php

// Access the cluster that is running on the local host, authenticating with
// the username and password of the Full Administrator. This
// provides all privileges.

$cluster = new \Couchbase\Cluster('couchbase://localhost');

// Make sure that 'default' bucket exists.
echo("Authenticating as administrator.\n");
$cluster->authenticateAs("Administrator", "password");


// Create a user and assign roles.
echo("Upserting new user.\n");
$userSettings = new \Couchbase\UserSettings();
$userSettings
    ->password('cbtestuserpwd')
    // Roles required for the reading of data from the bucket.
    ->role('data_reader', 'travel-sample')
    ->role('query_select', 'travel-sample')
    // Roles required for the writing data into the bucket.
    ->role('data_writer', 'travel-sample')
    ->role('query_insert', 'travel-sample')
    ->role('query_delete', 'travel-sample')
    // Role required for the creation of indexes on the bucket.
    ->role('query_manage_index', 'travel-sample');
$cluster->manager()->upsertUser('cbtestuser', $userSettings);


// List current users.
echo("Listing current users.\n");
$listOfUsers = $cluster->manager()->listUsers();
for ($j = 0; $j < count($listOfUsers); $j++) {
    $currentUser = $listOfUsers[$j];

    echo("\n\nUSER #" . $j .":\n\n");

    if (array_key_exists('name', $currentUser)) {
        echo("User's name is: " . $currentUser['name'] . "\n");
    }
    echo("User's id is: " . $currentUser['id'] . "\n");
    echo("User's domain is: " . $currentUser['domain'] . "\n");

    $currentRoles = $currentUser['roles'];

    for ($i = 0; $i < count($currentRoles); $i++) {
        echo("User has the role: " . $currentRoles[$i]['role'] . ", applicable to bucket " .
             $currentRoles[$i]['bucket_name'] . "\n");
    }
}


// Access the cluster that is running on the local host, specifying
// the username and password already assigned by the administrator
echo("Authenticating as user.\n");
$cluster->authenticateAs("cbtestuser", "cbtestuserpwd");

// Open a known, existing bucket (created by the administrator).
echo("Opening travel-sample bucket as user.\n");
$travelSample = $cluster->bucket("travel-sample");

// Create a N1QL Primary Index (but ignore if one already exists).
$travelSample->manager()->createN1qlPrimaryIndex('', true, false);

// Read out a known, existing document within the bucket (created
// by the administrator).
echo("Reading out airline_10 document\n");
$returnedAirline10doc = $travelSample->get("airline_10");
echo("Found: \n");
print_r($returnedAirline10doc);

// Create a new document.
echo("Creating new document as user.\n");
$airline11Object = [
    "callsign" => "MILE-AIR",
    "iata" => "Q5",
    "icao" => "MLA",
    "id" => 11,
    "name" => "40-Mile Air",
    "type" => "airline"
];

// Upsert the document to the bucket.
echo("Upserting new document as user.\n");
$travelSample->upsert("airline_11", $airline11Object);

echo("Reading out airline11Document as user.\n");
$returnedAirline11Doc = $travelSample->get("airline_11");
echo("Found: \n");
print_r($returnedAirline11Doc);

// Perform a N1QL Query.
echo("Performing query as user.\n");

$result = $travelSample->query(
    \Couchbase\N1qlQuery::fromString("SELECT * FROM `travel-sample` LIMIT 5")
);

echo("Query-results are:\n");
// Print each row returned by the query.
foreach ($result->rows as $row) {
    print_r($row);
}

// Access the cluster that is running on the local host, authenticating with
// the username and password of the Full Administrator. This
// provides all privileges.
echo("Re-authenticating as administrator.\n");
$cluster->authenticateAs("Administrator", "password");

// Remove known user.
echo("Removing user as administrator.\n");
$userToBeRemoved = "cbtestuser";
try {
    $cluster->manager()->removeUser($userToBeRemoved);
    echo("Deleted user " . $userToBeRemoved . ".\n");
} catch (\Couchbase\Exception $ex) {
    echo("Could not delete user " . $userToBeRemoved . ".\n");
}
