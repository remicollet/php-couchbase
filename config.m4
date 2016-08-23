PHP_ARG_WITH(couchbase, whether to enable Couchbase support,
[  --with-couchbase   Include Couchbase support])

PHP_ARG_WITH(system-fastlz, wheter to use system FastLZ bibrary,
    [  --with-system-fastlz   Use system FastLZ bibrary], no, no)

if test "$PHP_COUCHBASE" != "no"; then
  if test -r $PHP_COUCHBASE/include/libcouchbase/couchbase.h; then
    LIBCOUCHBASE_DIR=$PHP_COUCHBASE
  else
    AC_MSG_CHECKING(for libcouchbase in default path)
    for i in /usr/local /usr; do
      if test -r $i/include/libcouchbase/couchbase.h; then
        LIBCOUCHBASE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$LIBCOUCHBASE_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please reinstall the libcouchbase distribution -
                 libcouchbase.h should be <libcouchbase-dir>/include and
                 libcouchbase.a should be in <libcouchbase-dir>/lib)
  fi
  PHP_ADD_INCLUDE($LIBCOUCHBASE_DIR/include)

  PHP_SUBST(COUCHBASE_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(couchbase, $LIBCOUCHBASE_DIR/lib,
               COUCHBASE_SHARED_LIBADD)

  AC_DEFINE(HAVE_COUCHBASE, 1, [Whether you have Couchbase])

  ifdef([PHP_ADD_EXTENDION_DEP], [
	PHP_ADD_EXTENSION_DEP(couchbase, json)
  ])

  PHP_SUBST(COUCHBASE_SHARED_LIBADD)

  COUCHBASE_FILES="bucket.c cas.c cluster.c couchbase.c docfrag.c exception.c get.c unlock.c metadoc.c opcookie.c paramparser.c transcoding.c touch.c remove.c subdoc.c store.c n1ql.c http.c counter.c durability.c n1ix_spec.c n1ix_list.c n1ix_create.c n1ix_drop.c cbft.c token.c log.c"

  if test "$PHP_SYSTEM_FASTLZ" != "no"; then
    AC_CHECK_HEADERS([fastlz.h])
    PHP_CHECK_LIBRARY(fastlz, fastlz_compress,
      [PHP_ADD_LIBRARY(fastlz, 1, COUCHBASE_SHARED_LIBADD)],
      [AC_MSG_ERROR(FastLZ library not found)])
  else
    COUCHBASE_FILES="${COUCHBASE_FILES} fastlz/fastlz.c"
  fi
  PHP_NEW_EXTENSION(couchbase, ${COUCHBASE_FILES}, $ext_shared)
  PHP_ADD_BUILD_DIR($ext_builddir/fastlz, 1)
fi
