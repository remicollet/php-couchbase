/**
 *     Copyright 2016 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#ifndef CLUSTER_H_
#define CLUSTER_H_

#include <php.h>
#include "couchbase.h"
#include "zap.h"

typedef struct cluster_object {
    zap_ZEND_OBJECT_START

    lcb_t lcb;

    zap_ZEND_OBJECT_END
} cluster_object;

#endif // CLUSTER_H_
