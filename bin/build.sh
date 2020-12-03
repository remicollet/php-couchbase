#!/usr/bin/env bash

#    Copyright 2020 Couchbase, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

PROJECT_ROOT="$( cd "$(dirname "$0"/..)" >/dev/null 2>&1 ; pwd -P )"

set -x
set -e

CB_PHP_PREFIX=${CB_PHP_PREFIX:-/usr}
CB_LCB_PREFIX=${CB_LCB_PREFIX:-/usr}

${CB_PHP_PREFIX}/bin/php --version
${CB_PHP_PREFIX}/bin/php --ini

cd ${PROJECT_ROOT}

${CB_PHP_PREFIX}/bin/phpize
./configure --with-couchbase=${CB_LCB_PREFIX} --with-php-config=${CB_PHP_PREFIX}/bin/php-config
make clean
make -j8

COUCHBASE_EXT=${PROJECT_ROOT}/modules/couchbase.so

${CB_PHP_PREFIX}/bin/php -d extension=${COUCHBASE_EXT} -m | grep couchbase
${CB_PHP_PREFIX}/bin/php -d extension=${COUCHBASE_EXT} -i | grep couchbase

cat > ${PROJECT_ROOT}/build/try_to_load.php <<EOF
<?php
new \Couchbase\ClusterOptions();
EOF

${CB_PHP_PREFIX}/bin/php -d extension=${COUCHBASE_EXT} ${PROJECT_ROOT}/build/try_to_load.php
