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
CB_USE_VALGRIND=${CB_USE_VALGRIND:-no}

cd ${PROJECT_ROOT}

COUCHBASE_EXT=${PROJECT_ROOT}/modules/couchbase.so

PHP_EXECUTABLE="${CB_PHP_PREFIX}/bin/php"
if [ "x${CB_USE_VALGRIND}" = "xyes" ]
then
    export USE_ZEND_ALLOC=0
    export USE_DONT_UNLOAD_MODULES=0
    CB_VALGRIND_LOG=${CB_VALGRIND_LOG:-"${PROJECT_ROOT}/build/valgrind.log"}
    CB_VALGRIND_ARGS=${CB_VALGRIND_ARGS:-"--tool=memcheck --num-callers=30"}
    PHP_EXECUTABLE="valgrind ${CB_VALGRIND_ARGS} --log-file=${CB_VALGRIND_LOG} ${PHP_EXECUTABLE}"
fi

${PHP_EXECUTABLE} \
    -d extension=${COUCHBASE_EXT} \
    $*
