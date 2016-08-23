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

#ifndef COUCHBASE_H_
#define COUCHBASE_H_

#include <libcouchbase/couchbase.h>
#include <libcouchbase/api3.h>
#include <libcouchbase/views.h>
#include <libcouchbase/n1ql.h>
#include <libcouchbase/cbft.h>
#include <libcouchbase/ixmgmt.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <php.h>
#include <zend_exceptions.h>
#include "php_couchbase.h"
#include "log.h"

enum pcbc_constants {
	PERSISTTO_ONE = 1,
	PERSISTTO_TWO = 2,
	PERSISTTO_THREE = 4,
	PERSISTTO_MASTER = PERSISTTO_ONE,
	REPLICATETO_ONE = 1 << 4,
	REPLICATETO_TWO = 2 << 4,
	REPLICATETO_THREE = 4 << 4
};

typedef struct pcbc_lcb {
	char *key;
	char *bucket;
	lcb_t lcb;
	void *next;
} pcbc_lcb;

ZEND_BEGIN_MODULE_GLOBALS(couchbase)
	// Linked list of bucket connections
	pcbc_lcb *first_bconn;
	pcbc_lcb *last_bconn;
	char *log_level;
ZEND_END_MODULE_GLOBALS(couchbase)
ZEND_EXTERN_MODULE_GLOBALS(couchbase)

#ifdef ZTS
#define PCBCG(v) TSRMG(couchbase_globals_id, zend_couchbase_globals *, v)
#else
#define PCBCG(v) (couchbase_globals.v)
#endif

void couchbase_init_exceptions(INIT_FUNC_ARGS);
void couchbase_init_cluster(INIT_FUNC_ARGS);
void couchbase_init_bucket(INIT_FUNC_ARGS);

void couchbase_shutdown_bucket(SHUTDOWN_FUNC_ARGS);

#define PCBC_INIENT_LOG_LEVEL "couchbase.log_level"
#define PCBC_INIDFL_LOG_LEVEL "WARN"

#endif /* COUCHBASE_H_ */
