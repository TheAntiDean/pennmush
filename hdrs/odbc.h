#ifndef __ODBC_H
#define __ODBC_H

#include "dbdefs.h"
#include "mushtype.h"
#include "privtab.h"
#include "sqltypes.h"

#define ODBC_ERROR(res, handle) (ODBC_Process_Error(res, handle))

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



typedef struct {
  char *name; /**< The field name */
  SQLCHAR *sValue;  /**< The value of the field */
  SQLINTEGER iValue; /**< The value of the field */
  SQLFLOAT fValue; /**< The value of the field */ 
  SQLDOUBLE dValue; /**< The value of the field */
  SQL_DATE_STRUCT date; /**< The value of the field */
  SQL_TIME_STRUCT time; /**< The value of the field */
  SQL_TIMESTAMP_STRUCT timestamp; /**< The value of the field */
  int type;         /**< The type of the field */
} ODBC_Field;

typedef struct {
  int num;          /**< The row number */
  ODBC_Field *fields; /**< The fields in the row */
} ODBC_Row;

/*----------------------------------------------------------------------------*/
/*                                                                      */
typedef struct {
  const char *table;   /**< The table name */
  int row_count; 
  int field_count;      /**< The number of rows in the result set */
  ODBC_Field *fields;  /**< The input data */
  ODBC_Row *rows;      /**< The result set */
  char *where;        /**<  where clause */
  int type;            /**< Type of query */
} ODBC_Query;



/*----------------------------------------------------------------------------*/
/* ODBC_Put */
/*----------------------------------------------------------------------------*/
extern ODBC_Query *ODBC_ExecuteQuery(ODBC_Query *query);

/*----------------------------------------------------------------------------*/
/* ODBC_Get */
/*---------------------------------------------------------------------------
 * -*/
extern ODBC_Query *ODBC_Get(ODBC_Query *query);


extern void ODBC_Init(void);

extern void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType,
                                   RETCODE RetCode);

int ODBC_Process_Error(SQLRETURN sqlreturn, SQLHANDLE handle);
extern int ODBC_ExecuteStatement(SQLHSTMT hstmt);
int ODBC_InsertQuery(SQLCHAR *query);

extern int ODBC_Write_Object(dbref objID, struct object *DBObj);
extern int ODBC_DBRead(void);

extern void ODBC_dump_attrs(dbref objID);
extern void ODBC_dump_locks(dbref objID, lock_list *l);
extern void ODBC_get_locks(dbref objID);

#endif /* __ODBC_H */
