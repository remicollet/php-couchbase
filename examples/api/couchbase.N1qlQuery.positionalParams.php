<?php
use \Couchbase\N1qlQuery;
$query = N1qlQuery::fromString('SELECT airportname FROM `travel-sample` WHERE city=$1 AND type=$2');
$query->positionalParams(["Los Angeles", "airport"]);