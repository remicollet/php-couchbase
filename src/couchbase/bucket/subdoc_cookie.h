/**
 *     Copyright 2016-2020 Couchbase, Inc.
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

#ifndef BUCKET_SUBDOC_COOKIE_H
#define BUCKET_SUBDOC_COOKIE_H

struct subdoc_cookie {
    lcb_STATUS rc;
    zval *return_value;
    zend_bool is_get;
    zend_bool with_expiry;
};

#endif // BUCKET_SUBDOC_COOKIE_H
