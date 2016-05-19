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

#ifndef TRANSCODING_H_
#define TRANSCODING_H_

#include <php.h>
#include <libcouchbase/couchbase.h>
#include "zap.h"
#include "bucket.h"

int pcbc_decode_value(bucket_object *bucket, zapval *zvalue,
        zapval *zbytes, zapval *zflags, zapval *zdatatype TSRMLS_DC);

int pcbc_encode_value(bucket_object *bucket, zval *value,
	void **bytes, lcb_size_t *nbytes, lcb_uint32_t *flags,
	lcb_uint8_t *datatype TSRMLS_DC);

#endif // TRANSCODING_H_
