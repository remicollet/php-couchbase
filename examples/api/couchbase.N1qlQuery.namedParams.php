<?php
use \Couchbase\N1qlQuery;
$query = N1qlQuery::fromString('SELECT airportname FROM `travel-sample` WHERE city=$city AND type=$airport');
$query->namedParams(['city' => "Los Angeles", 'type' => "airport"]);
