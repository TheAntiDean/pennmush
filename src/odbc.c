#include "odbc.h"

#include <sql.h>
#include <sqlext.h>
#include <string.h>

#include "options.h"
#include "conf.h"

#include "lock.h"
#include "log.h"
#include "mushdb.h"



SQLRETURN retcode;
SQLHENV henv;
SQLHDBC hdbc;





int
ODBC_InsertQuery(SQLCHAR *query)
{
  SQLHSTMT hstmt = 0;


  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  HandleDiagnosticRecord(hdbc, SQL_HANDLE_STMT, retcode);
  retcode = SQLPrepare(hstmt, (SQLCHAR *) query, SQL_NTS);
  return ODBC_ExecuteStatement(hstmt);
}

int
ODBC_ExecuteStatement(SQLHSTMT hstmt)
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
          hdbc,                 /* Connection handle */
          NULL,                 /* Window handle */
          (SQLCHAR*)options.sql_database, /* Connection string */
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
    do_rawlog(LT_TRACE,"Invalid handle!" );
    return;
  }

  while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
                       (SQLSMALLINT) (sizeof(wszMessage) / sizeof(WCHAR)),
                       (SQLSMALLINT *) NULL) == SQL_SUCCESS) {

    do_rawlog(LT_TRACE, "[%5.5s] %s (%d)", wszState, wszMessage, iError);
  }
}



