<?php
/**
 * The following example demonstrates how you might use query scan consistency
 * when your fetching code sensitive to data mutations. For example, you are
 * updating someonce score and it is necessary to apply change immediately and
 * do not allow stale information in the queries.
 *
 * In this example we will implement helper for autoincrementing document IDs.
 *
 * Read about how it works here:
 * http://developer.couchbase.com/documentation/server/4.0/architecture/querying-data-with-n1ql.html#story-h2-2
 */

/*
 * Create a new Cluster object to represent the connection to our
 * cluster and specify any needed options such as SSL.
 */
$cluster = new \Couchbase\Cluster('couchbase://localhost');
/*
 * We open the default bucket to store our docuemtns in.
 */
$bucket = $cluster->openBucket('default');


function get_next_doc_id($bucket, $consistency)
{
    /*
     * Order all documents in the bucket by their IDs lexicographically
     * from higher to lower and select first result.
     */
    $query = \Couchbase\N1qlQuery::fromString('select meta(`default`).id from `default` order by meta(`default`).id desc limit 1');
    $query->consistency($consistency);
    $res = $bucket->query($query);
    $id = 1; // for empty bucket lets consider next ID be 1.
    if (count($res->rows) > 0) {
        // If there are documents in the bucket just increment it.
        $id = (int)$res->rows[0]->id + 1;
    }
    return sprintf("%08d", $id);
}

/*
 * First round with NOT_BOUNDED consistency, which is default.
 * Obviously this round will complete faster.
 */
printf("using NOT_BOUNDED consistency (generates 'STALE ID...' messages)\n");
for ($prev_id = "", $i = 0; $i < 10; $i++, $prev_id = $next_id) {
    printf(".");
    $next_id = get_next_doc_id($bucket, \Couchbase\N1qlQuery::NOT_BOUNDED);
    if ($prev_id == $next_id) {
        /*
         * In this case the program reports STALE ID for almost every iteration.
         */
        printf("STALE ID: %d\n", $next_id);
    }
    $bucket->upsert($next_id, array('i' => $i));
}

/*
 * Second round with REQUEST_PLUS consistency, and it will not
 * generate STALE ID messages.
 */
printf("using REQUEST_PLUS consistency (works as expected)\n");
for ($prev_id = "", $i = 0; $i < 10; $i++, $prev_id = $next_id) {
    printf(".");
    $next_id = get_next_doc_id($bucket, \Couchbase\N1qlQuery::REQUEST_PLUS);
    if ($prev_id == $next_id) {
        printf("STALE ID: %d\n", $next_id);
    }
    $bucket->upsert($next_id, array('i' => $i));
}
