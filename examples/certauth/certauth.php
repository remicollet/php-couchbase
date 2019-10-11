<?php
// This example uses X.509 certificates to authenticate on Couchbase Server
// Read more about this feature at
//
//   https://developer.couchbase.com/documentation/server/5.0/security/security-x509certsintro.html
//
// Also helper script, which generates certificates for local cluster, could be found here:
//
//   https://gist.github.com/avsej/e1a05532b605ddd3a282734a6049a858

// Certificate chain, which includes client public certificate (going first), and then concatenated with intermediate
// and root certificates if they are not part of the system trusted certificates.
$cert = "/tmp/x509-cert/SSLCA/clientdir/chain.pem";
// Private key for the client. Note that the username should be embedded into certificate on the generation stage into
// corresponding field. In our example, we use CN and user "testuser", which has access to bucket "default".
$key = "/tmp/x509-cert/SSLCA/clientdir/client.key";

// NOTE: that it have to use "couchbases://" schema ("https://" will also work)
$cluster = new \Couchbase\Cluster("couchbases://127.0.0.1?certpath=$cert&keypath=$key");
$bucket = $cluster->bucket("default");
$bucket->upsert("foo", ["bar" => 42]);
