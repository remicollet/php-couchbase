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

#include "expiry_util.h"

#include <ext/date/php_date.h>

extern struct pcbc_logger_st pcbc_logger;
#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/pool", __FILE__, __LINE__

#define RELATIVE_EXPIRY_CUTOFF_SECONDS (30 * 24 * 60 * 60)
#define WORKAROUND_EXPIRY_CUTOFF_SECONDS (50 * 365 * 24 * 60 * 60)

zend_long pcbc_extract_expiry_time(zval *expiry)
{

    if (Z_TYPE_P(expiry) == IS_LONG) {
        zend_long relative_time = Z_LVAL_P(expiry);
        if (relative_time < RELATIVE_EXPIRY_CUTOFF_SECONDS) {
            // looks like valid relative duration as specified in protocol (less than 30 days)
            return relative_time;
        }
        if (relative_time > WORKAROUND_EXPIRY_CUTOFF_SECONDS) {
            pcbc_log(LOGARGS(WARN),
                     "The specified expiry duration %lu is longer than 50 years. For bug-compatibility with previous "
                     "versions of SDK 3.0.x, the number of seconds in the duration will be interpreted as the epoch "
                     "second when the document should expire (#{effective_expiry}). Stuffing an epoch second into a "
                     "Duration is deprecated and will no longer work in SDK 3.1. Consider using Time instance instead.",
                     (unsigned long)(relative_time));
            return relative_time;
        }
        return php_time() + relative_time;
    } else if (Z_TYPE_P(expiry) == IS_OBJECT &&
               instanceof_function(Z_OBJCE_P(expiry), php_date_get_interface_ce()) != 0) {
        zval zfuncname;
        zval retval;
        ZVAL_STRING(&zfuncname, "getTimestamp");
        int rv = call_user_function(NULL, expiry, &zfuncname, &retval, 0, NULL);
        zval_ptr_dtor(&zfuncname);
        if (rv == SUCCESS && Z_TYPE(retval) == IS_LONG) {
            return Z_LVAL(retval);
        }
    }
    return 0;
}
