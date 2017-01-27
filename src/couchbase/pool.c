/**
 *     Copyright 2017 Couchbase, Inc.
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
#include <ext/standard/url.h>

extern struct pcbc_logger_st pcbc_logger;
#define LOGARGS(conn, lvl) LCB_LOG_##lvl, conn, "pcbc/pool", __FILE__, __LINE__

char *pcbc_client_string = "PCBC/" PHP_COUCHBASE_VERSION;

void get_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void store_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void unlock_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void remove_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void touch_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void counter_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void subdoc_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void http_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
void durability_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_durability_resp_t *resp);

static lcb_error_t pcbc_establish_connection(lcb_type_t type, lcb_t *result, const char *connstr, const char *name,
                                             lcb_AUTHENTICATOR *auth, char *auth_hash TSRMLS_DC)
{
    struct lcb_create_st create_options;
    lcb_error_t err;
    lcb_t conn;

    memset(&create_options, 0, sizeof(create_options));
    create_options.version = 3;
    create_options.v.v3.connstr = connstr;
    create_options.v.v3.type = type;
    err = lcb_create(&conn, &create_options);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(NULL, ERROR), "Failed to initialize LCB connection: %s", pcbc_lcb_strerror(err));
        return err;
    }
    pcbc_log(LOGARGS(conn, INFO), "New lcb_t instance has been initialized");
    lcb_set_auth(conn, auth);
    lcbauth_unref(auth);
    pcbc_log(LOGARGS(conn, INFO), "Using authenticator. md5=\"%s\"", auth_hash);
    err = lcb_cntl(conn, LCB_CNTL_SET, LCB_CNTL_LOGGER, &pcbc_logger);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(conn, ERROR), "Failed to configure LCB logging hook: %s", pcbc_lcb_strerror(err));
        lcb_destroy(conn);
        return err;
    }
    err = lcb_cntl(conn, LCB_CNTL_SET, LCB_CNTL_CLIENT_STRING, pcbc_client_string);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(conn, ERROR), "Failed to configure LCB client string: %s", pcbc_lcb_strerror(err));
        lcb_destroy(conn);
        return err;
    }

    lcb_install_callback3(conn, LCB_CALLBACK_GET, get_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_GETREPLICA, get_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_STORE, store_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_STOREDUR, store_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_UNLOCK, unlock_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_REMOVE, remove_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_TOUCH, touch_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_COUNTER, counter_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_SDLOOKUP, subdoc_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_SDMUTATE, subdoc_callback);
    lcb_install_callback3(conn, LCB_CALLBACK_HTTP, http_callback);

    err = lcb_connect(conn);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(conn, ERROR), "Failed to connect LCB connection: %s", pcbc_lcb_strerror(err));
        lcb_destroy(conn);
        return err;
    }
    // We use lcb_wait here as no callbacks are invoked by connect.
    lcb_wait(conn);
    err = lcb_get_bootstrap_status(conn);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(conn, ERROR), "Failed to bootstrap LCB connection: %s", pcbc_lcb_strerror(err));
        lcb_destroy(conn);
        return err;
    }

    *result = conn;
    return LCB_SUCCESS;
}

static lcb_error_t pcbc_normalize_connstr(lcb_type_t type, char *connstr, const char *bucketname,
                                          char **normalized TSRMLS_DC)
{
    php_url *url;
    zend_bool need_free = 0;

    if (type != LCB_TYPE_BUCKET && type != LCB_TYPE_CLUSTER) {
        return LCB_EINVAL;
    }

    url = php_url_parse(connstr);
    if (!url) {
        return LCB_EINVAL;
    }
    if (url->scheme == NULL && url->host == NULL) {
        // hosts has gone into path section,
        // explicitly prepend schema to workaround it
        smart_str buf = {0};
        char *with_schema = NULL;
        smart_str_appendl(&buf, "couchbase://", 12);
        smart_str_appends(&buf, connstr);
        smart_str_0(&buf);
        PCBC_SMARTSTR_DUP(buf, with_schema);
        pcbc_log(LOGARGS(NULL, DEBUG), "Rewrite connection string from \"%s\" to \"%s\"", connstr, with_schema);
        need_free = 1;
        connstr = with_schema;
        php_url_free(url);
        url = php_url_parse(connstr);
        if (!url) {
            efree(with_schema);
            return LCB_EINVAL;
        }
    }
    switch (type) {
    case LCB_TYPE_BUCKET:
        if (bucketname && bucketname[0] != '\0') {
            // rebuild connection string with username as the bucket
            smart_str buf = {0};
            if (url->scheme) {
                smart_str_appends(&buf, url->scheme);
                smart_str_appendl(&buf, "://", 3);
            }
            if (url->host) {
                smart_str_appends(&buf, url->host);
            }
            if (url->port) {
                smart_str_appendc(&buf, ':');
                smart_str_append_long(&buf, (long)url->port);
            }
            smart_str_appendc(&buf, '/');
            smart_str_appends(&buf, bucketname);
            if (url->query) {
                smart_str_appendc(&buf, '?');
                smart_str_appends(&buf, url->query);
            }
            smart_str_0(&buf);
            PCBC_SMARTSTR_DUP(buf, *normalized);
            smart_str_free(&buf);
            pcbc_log(LOGARGS(NULL, DEBUG), "Rewrite connection string from \"%s\" to \"%s\"", connstr, *normalized);
        } else {
            *normalized = estrdup(connstr);
        }
        break;
    case LCB_TYPE_CLUSTER:
        if (url->path != NULL && url->path[0] != '\0') {
            // strip bucket from the connection string
            smart_str buf = {0};
            if (url->scheme) {
                smart_str_appends(&buf, url->scheme);
                smart_str_appendl(&buf, "://", 3);
            }
            if (url->host) {
                smart_str_appends(&buf, url->host);
            }
            if (url->port) {
                smart_str_appendc(&buf, ':');
                smart_str_append_long(&buf, (long)url->port);
            }
            if (url->query) {
                smart_str_appendc(&buf, '?');
                smart_str_appends(&buf, url->query);
            }
            smart_str_0(&buf);
            PCBC_SMARTSTR_DUP(buf, *normalized);
            smart_str_free(&buf);
            pcbc_log(LOGARGS(NULL, DEBUG), "Rewrite connection string from \"%s\" to \"%s\"", connstr, *normalized);
        } else {
            *normalized = estrdup(connstr);
        }
        break;
    }
    php_url_free(url);
    if (need_free) {
        efree(connstr);
    }
    return LCB_SUCCESS;
}

static pcbc_connection_t *pcbc_connection_lookup(lcb_type_t type, const char *connstr, const char *auth_hash TSRMLS_DC)
{
    pcbc_connection_pool_t *pool = &PCBCG(pool);
    pcbc_connection_t *iter = pool->head;

    for (iter = pool->head; iter; iter = iter->next) {
        if (iter->refs == 0) {
            continue;
        }
        if (iter->type != type) {
            continue;
        }
        if (strcmp(iter->connstr, connstr) != 0) {
            continue;
        }
        if (iter->auth_hash != NULL) {
            if (strcmp(iter->auth_hash, auth_hash) != 0) {
                continue;
            }
        } else if (auth_hash != NULL) {
            continue;
        }
        return iter;
    }
    return NULL;
}

static void pcbc_connection_cache(pcbc_connection_t *conn TSRMLS_DC)
{
    pcbc_connection_pool_t *pool = &PCBCG(pool);

    conn->next = NULL;
    if (pool->tail != NULL) {
        pool->tail->next = conn;
    } else {
        pool->tail = conn;
    }
    if (pool->head == NULL) {
        pool->head = conn;
    }
    pool->nconn++;
}

int pcbc_connection_addref(pcbc_connection_t *conn TSRMLS_DC)
{
    if (conn) {
        conn->refs++;
        return SUCCESS;
    }
    return FAILURE;
}

int pcbc_connection_delref(pcbc_connection_t *conn TSRMLS_DC)
{
    if (conn) {
        conn->refs--;
        if (conn->refs <= 0) {
            pcbc_connection_pool_t *pool = &PCBCG(pool);
            pcbc_connection_t *iter = pool->head;

            pcbc_log(LOGARGS(conn->lcb, INFO), "Connection refcounter is zero. Release resources");
            pool->nconn--;
            if (pool->head == conn) {
                pool->head = conn->next;
                if (pool->head == NULL) {
                    pool->tail = NULL;
                }
            } else {
                for (iter = pool->head; iter; iter = iter->next) {
                    if (iter->next == conn) {
                        iter->next = conn->next;
                        if (iter->next == NULL) {
                            pool->tail = iter;
                        }
                        break;
                    }
                }
            }
            efree(conn->connstr);
            if (conn->bucketname) {
                efree(conn->bucketname);
                conn->bucketname = NULL;
            }
            if (conn->auth_hash) {
                efree(conn->auth_hash);
                conn->auth_hash = NULL;
            }
            lcb_destroy(conn->lcb);
            conn->lcb = NULL;
            efree(conn);
            return SUCCESS;
        }
        pcbc_log(LOGARGS(conn->lcb, INFO), "Destroying connection, ref=%d", conn->refs);
        return SUCCESS;
    }
    return FAILURE;
}

lcb_error_t pcbc_connection_get(pcbc_connection_t **result, lcb_type_t type, const char *connstr,
                                const char *bucketname, lcb_AUTHENTICATOR *auth, char *auth_hash TSRMLS_DC)
{
    char *cstr = NULL;
    lcb_error_t rv;
    lcb_t lcb;
    pcbc_connection_t *conn = NULL;

    rv = pcbc_normalize_connstr(type, (char *)connstr, bucketname, &cstr TSRMLS_CC);
    if (rv != LCB_SUCCESS) {
        pcbc_log(LOGARGS(NULL, ERROR), "Failed to normalize connection string: %s", connstr);
        return rv;
    }

    conn = pcbc_connection_lookup(type, cstr, auth_hash TSRMLS_CC);
    if (conn) {
        efree(cstr);
        conn->refs++;
        *result = conn;
        pcbc_log(LOGARGS(conn->lcb, INFO), "Connection has been fetched from pool ref=%d", conn->refs);
        return LCB_SUCCESS;
    }

    rv = pcbc_establish_connection(type, &lcb, cstr, bucketname, auth, auth_hash TSRMLS_CC);
    if (rv != LCB_SUCCESS) {
        efree(cstr);
        return rv;
    }

    conn = emalloc(sizeof(pcbc_connection_t));
    conn->refs = 1;
    conn->type = type;
    conn->connstr = cstr;
    conn->bucketname = NULL;
    conn->auth_hash = estrdup(auth_hash);
    if (type == LCB_TYPE_BUCKET) {
        char *tmp;
        lcb_cntl(lcb, LCB_CNTL_GET, LCB_CNTL_BUCKETNAME, &tmp);
        if (tmp) {
            conn->bucketname = estrdup(tmp);
        }
    }
    conn->lcb = lcb;
    pcbc_connection_cache(conn TSRMLS_CC);
    pcbc_log(LOGARGS(conn->lcb, INFO), "Put connection into the pool ref=%d", conn->refs);
    *result = conn;

    return LCB_SUCCESS;
}
