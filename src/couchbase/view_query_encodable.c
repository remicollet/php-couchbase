/**
 *     Copyright 2017-2019 Couchbase, Inc.
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

#include "couchbase.h"

zend_class_entry *pcbc_view_query_encodable_ce;

ZEND_BEGIN_ARG_INFO_EX(ai_ViewQueryEncodable_encode, 0, 0, 0)
ZEND_END_ARG_INFO()

// clang-format off
static const zend_function_entry view_query_encodable_interface[] = {
    PHP_ABSTRACT_ME(ViewQueryEncodable, encode, ai_ViewQueryEncodable_encode)
    PHP_FE_END
};
// clang-format on

PHP_MINIT_FUNCTION(ViewQueryEncodable)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ViewQueryEncodable", view_query_encodable_interface);
    pcbc_view_query_encodable_ce = zend_register_internal_interface(&ce TSRMLS_CC);
    return SUCCESS;
}
