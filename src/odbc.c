#include "odbc.h"

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <string.h>

#include "options.h"
#include "conf.h"

#include "lock.h"
#include "log.h"
#include "mushdb.h"
#include "strutil.h"
#include "mymalloc.h"

SQLRETURN retcode;
SQLHENV henv;
SQLHDBC hdbc;

// ODBC_FreeQuery
// Free the memory used by an ODBC_Query structure.
//
// Parameters:
//   query - The ODBC_Query structure to free.
//
// Returns:
//   Nothing.
//
void
ODBC_FreeQuery(ODBC_Query *query)
{


  if (query->fields && query->type == ODBC_GET) {
      for(int i = 0; i < query->field_count; i++)
  {

      if (query->fields[i].sValue)
          mush_free(query->fields[i].sValue, "odbc.sValue");
  }
    mush_free(query->fields, "odbc.fields");
  }

  mush_free(query, "odbc.query");
}


// ODBC_NewQuery
// Create a new ODBC_Query structure.
//
// Parameters:
//   table - The table name.
//   field_count - The number of fields in the table.
//
// Returns:
//   A pointer to the new ODBC_Query structure.
//
ODBC_Query *
ODBC_NewQuery(const char *table, int field_count, char *where, int type)
{
  ODBC_Query *query;

  query = mush_malloc(sizeof(ODBC_Query), "odbc.query");
  query->table = table;
  query->field_count = field_count;
  query->type = type;
   query->where = where;
  query->fields = mush_malloc(sizeof(ODBC_Field) * field_count, "odbc.fields");
  if(query->type == ODBC_GET) {
  for(int i = 0; i < field_count; i++) {
    query->fields[i].sValue = mush_malloc(BUFFER_LEN, "odbc.sValue");
  }
}

  return query;
}

// ODBC_FreeResult
// Free the memory used by an ODBC_Result structure.
//
// Parameters:
//   result - The ODBC_Result structure to free.
//
// Returns:
//   Nothing.
//
void
ODBC_FreeResult(ODBC_Result *result)
{
  int i;

  if (result->fields) {
    for (i = 0; i < result->field_count; i++) {
      if (result->fields[i].sValue) {
        mush_free(result->fields[i].sValue, "odbc.sValue");
      }
    }
    mush_free(result->fields, "odbc.fields");
  }

  mush_free(result, "odbc.result");
}
  

// ODBC_NewResult
// Create a new ODBC_Result structure.
//
// Parameters:
//   field_count - The number of fields in the result set.
//
// Returns:
//   A pointer to the new ODBC_Result structure.
//
ODBC_Result *
ODBC_NewResult(int field_count)
{
  ODBC_Result *result;
  int i;

  result = mush_malloc(sizeof(ODBC_Result), "odbc.result");
  result->field_count = field_count;
  result->fields = mush_malloc(sizeof(ODBC_Field) * field_count, "odbc.fields");
  for (i = 0; i < field_count; i++) {
    result->fields[i].name = NULL;
    result->fields[i].sValue = mush_malloc(BUFFER_LEN, "odbc.field.sValue");
    result->fields[i].length = 0;
    result->fields[i].type = ODBC_CHAR;
  }
  return result;
}

// ODBC_ExecuteQuery
// Execute a query against the database.
//
// Parameters:
//   query - The query to execute.
//
// Returns:
//   A pointer to an ODBC_Result structure containing the results of the query.
//
ODBC_Result *
ODBC_ExecuteQuery(ODBC_Query *query)
{
  ODBC_Result *result = NULL;
  SQLHANDLE hstmt = 0;
  char tbuf1[BUFFER_LEN];
  char columns[BUFFER_LEN];
  char values[BUFFER_LEN];
  char valueSub[BUFFER_LEN];
  char *bp;
  char *cp;
  char *vp;
  char *vsp;

  memset(tbuf1, 0, BUFFER_LEN);
  memset(columns, 0, BUFFER_LEN);
  memset(values, 0, BUFFER_LEN);
  memset(valueSub, 0, BUFFER_LEN);


  bp = tbuf1;
  cp = columns;
  vp = values;
  vsp = valueSub;
  *bp = '\0';
  *cp = '\0';
  *vp = '\0';
  *vsp = '\0';

  if (query->type == ODBC_DELETE && query->where != NULL) {
    safe_format(tbuf1, &bp, "DELETE FROM %s WHERE %s", query->table,
             query->where);
  }

  if (query->type == ODBC_PUT) {

    for (int i = 0; i < query->field_count; i++) {
      if (query->where != NULL) {
        safe_format(values, &vp, "%s = ?", query->fields[i].name,
                    query->fields[i].name);
        if (i < query->field_count - 1) {

          safe_chr(',', values, &vp);
        }
      } else {
        safe_format(columns, &cp, "%s", query->fields[i].name);
        safe_format(values, &vp, "%s = VALUES(%s)", query->fields[i].name,
                    query->fields[i].name);
        safe_chr('?', valueSub, &vsp);
        if (i < query->field_count - 1) {
          safe_chr(',', columns, &cp);
          safe_chr(',', values, &vp);
          safe_chr(',', valueSub, &vsp);
        }
      }
    }
    // insert where
    if (query->where != NULL) {
          safe_format(tbuf1, &bp,
               "UPDATE %s SET %s WHERE %s",
               query->table, values, query->where);
    } else {
          safe_format(tbuf1, &bp,
               "INSERT INTO %s (%s) VALUES(%s) ON DUPLICATE KEY UPDATE %s",
               query->table, columns, valueSub, values);
    }

  } else if (query->type == ODBC_GET) {
    for (int i = 0; i < query->field_count; i++) {
      safe_format(columns, &cp, "%s", query->fields[i].name);
      if (i < query->field_count - 1) {
        safe_chr(',', columns, &cp);
      }
    }
     if (query->where) {
      safe_format(tbuf1, &bp, "SELECT %s FROM %s WHERE %s", columns,
               query->table, query->where);
    } else {
      if (query->reverse == 1) {

        safe_format(tbuf1, &bp, "SELECT %s FROM %s ORDER BY %s DESC",
                 columns, query->table, query->sort_field);
      } else {
        safe_format(tbuf1, &bp, "SELECT %s FROM %s", columns, query->table);
      }
    }
  
  }
  
  
  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
    retcode = SQLPrepare(hstmt, (SQLCHAR *) tbuf1, SQL_NTS);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
      if (query->type == ODBC_DELETE && query->where != NULL) {
        retcode = SQLExecute(hstmt);
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
          SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
          return NULL;
        }
      } else if (query->type == ODBC_PUT) {
        for (int i = 0; i < query->field_count; i++) {
          switch (query->fields[i].type) {
          case ODBC_CHAR:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT,
                                       SQL_C_CHAR, SQL_CHAR, 0, 0,
                                       query->fields[i].sValue, 0, NULL);
            break;
          case ODBC_INT:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT,
                                       SQL_C_LONG, SQL_INTEGER, 0, 0,
                                       &query->fields[i].iValue, 0, NULL);
            break;
          case ODBC_FLOAT:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT,
                                       SQL_C_FLOAT, SQL_FLOAT, 0, 0,
                                       &query->fields[i].fValue, 0, NULL);
            break;
          case ODBC_DOUBLE:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT,
                                       SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
                                       &query->fields[i].dValue, 0, NULL);
            break;
          case ODBC_DATE:
            retcode =
              SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_DATE,
                               SQL_DATE, 0, 0, &query->fields[i].date, 0, NULL);
            break;
          case ODBC_TIME:
            retcode =
              SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_TIME,
                               SQL_TIME, 0, 0, &query->fields[i].time, 0, NULL);
            break;
          case ODBC_TIMESTAMP:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT,
                                       SQL_C_TIMESTAMP, SQL_TIMESTAMP, 0, 0,
                                       &query->fields[i].timestamp, 0, NULL);
            break;
          }
        }
        retcode = SQLExecute(hstmt);
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
          SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
          //return NULL;
        }

        ODBC_ERROR(retcode, hstmt);
      } else if (query->type == ODBC_GET) { 

        retcode = SQLExecute(hstmt);

        while (1) {
          ODBC_Result *temp =
            (ODBC_Result *) ODBC_NewResult(query->field_count);

          retcode = SQLFetch(hstmt);
          // ODBC error

          if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

            for (int i = 0; i < query->field_count; i++) {
              switch (query->fields[i].type) {
              case ODBC_CHAR:
                retcode =
                  SQLGetData(hstmt, i + 1, SQL_C_CHAR, temp->fields[i].sValue,
                             BUFFER_LEN, &temp->fields[i].length);
                break;
              case ODBC_INT:
                retcode =
                  SQLGetData(hstmt, i + 1, SQL_C_LONG, &temp->fields[i].iValue,
                             0, &temp->fields[i].length);
                break;
              case ODBC_FLOAT:
                retcode =
                  SQLGetData(hstmt, i + 1, SQL_C_FLOAT, &temp->fields[i].fValue,
                             0, &temp->fields[i].length);
                break;
              case ODBC_DOUBLE:
                retcode = SQLGetData(hstmt, i + 1, SQL_C_DOUBLE,
                                     &temp->fields[i].dValue, 0,
                                     &temp->fields[i].length);
                break;
              case ODBC_DATE:
                retcode =
                  SQLGetData(hstmt, i + 1, SQL_C_DATE, &temp->fields[i].date, 0,
                             &temp->fields[i].length);
                break;
              case ODBC_TIME:
                retcode =
                  SQLGetData(hstmt, i + 1, SQL_C_TIME, &temp->fields[i].time, 0,
                             &temp->fields[i].length);
                break;
              case ODBC_TIMESTAMP:
                retcode = SQLGetData(hstmt, i + 1, SQL_C_TIMESTAMP,
                                     &temp->fields[i].timestamp, 0,
                                     &temp->fields[i].length);
                break;
              }
            }
            temp->next = result;
            result = temp;
          } else if (retcode == SQL_NO_DATA) {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return result;
          } else {
            ODBC_ERROR(retcode, hstmt);
          }
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return result;
      }
      

    } else {
      ODBC_ERROR(retcode, hstmt);
    }
  } else {
    ODBC_ERROR(retcode, hstmt);
  }
  SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  return result;
}





// ODBC_ProcessError
// Process the error and return an integer
//
int
ODBC_ProcessError(SQLRETURN sqlreturn, SQLHANDLE handle)
{
  if (sqlreturn == SQL_SUCCESS || sqlreturn == SQL_SUCCESS_WITH_INFO) {
    HandleDiagnosticRecord(handle, SQL_HANDLE_STMT, sqlreturn);
    return 1;

  } else {
    HandleDiagnosticRecord(handle, SQL_HANDLE_STMT, sqlreturn);
    return 0;
  }
}

// ODBC_Init
// Initialize the ODBC connection
//
void
ODBC_Init(void)
{
  retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

  // Set the ODBC version environment attribute
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
                            (SQLPOINTER *) SQL_OV_ODBC3, 0);

    // Allocate connection handle
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
      retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

      // Set login timeout to 5 seconds
      if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER) 5, 0);

        // Connect to data source
        retcode = SQLDriverConnect(
          hdbc,                             /* Connection handle */
          NULL,                             /* Window handle */
          (SQLCHAR *) options.sql_database, /* Connection string */
          SQL_NTS,              /* This is a null-terminated string */
          NULL,                 /* Output (result) connection string */
          SQL_NTS,              /* This is a null-terminated string */
          0,                    /* Length of output connect string */
          SQL_DRIVER_NOPROMPT); /* Don’t display a prompt window */
        HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, retcode);
        // Allocate statement handle
      }
    }
  }
}

// HandleDiagnosticRecord
// Handle errors
//
void
HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
  SQLSMALLINT iRec = 0;
  SQLINTEGER iError;
  SQLCHAR wszMessage[1000];
  SQLCHAR wszState[SQL_SQLSTATE_SIZE + 1];

  if (RetCode == SQL_INVALID_HANDLE) {
    do_rawlog(LT_TRACE, "Invalid handle!");
    return;
  }

  while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
                       (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
                       (SQLSMALLINT *) NULL) == SQL_SUCCESS) {

    do_rawlog(LT_TRACE, "[%5.5s] %s (%d)", wszState, wszMessage, iError);
  }
}
