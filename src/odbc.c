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

SQLRETURN retcode;
SQLHENV henv;
SQLHDBC hdbc;

ODBC_Query *
ODBC_ExecuteQuery(ODBC_Query *query)
{
  ODBC_Query *ret;
  ODBC_Row *rows = NULL;
  SQLHANDLE hstmt = 0;
  char tbuf1[BUFFER_LEN], columns[BUFFER_LEN], values[BUFFER_LEN], valueSub[BUFFER_LEN], *cp, *vp, *vsp;

  memset(tbuf1, 0, BUFFER_LEN);
  
  memset(columns, 0, BUFFER_LEN);
  cp= columns;
  vp = values;
  vsp = valueSub;

  if(query->type == ODBC_DELETE && query->where != NULL)
  {
    snprintf(tbuf1, BUFFER_LEN, "DELETE FROM %s WHERE %s", query->table, query->where);
  }
  


  if (query->type == ODBC_PUT) {
    
    for (int i = 0; i < query->field_count; i++) {
      safe_format(columns, &cp, "%s", query->fields[i].name);
      safe_format(values, &vp, "%s = VALUES(%s)", query->fields[i].name, query->fields[i].name);
      safe_chr('?', valueSub, &vsp);
      if (i < query->field_count - 1) {
        safe_chr(',', columns, &cp);
        safe_chr(',', values, &vp);
        safe_chr(',', valueSub, &vsp);
      }

      snprintf(tbuf1, BUFFER_LEN, "INSERT INTO %s (%s) VALUES(%s) ON DUPLICATE KEY UPDATE %s", query->table, columns, valueSub,values);
      
    }


  } else if (query->type == ODBC_GET) {
    for (int i; i < query->field_count; i++) {
      safe_format(columns, &cp , "%s", query->fields[i].name);
      if (i < query->field_count - 1) {
        safe_chr(',', columns, &cp);
      }
    }
    if (query->where) {
      snprintf(tbuf1, BUFFER_LEN, "SELECT %s FROM %s WHERE %s", columns, tbuf1, query->where);
    } else
    {
      snprintf(tbuf1, BUFFER_LEN, "SELECT %s FROM %s", columns, query->table);
    }
  }
  // if (query->clause) {
  //   snprintf(tbuf1, BUFFER_LEN, "%s WHERE %s", tbuf1, query->clause);
  // }
  do_rawlog(LT_TRACE, "ODBC: %s", tbuf1);
  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
    retcode = SQLPrepare(hstmt, (SQLCHAR *) tbuf1, SQL_NTS);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
      if(query->type == ODBC_DELETE && query->where != NULL)
      {
        retcode = SQLExecute(hstmt);
        if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
          ret = query;
          ret->rows = rows;
          return ret;
        }
      }
      if (query->type == ODBC_PUT) {
        for (int i = 0; i < query->field_count; i++) {
          switch (query->fields[i].type) {
          case ODBC_CHAR:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, query->fields[i].sValue, 0, NULL);
            break;
          case ODBC_INT:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &query->fields[i].iValue, 0, NULL);
            break;
          case ODBC_FLOAT:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &query->fields[i].fValue, 0, NULL);
            break;
          case ODBC_DOUBLE:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &query->fields[i].dValue, 0, NULL);
            break;
          case ODBC_DATE:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_DATE, SQL_DATE, 0, 0, &query->fields[i].date, 0, NULL);
            break;
          case ODBC_TIME:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_TIME, SQL_TIME, 0, 0, &query->fields[i].time, 0, NULL);
            break;
          case ODBC_TIMESTAMP:
            retcode = SQLBindParameter(hstmt, i + 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, 0, 0, &query->fields[i].timestamp, 0, NULL);
            break;
          }
          retcode = SQLExecute(hstmt);
        }

        ODBC_ERROR(retcode, hstmt);
      } else {
          SQLLEN row_count;
          SQLRowCount(hstmt, &row_count);
          if (row_count == 0) {
            ret->type = ODBC_NODATA;
          } else {
            rows = malloc(sizeof(ODBC_Row) * row_count);
            ret->row_count = row_count;
            ret->type = ODBC_RESULT;
          } 
          while(1) {
            retcode = SQLFetch(hstmt);
            ODBC_Field *field;
            field = (ODBC_Field *) malloc(sizeof(ODBC_Field) * query->field_count);
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
              for (int i = 0; i < query->field_count; i++) {
                switch (query->fields[i].type) {
                  case ODBC_CHAR:
                    ret->fields[i].sValue = (SQLCHAR *) malloc(sizeof(SQLCHAR) * BUFFER_LEN);
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_CHAR, field->sValue, BUFFER_LEN, NULL);
                    break;
                  case ODBC_INT:
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_LONG, &field->iValue, 0, NULL);
                    break;
                  case ODBC_FLOAT:
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_FLOAT, &field->fValue, 0, NULL);
                    break;
                  case ODBC_DOUBLE:
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_DOUBLE, &field->dValue, 0, NULL);
                    break;
                  case ODBC_DATE:
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_DATE, &field->date, 0, NULL);
                    break;
                  case ODBC_TIME:
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_TIME, &field->time, 0, NULL);
                    break;
                  case ODBC_TIMESTAMP:
                    retcode = SQLGetData(hstmt, i + 1, SQL_C_TIMESTAMP, &field->timestamp, 0, NULL);
                    break;
                }
                rows[i].fields = field;
              }
            } else {
              break;
            }
            ret->rows = rows; 
          }
  
      }

      SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
      return ret;
    } else {
      ODBC_ERROR(retcode, hstmt);
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
      return NULL;
    }
  } else {
    ODBC_ERROR(retcode, hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return NULL;
  }

  return ret;
}

int ODBC_ExecuteStatement(SQLHSTMT hstmt)
{
  retcode = SQLExecute(hstmt);

  HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

    //
    // Bind columns 1, 2, and 3
    // retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, objID, 11, &objID);

    // Fetch and print each row of data. On an error, display a message
    // and exit.

     // if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
        HandleDiagnosticRecord(henv, SQL_HANDLE_DBC, retcode);
      if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

      }
  }
      SQLCancel(hstmt);
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

  return 0;
}

int
ODBC_Process_Error(SQLRETURN sqlreturn, SQLHANDLE handle)
{
  if (sqlreturn == SQL_SUCCESS || sqlreturn == SQL_SUCCESS_WITH_INFO) {
    HandleDiagnosticRecord(handle, SQL_HANDLE_STMT, sqlreturn);
    return 1;

  } else {
    HandleDiagnosticRecord(handle, SQL_HANDLE_STMT, sqlreturn);
    return 0;
  }
}

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
