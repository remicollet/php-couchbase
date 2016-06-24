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

#include "bucket.h"
#include "cas.h"
#include "transcoding.h"
#include "exception.h"

zend_class_entry *token_ce;

zend_function_entry token_methods[] = {
    { NULL, NULL, NULL }
};

void couchbase_init_token(INIT_FUNC_ARGS) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "CouchbaseMutationToken", token_methods);
    token_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(token_ce, "bucket", strlen("bucket"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(token_ce, "vbucketID", strlen("vbucketID"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(token_ce, "vbucketUUID", strlen("vbucketUUID"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(token_ce, "sequenceNumber", strlen("sequenceNumber"), ZEND_ACC_PUBLIC TSRMLS_CC);
}

int pcbc_make_token(zval *obj, const char *bucket, lcb_U16 vbucketID, lcb_U64 vbucketUUID, lcb_U64 seqno TSRMLS_DC)
{
    char buf[64] = {0};
    zapval zbucket, zvbucketID, zvbucketUUID, zseqno;
    object_init_ex(obj, token_ce);

    zapval_alloc_stringl(zbucket, bucket, strlen(bucket));
    zend_update_property(token_ce, obj, "bucket", sizeof("bucket") - 1, zapval_zvalptr(zbucket) TSRMLS_CC);

    zapval_alloc_long(zvbucketID, vbucketID);
    zend_update_property(token_ce, obj, "vbucketID", sizeof("vbucketID") - 1, zapval_zvalptr(zvbucketID) TSRMLS_CC);

    sprintf(buf, "%llu", vbucketUUID);
    zapval_alloc_stringl(zvbucketUUID, buf, strlen(buf));
    zend_update_property(token_ce, obj, "vbucketUUID", sizeof("vbucketUUID") - 1, zapval_zvalptr(zvbucketUUID) TSRMLS_CC);

    zapval_alloc_long(zseqno, seqno);
    zend_update_property(token_ce, obj, "sequenceNumber", sizeof("sequenceNumber") - 1, zapval_zvalptr(zseqno) TSRMLS_CC);

    return SUCCESS;
}
