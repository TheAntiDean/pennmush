#ifndef __ODBC_H
#define __ODBC_H

#include "dbdefs.h"
#include "mushtype.h"
#include "privtab.h"
#include "sqltypes.h"
#include "mymalloc.h"
#include "boolexp.h"

#define ODBC_ERROR(res, handle) (ODBC_ProcessError(res, handle))

extern SQLRETURN retcode;
extern SQLHENV henv;
extern SQLHDBC hdbc;

#define ODBC_GET 0
#define ODBC_PUT 1
#define ODBC_DELETE 2
#define ODBC_NODATA 3
#define ODBC_RESULT 4

#define ODBC_CHAR 0
#define ODBC_INT 1
#define ODBC_FLOAT 2
#define ODBC_DOUBLE 3
#define ODBC_DATE 4
#define ODBC_TIME 5
#define ODBC_TIMESTAMP 6

/* ODBC_Field is a structure that holds information about a field in a
 * database table.  It is used to describe the fields in a table, and
 * to hold the data for a field in a row.
 */
typedef struct {
  char *name; /**< The field name */
  SQLLEN length; /**< The length of the field */
  SQLCHAR *sValue;  /**< The value of the field */
  SQLINTEGER iValue; /**< The value of the field */
  SQLFLOAT fValue; /**< The value of the field */ 
  SQLDOUBLE dValue; /**< The value of the field */
  SQL_DATE_STRUCT date; /**< The value of the field */
  SQL_TIME_STRUCT time; /**< The value of the field */
  SQL_TIMESTAMP_STRUCT timestamp; /**< The value of the field */
  int type;         /**< The type of the field */
} ODBC_Field;


/* ODBC_Result is a structure that holds information about a result
 * set from a database query.  It is used to hold the data for a result
 * set.
 */
typedef struct {
  const char *table;   /**< The table name */
  int row_count; 
  SQLCHAR* sort_field; /**< The field to sort by */
  int field_count;      /**< The number of rows in the result set */
  ODBC_Field *fields;  /**< The input data */
  char *where;        /**<  where clause */
  int reverse;       /**<  reverse order */
  int type;            /**< Type of query */
} ODBC_Query;


typedef struct ODBC_Result_t ODBC_Result;
/* ODBC_Result is a structure that holds information about a result
 * set from a database query.  It is used to hold the data for a result
 * set.
 */
struct ODBC_Result_t{
  ODBC_Result *next; /**< The next result in the list */
  ODBC_Result *prev; /**< The previous result in the list */

  int field_count;      /**< The number of fields in the result set */
  ODBC_Field *fields;      /**< The result set */
};



// odbc_free_result(ODBC_Result)
// Free the memory used by an ODBC_Result
extern void ODBC_FreeResult(ODBC_Result *res);
/*----------------------------------------------------------------------------*/
/* ODBC_Put */
/*----------------------------------------------------------------------------*/
extern ODBC_Result *ODBC_ExecuteQuery(ODBC_Query *query);

extern void ODBC_Init(void);

extern void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType,
                                   RETCODE RetCode);

int ODBC_ProcessError(SQLRETURN sqlreturn, SQLHANDLE handle);
extern int ODBC_ExecuteStatement(SQLHSTMT hstmt);
int ODBC_InsertQuery(SQLCHAR *query);

extern int ODBC_MUSH_WriteObject(dbref objID);
extern int
ODBC_MUSH_LoadAllObjects(void);

extern void ODBC_MUSH_WriteObjAttributes(dbref objID);
extern void ODBC_MUSH_WriteObjLocks(dbref objID, lock_list *l);
extern void ODBC_MUSH_WriteChannelLock(char *name, char *type,  boolexp key);
extern void ODBC_MUSH_LoadLock(dbref objID);
extern ODBC_Query *ODBC_NewQuery(const char *table, int field_count, char *where,
                           int type);
extern void ODBC_FreeQuery(ODBC_Query *query);
extern int ODBC_NextResult(ODBC_Result *result);

#endif /* __ODBC_H */
