# Couchbase PHP Client

This library allows you to connect to a Couchbase cluster from PHP.
It is a native PHP extension and uses the very fast libcouchbase library to
handle communicating to the cluster over the Couchbase binary protocol.
It supports 5.x and 7.x releases of PHP interpreter.

## Useful Links

Source - http://github.com/couchbaselabs/php-couchbase

Bug Tracker - http://www.couchbase.com/issues/browse/PCBC

Couchbase PHP Community - https://forums.couchbase.com/c/php-sdk

Documentation - http://developer.couchbase.com/documentation/server/current/sdk/php/start-using-sdk.html


## Installing

Couchbase PHP client generally available through PECL: http://pecl.php.net/package/couchbase

```bash
pecl install couchbase
```

Additionally Windows builds available from [Release Notes and Archives][http://developer.couchbase.com/server/other-products/release-notes-archives/php-sdk] page.

On MacOS platform, the library could be installed via Homebrew:

```bash
brew tap homebrew/homebrew-php
brew install php70-couchbase # or other version instead of 70 (PHP 7.0)
```

If you are going to prepare patches, or just need to install the most recent
version from git, make sure you have PHP development tools and headers
installed, and run the following commands:

```bash
git clone git://github.com/couchbase/php-couchbase.git
cd php-couchbase
phpize
./configure --with-couchbase
make && make install
```

## Introduction

Connecting to a Couchbase bucket is as simple as creating a new Connection
instance.  Once you are connect, you may execute any of Couchbases' numerous
operations against this connection.

Here is a simple example of instantiating a connection, setting a new document
into the bucket and then retrieving its contents:

```php
$cluster = new \Couchbase\Cluster('localhost');
$db = $cluster->openBucket('default');
$db->upsert('testdoc', array('name'=>'Frank'));
$res = $db->get('testdoc');
var_dump($res->value);
// array(1) {
//   ["name"]=>
//   string(5) "Frank"
// }
```

## Documentation

An extensive documentation is available on the Couchbase website.  Visit our
[PHP Community](https://forums.couchbase.com/c/php-sdk) on
the [Couchbase](http://developer.couchbase.com/documentation/server/current/sdk/php/start-using-sdk.html) website for the documentation as well as
numerous examples and samples.

To build PHP API reference, phpDocumentor have to be installed

    pear channel-discover pear.phpdoc.org
    pear install phpdoc/phpDocumentor

That will bring phpdoc command into PATH. The following steps assume that current directory is the root of this repository:

    ver=$(git describe | sed 's/^v//')
    rm -rf couchbase-php-client-$ver
    phpdoc --target couchbase-php-client-$ver --directory api

After that all reference documentation will be stored in couchbase-php-client-2.3.0, if current tag is 2.3.0.

## Source Control

The source code is available at
[https://github.com/couchbase/php-couchbase](https://github.com/couchbase/php-couchbase).

To execute our test suite, simply install and execute phpunit against your
checked out source code. Tests assume that you have Couchbase Server with
default bucket running on localhost (otherwise use environment variable
`CPDSN`, `CPBUCKET`, `CPUSER`, `CPPASS`. E.g. `CPDSN=couchbase://192.168.1.42/
CPBUCKET=travel-sample`).

```bash
curl -L https://phar.phpunit.de/phpunit.phar > ~/bin/phpunit
chmod a+x ~/bin/phpunit
# or just 'dnf install php-phpunit-PHPUnit' on Fedora 24+

phpunit tests/
```

Integration tests require some prerequisites, so once they met, you can run integration
tests with:

```bash
phpunit tests/
```


## License
Copyright 2016-2017 Couchbase Inc.

Licensed under the Apache License, Version 2.0.

See
[LICENSE](https://github.com/couchbaselabs/php-couchbase/blob/master/LICENSE)
for further details.
