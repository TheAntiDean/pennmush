#ifndef __ODBC_H
#define __ODBC_H

#include "dbdefs.h"
#include "mushtype.h"
#include "sqltypes.h"

#define ODBC_ERROR(res, handle) (ODBC_Process_Error(res, handle))

extern SQLRETURN retcode;
extern SQLHENV henv;
extern SQLHDBC hdbc;

extern void ODBC_Init(void);

extern void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType,
                                   RETCODE RetCode);

int ODBC_Process_Error(SQLRETURN sqlreturn, SQLHANDLE handle);
extern int
ODBC_ExecuteStatement(SQLHSTMT hstmt);
int ODBC_InsertQuery(SQLCHAR *query);

extern int
ODBC_Set_Object(dbref objID, struct object *DBObj);
extern void
ODBC_Get_Object(dbref objID, struct object *DBObj);

extern void
ODBC_dump_attrs(dbref objID);
extern void ODBC_dump_locks(dbref objID, lock_list *l);
extern void
ODBC_get_locks(dbref objID);

#endif /* __ODBC_H */
