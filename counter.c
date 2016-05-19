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

#include "couchbase.h"
#include "ext/standard/php_var.h"
#include "ext/json/php_json.h"
#include "exception.h"
#include "datainfo.h"
#include "paramparser.h"
#include "zap.h"
#include "bucket.h"
#include "cas.h"
#include "metadoc.h"
#include "docfrag.h"
#include "transcoding.h"
#include "opcookie.h"

typedef struct {
    opcookie_res header;
    zapval key;
    zapval value;
    zapval cas;
} opcookie_arithmetic_res;

void counter_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_arithmetic_res *result = ecalloc(1, sizeof(opcookie_arithmetic_res));
    const lcb_RESPCOUNTER *resp = (const lcb_RESPCOUNTER *)rb;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    zapval_alloc_stringl(result->key, resp->key, resp->nkey);
    zapval_alloc_long(result->value, resp->value);
    cas_encode(&result->cas, resp->cas TSRMLS_CC);

    opcookie_push((opcookie*)rb->cookie, &result->header);
}

static lcb_error_t proc_arithmetic_results(bucket_object *bucket, zval *return_value,
                                           opcookie *cookie, int is_mapped TSRMLS_DC)
{
    opcookie_arithmetic_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // If we are not mapped, we need to throw any op errors
    if (is_mapped == 0) {
        err = opcookie_get_first_error(cookie);
    }

    if (err == LCB_SUCCESS) {
        FOREACH_OPCOOKIE_RES(opcookie_arithmetic_res, res, cookie) {
            zval *doc = bop_get_return_doc(
                return_value, &res->key, is_mapped);

            if (res->header.err == LCB_SUCCESS) {
                make_metadoc(doc, &res->value, NULL, &res->cas TSRMLS_CC);
            } else {
                make_metadoc_error(doc, res->header.err TSRMLS_CC);
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_arithmetic_res, res, cookie) {
        zapval_destroy(res->key);
        zapval_destroy(res->value);
        zapval_destroy(res->cas);
    }

    return err;
}

// counter($id, $delta {, $initial, $expiry}) : MetaDoc
PHP_METHOD(Bucket, counter)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zdelta, *zinitial, *zexpiry, *zgroupid;
    opcookie *cookie;
    lcb_error_t err = LCB_SUCCESS;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state,
                      "id|delta|initial,expiry,groupid",
                      &id, &zdelta, &zinitial, &zexpiry, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDCOUNTER cmd = {0};
        PCBC_CHECK_ZVAL_LONG(zdelta, "delta must be an integer");
        PCBC_CHECK_ZVAL_LONG(zinitial, "initial must be an integer");
        PCBC_CHECK_ZVAL_LONG(zexpiry, "expiry must be an integer");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        LCB_CMD_SET_KEY(&cmd, id.str, id.len);
        cmd.delta = Z_LVAL_P(zdelta);
        if (zinitial) {
            cmd.initial = Z_LVAL_P(zinitial);
            cmd.create = 1;
        }
        if (zexpiry) {
            cmd.exptime = Z_LVAL_P(zexpiry);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        err = lcb_counter3(data->conn->lcb, cookie, &cmd);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("counter", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_arithmetic_results(data, return_value,
                                      cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
