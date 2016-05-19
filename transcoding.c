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

#include "transcoding.h"
#include "zap.h"

int pcbc_decode_value(bucket_object *bucket, zapval *zvalue,
        zapval *zbytes, zapval *zflags, zapval *zdatatype TSRMLS_DC) {
    zapval zparams[] = { *zbytes, *zflags, *zdatatype };

    if (call_user_function(CG(function_table), NULL,
            zapval_zvalptr(bucket->decoder), zapval_zvalptr_p(zvalue),
            3, zparams TSRMLS_CC) != SUCCESS)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int pcbc_encode_value(bucket_object *bucket, zval *value,
        void **bytes, lcb_size_t *nbytes, lcb_uint32_t *flags,
        lcb_uint8_t *datatype TSRMLS_DC) {
    zapval *zpbytes, *zpflags, *zpdatatype;
    zapval zretval;
    HashTable *retval;

    zapval_alloc_null(zretval);

    if (call_user_function(CG(function_table), NULL,
            zapval_zvalptr(bucket->encoder), zapval_zvalptr(zretval),
            1, zapvalptr_from_zvalptr(value) TSRMLS_CC) != SUCCESS) {
        zapval_destroy(zretval);
        return FAILURE;
    }

    if (!zapval_is_array(zretval)) {
        zapval_destroy(zretval);
        return FAILURE;
    }

    retval = zapval_arrval(zretval);

    if (zend_hash_num_elements(retval) != 3) {
        zapval_destroy(zretval);
        return FAILURE;
    }

    zpbytes = zap_hash_index_find(retval, 0);
    zpflags = zap_hash_index_find(retval, 1);
    zpdatatype = zap_hash_index_find(retval, 2);

    if (!zapval_is_string_p(zpbytes)) {
        zapval_destroy(zretval);
        return FAILURE;
    }
    if (!zapval_is_long_p(zpflags)) {
        zapval_destroy(zretval);
        return FAILURE;
    }
    if (!zapval_is_long_p(zpdatatype)) {
        zapval_destroy(zretval);
        return FAILURE;
    }

    *nbytes = zapval_strlen_p(zpbytes);
    *bytes = estrndup(zapval_strval_p(zpbytes), *nbytes);
    *flags = zapval_lval_p(zpflags);
    *datatype = (lcb_uint8_t)zapval_lval_p(zpdatatype);

    zapval_destroy(zretval);
    return SUCCESS;
}
