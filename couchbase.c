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
#include "cas.h"
#include "metadoc.h"
#include "docfrag.h"
#include "token.h"
#include "n1ix_spec.h"
#include "phpstubstr.h"
#include "zap.h"
#include "ext/standard/info.h"

#ifdef HAVE_FASTLZ_H
#include <fastlz.h>
#else
#include "fastlz/fastlz.h"
#endif

#if HAVE_ZLIB
#include <zlib.h>
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/ext", __FILE__, __LINE__

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

ZEND_DECLARE_MODULE_GLOBALS(couchbase)

extern struct pcbc_logger_st pcbc_logger;

static PHP_INI_MH(OnUpdateLogLevel)
{
    const char *str_val =
#if PHP_VERSION_ID >= 70000
        ZSTR_VAL(new_value);
#else
        new_value;
#endif
    if (!new_value) {
        pcbc_logger.minlevel = LCB_LOG_WARN;
    } else if (!strcmp(new_value, "TRACE") || !strcmp(new_value, "TRAC")) {
        pcbc_logger.minlevel = LCB_LOG_TRACE;
    } else if (!strcmp(new_value, "DEBUG") || !strcmp(new_value, "DEBG")) {
        pcbc_logger.minlevel = LCB_LOG_DEBUG;
    } else if (!strcmp(new_value, "INFO")) {
        pcbc_logger.minlevel = LCB_LOG_INFO;
    } else if (!strcmp(new_value, "WARN")) {
        pcbc_logger.minlevel = LCB_LOG_WARN;
    } else if (!strcmp(new_value, "ERROR") || !strcmp(new_value, "EROR")) {
        pcbc_logger.minlevel = LCB_LOG_ERROR;
    } else if (!strcmp(new_value, "FATAL") || !strcmp(new_value, "FATL")) {
        pcbc_logger.minlevel = LCB_LOG_FATAL;
    } else {
        return FAILURE;
    }

#if PHP_VERSION_ID >= 70000
    return OnUpdateString(entry, new_value, mh_arg1, mh_arg2, mh_arg3, stage);
#else
    return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
#endif
}

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY(PCBC_INIENT_LOG_LEVEL, PCBC_INIDFL_LOG_LEVEL, PHP_INI_ALL,
                  OnUpdateLogLevel, log_level, zend_couchbase_globals, couchbase_globals)
PHP_INI_END()

#define PCBC_LONG_CONSTANT(key, val) \
	REGISTER_LONG_CONSTANT("COUCHBASE_"key, val, CONST_CS | CONST_PERSISTENT)
#define PCBC_REGISTER_CONST(c) \
	REGISTER_LONG_CONSTANT("COUCHBASE_"#c, c, CONST_CS | CONST_PERSISTENT)
#define PCBC_REGISTER_LCBCONST(c) \
	REGISTER_LONG_CONSTANT("COUCHBASE_"#c, LCB_##c, CONST_CS | CONST_PERSISTENT)

static void php_extname_init_globals(zend_couchbase_globals *couchbase_globals)
{
	couchbase_globals->first_bconn = NULL;
	couchbase_globals->last_bconn = NULL;
}

PHP_MINIT_FUNCTION(couchbase)
{
	ZEND_INIT_MODULE_GLOBALS(couchbase, php_extname_init_globals, NULL);
        REGISTER_INI_ENTRIES();

	couchbase_init_exceptions(INIT_FUNC_ARGS_PASSTHRU);
	couchbase_init_metadoc(INIT_FUNC_ARGS_PASSTHRU);
	couchbase_init_docfrag(INIT_FUNC_ARGS_PASSTHRU);
	couchbase_init_n1ix_spec(INIT_FUNC_ARGS_PASSTHRU);
	couchbase_init_cluster(INIT_FUNC_ARGS_PASSTHRU);
	couchbase_init_bucket(INIT_FUNC_ARGS_PASSTHRU);
	couchbase_init_token(INIT_FUNC_ARGS_PASSTHRU);

	PCBC_REGISTER_CONST(PERSISTTO_MASTER);
	PCBC_REGISTER_CONST(PERSISTTO_ONE);
	PCBC_REGISTER_CONST(PERSISTTO_TWO);
	PCBC_REGISTER_CONST(PERSISTTO_THREE);
	PCBC_REGISTER_CONST(REPLICATETO_ONE);
	PCBC_REGISTER_CONST(REPLICATETO_TWO);
	PCBC_REGISTER_CONST(REPLICATETO_THREE);

	PCBC_REGISTER_LCBCONST(SDCMD_GET);
	PCBC_REGISTER_LCBCONST(SDCMD_REPLACE);
	PCBC_REGISTER_LCBCONST(SDCMD_EXISTS);
	PCBC_REGISTER_LCBCONST(SDCMD_DICT_ADD);
	PCBC_REGISTER_LCBCONST(SDCMD_DICT_UPSERT);
	PCBC_REGISTER_LCBCONST(SDCMD_ARRAY_ADD_FIRST);
	PCBC_REGISTER_LCBCONST(SDCMD_ARRAY_ADD_LAST);
	PCBC_REGISTER_LCBCONST(SDCMD_ARRAY_INSERT);
	PCBC_REGISTER_LCBCONST(SDCMD_ARRAY_ADD_UNIQUE);
	PCBC_REGISTER_LCBCONST(SDCMD_COUNTER);
	PCBC_REGISTER_LCBCONST(SDCMD_REMOVE);

	PCBC_REGISTER_LCBCONST(CNTL_OP_TIMEOUT);
	PCBC_REGISTER_LCBCONST(CNTL_VIEW_TIMEOUT);
	PCBC_REGISTER_LCBCONST(CNTL_DURABILITY_INTERVAL);
	PCBC_REGISTER_LCBCONST(CNTL_DURABILITY_TIMEOUT);
	PCBC_REGISTER_LCBCONST(CNTL_HTTP_TIMEOUT);
	PCBC_REGISTER_LCBCONST(CNTL_CONFIGURATION_TIMEOUT);
	PCBC_REGISTER_LCBCONST(CNTL_CONFDELAY_THRESH);
	PCBC_REGISTER_LCBCONST(CNTL_CONFIG_NODE_TIMEOUT);
	PCBC_REGISTER_LCBCONST(CNTL_HTCONFIG_IDLE_TIMEOUT);

	PCBC_LONG_CONSTANT("VALUE_F_JSON", LCB_VALUE_F_JSON);

        PCBC_REGISTER_LCBCONST(N1XSPEC_T_DEFAULT);
        PCBC_REGISTER_LCBCONST(N1XSPEC_T_GSI);
        PCBC_REGISTER_LCBCONST(N1XSPEC_T_VIEW);

    {
        char buf[128];

#if PHP_VERSION_ID >= 70000
#define PCBC_CONST_LENGTH(buf) (strlen(buf))
#else
#define PCBC_CONST_LENGTH(buf) (strlen(buf) + 1)
#endif
#define X(name, value, cls, s)                                          \
        {                                                               \
            ap_php_snprintf(buf, sizeof(buf), "COUCHBASE_%s", #name + 4); \
            zend_register_long_constant(buf, PCBC_CONST_LENGTH(buf), value, \
                                        CONST_CS | CONST_PERSISTENT,    \
                                        module_number TSRMLS_CC);      \
        }

        LCB_XERR(X)
#undef X
#undef PCBC_CONST_LENGTH
    }

	// TODO: Maybe not have these?
	PCBC_LONG_CONSTANT("TMPFAIL", LCB_ETMPFAIL);
	PCBC_LONG_CONSTANT("KEYALREADYEXISTS", LCB_KEY_EEXISTS);
	PCBC_LONG_CONSTANT("KEYNOTFOUND", LCB_KEY_ENOENT);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(couchbase)
{
	couchbase_shutdown_bucket(SHUTDOWN_FUNC_ARGS_PASSTHRU);
        UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(couchbase)
{
	if (PG(last_error_message) && PG(last_error_type) == E_ERROR) {
		couchbase_shutdown_bucket(SHUTDOWN_FUNC_ARGS_PASSTHRU);
	}
	return SUCCESS;
}

PHP_RINIT_FUNCTION(couchbase)
{
    int stub_idx;
    for (stub_idx = 0; stub_idx < sizeof(PCBC_PHP_CODESTR) / sizeof(pcbc_stub_data); ++stub_idx) {
        pcbc_stub_data *this_stub = &PCBC_PHP_CODESTR[stub_idx];
        int retval = zend_eval_string((char*)this_stub->data, NULL, (char*)this_stub->filename TSRMLS_CC);
        if (retval != SUCCESS) {
            pcbc_log(LOGARGS(ERROR), "Failed to inject Couchbase stub: %s.", (char *)this_stub->filename);
            return FAILURE;
        }
    }
    return SUCCESS;
}

PHP_FUNCTION(couchbase_zlib_compress)
{
#if HAVE_ZLIB
    zval *zdata;
    void *dataIn, *dataOut;
    unsigned long dataSize, dataOutSize;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
            "z", &zdata) == FAILURE) {
        RETURN_NULL();
    }

    dataIn = Z_STRVAL_P(zdata);
    dataSize = Z_STRLEN_P(zdata);
    dataOutSize = compressBound(dataSize);
    dataOut = emalloc(dataOutSize);
    compress((uint8_t*)dataOut + 4, &dataOutSize, dataIn, dataSize);
    *(uint32_t*)dataOut = dataSize;

    zap_zval_stringl_p(return_value, dataOut, 4 + dataOutSize);
    efree(dataOut);
#else
    zend_throw_exception(NULL, "The zlib library was not available when the couchbase extension was built.", 0 TSRMLS_CC);
#endif
}

PHP_FUNCTION(couchbase_zlib_decompress)
{
#if HAVE_ZLIB
    zval *zdata;
    void *dataIn, *dataOut;
    unsigned long dataSize, dataOutSize;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
            "z", &zdata) == FAILURE) {
        RETURN_NULL();
    }

    dataIn = Z_STRVAL_P(zdata);
    dataSize = Z_STRLEN_P(zdata);
    dataOutSize = *(uint32_t*)dataIn;
    dataOut = emalloc(dataOutSize);
    uncompress(dataOut, &dataOutSize, (uint8_t*)dataIn + 4, dataSize - 4);

    zap_zval_stringl_p(return_value, dataOut, dataOutSize);
    efree(dataOut);
#else
    zend_throw_exception(NULL, "The zlib library was not available when the couchbase extension was built.", 0 TSRMLS_CC);
#endif
}

PHP_FUNCTION(couchbase_fastlz_compress)
{
    zval *zdata;
    void *dataIn, *dataOut;
    unsigned long dataSize, dataOutSize;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
            "z", &zdata) == FAILURE) {
        RETURN_NULL();
    }

    dataIn = Z_STRVAL_P(zdata);
    dataSize = Z_STRLEN_P(zdata);
    dataOutSize = 4 + (dataSize * 1.05);
    dataOut = emalloc(dataOutSize);
    dataOutSize = fastlz_compress(dataIn, dataSize, (uint8_t*)dataOut + 4);
    *(uint32_t*)dataOut = dataSize;

    zap_zval_stringl_p(return_value, dataOut, 4 + dataOutSize);

    efree(dataOut);
}

PHP_FUNCTION(couchbase_fastlz_decompress)
{
    zval *zdata;
    void *dataIn, *dataOut;
    unsigned long dataSize, dataOutSize;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
            "z", &zdata) == FAILURE) {
        RETURN_NULL();
    }

    dataIn = Z_STRVAL_P(zdata);
    dataSize = Z_STRLEN_P(zdata);
    dataOutSize = *(uint32_t*)dataIn;
    dataOut = emalloc(dataOutSize);
    dataOutSize = fastlz_decompress(
            (uint8_t*)dataIn + 4, dataSize - 4, dataOut, dataOutSize);

    zap_zval_stringl_p(return_value, dataOut, dataOutSize);

    efree(dataOut);
}

static PHP_MINFO_FUNCTION(couchbase)
{
    char buf[128];
    const char *changeset;
    lcb_error_t err;

    err = lcb_cntl(NULL, LCB_CNTL_GET, LCB_CNTL_CHANGESET, (void*)&changeset);
    if (err != LCB_SUCCESS) {
        changeset = "UNKNOWN";
    }
    ap_php_snprintf(buf, sizeof(buf), "%s (git: %s)", lcb_get_version(NULL), changeset);

    php_info_print_table_start();
    php_info_print_table_row(2, "couchbase support", "enabled");
    php_info_print_table_row(2, "extension version", PHP_COUCHBASE_VERSION);
    php_info_print_table_row(2, "libcouchbase runtime version", buf);
    php_info_print_table_row(2, "libcouchbase headers version", LCB_VERSION_STRING " (git: " LCB_VERSION_CHANGESET ")");
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}

static zend_function_entry couchbase_functions[] = {
    PHP_FE(couchbase_fastlz_compress, NULL)
    PHP_FE(couchbase_fastlz_decompress, NULL)
    PHP_FE(couchbase_zlib_compress, NULL)
    PHP_FE(couchbase_zlib_decompress, NULL)
    {NULL, NULL, NULL}
};

#if ZEND_MODULE_API_NO >= 20050617
static zend_module_dep php_couchbase_deps[] = {
        ZEND_MOD_REQUIRED("json")
		{NULL,NULL,NULL}
};
#endif

zend_module_entry couchbase_module_entry = {
#if ZEND_MODULE_API_NO >= 20050617
	STANDARD_MODULE_HEADER_EX,
	NULL,
	php_couchbase_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_COUCHBASE_EXTNAME,
	couchbase_functions,
	PHP_MINIT(couchbase),
	PHP_MSHUTDOWN(couchbase),
	PHP_RINIT(couchbase),
	PHP_RSHUTDOWN(couchbase),
	PHP_MINFO(couchbase),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_COUCHBASE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_COUCHBASE
ZEND_GET_MODULE(couchbase)
#endif
