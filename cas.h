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

#ifndef CAS_H_
#define CAS_H_

#include <php.h>
#include <libcouchbase/couchbase.h>
#include "zap.h"

lcb_cas_t cas_decode(zval *zcas TSRMLS_DC);
void cas_encode(zapval *casout, lcb_cas_t value TSRMLS_DC);

#endif // CAS_H_
