#ifndef __SQL_H
#define __SQL_H

/**
 * \file sql.c
 *
 * \brief Code to support PennMUSH connection to SQL databases.
 *
 * \verbatim
 * Each sql database we support must define its own set of the
 * following functions:
 *
 *  penn_<db>_sql_init
 *  penn_<db>_sql_connected
 *  penn_<db>_sql_errormsg
 *  penn_<db>_sql_shutdown
 *  penn_<db>_sql_query
 *  penn_<db>_free_sql_query
 *
 * We define generic functions (named as above, but without <db>_)
 * that determine the platform and call the appropriate platform-specific
 * function. We also define the softcode interfaces:
 *
 *  fun_sql_escape
 *  fun_sql
 *  fun_mapsql
 *  cmd_sql
 *
 * \endverbatim
 */

#include "copyrite.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#define sleep Sleep
#endif

#ifdef HAVE_MYSQL
#include <mysql.h>
#include <errmsg.h>
static MYSQL *mysql_connp = NULL;
#endif

#ifdef HAVE_POSTGRESQL
#include <libpq-fe.h>
static PGconn *postgres_connp = NULL;
#endif

#ifdef HAVE_SQLITE3
#include "sqlite3.h"
static sqlite3 *sqlite3_connp = NULL;
#endif

#include "ansi.h"
#include "command.h"
#include "conf.h"
#include "dbdefs.h"
#include "externs.h"
#include "function.h"
#include "log.h"
#include "match.h"
#include "mushdb.h"
#include "mymalloc.h"
#include "notify.h"
#include "parse.h"
#include "strutil.h"
#include "charconv.h"
#include "mushsql.h"
#include "charclass.h"
#include "sql.h"

typedef enum {
  SQL_PLATFORM_DISABLED = -1,
  SQL_PLATFORM_MYSQL = 1,
  SQL_PLATFORM_POSTGRESQL,
  SQL_PLATFORM_SQLITE3
} sqlplatform;



/* Number of times to try a connection */
#define SQL_RETRY_TIMES 3

#define sql_test_result(qres)                                                  \
  if (!qres) {                                                                 \
    if (affected_rows >= 0) {                                                  \
    } else if (!sql_connected()) {                                             \
      safe_str(T("#-1 SQL ERROR: NO DATABASE CONNECTED"), buff, bp);           \
    } else {                                                                   \
      safe_format(buff, bp, T("#-1 SQL ERROR: %s"), sql_error());              \
    }                                                                          \
    return;                                                                    \
  }

#ifdef HAVE_MYSQL
extern MYSQL_RES *penn_mysql_sql_query(const char *, int *);
extern void penn_mysql_free_sql_query(MYSQL_RES *qres);
extern int penn_mysql_sql_init(void);
extern void penn_mysql_sql_shutdown(void);
extern int penn_mysql_sql_connected(void);
#endif
#ifdef HAVE_POSTGRESQL
extern PGresult *penn_pg_sql_query(const char *, int *);
extern void penn_pg_free_sql_query(PGresult *qres);
extern int penn_pg_sql_init(void);
extern void penn_pg_sql_shutdown(void);
extern int penn_pg_sql_connected(void);
#endif
#ifdef HAVE_SQLITE3
extern int penn_sqlite3_sql_init(void);
extern void penn_sqlite3_sql_shutdown(void);
extern int penn_sqlite3_sql_connected(void);
extern sqlite3_stmt *penn_sqlite3_sql_query(const char *, int *);
extern void penn_sqlite3_free_sql_query(sqlite3_stmt *);
#endif
extern sqlplatform sql_platform(void);
extern char *sql_sanitize(const char *res);
#define SANITIZE(s, n) ((s && *s) ? mush_strdup(sql_sanitize(s), n) : NULL)
extern void *sql_query(const char *query_str __attribute__((__unused__)),
                       int *affected_rows __attribute__((__unused__)));
extern const char *
sql_error(void);
extern char *do_sql(char *query, char *buff, char **bp);

extern char *SANITIZEUTF8(const char *restrict s, const char *restrict n);

/* A helper function to translate SQL_PLATFORM into one of our
 * supported platform codes. We remember this value, so a reboot
 * is necessary to change it.
 */
extern sqlplatform sql_platform(void);

#endif
