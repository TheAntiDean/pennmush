

#include "odbc.h"

#include <sql.h>
#include <sqlext.h>

#include <string.h>
#include <stdio.h>
#include "options.h"
#include "attrib.h"
#include "conf.h"
#include "dbdefs.h"
#include "extchat.h"
#include "flags.h"
#include "lock.h"
#include "mushdb.h"
#include "privtab.h"
#include "externs.h"
#include "mymalloc.h"

SQLRETURN
ODBCParamString(SQLHSTMT *hstmt, int idx, char *value)
{
  size_t lenValue = strlen(value);
  retcode =
    SQLBindParameter(*hstmt, idx, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255,
                     0, value, sizeof(value), &lenValue);
}

SQLRETURN
ODBCParamInt(SQLHSTMT *hstmt, int idx, int value)
{
  retcode = SQLBindParameter(*hstmt, idx, SQL_PARAM_INPUT, SQL_C_SLONG,
                             SQL_INTEGER, 11, 0, &value, sizeof(&value), NULL);
}

struct object *
ODBC_Get_Object(dbref objID)
{

 SQLHSTMT hstmt = 0;
 struct object *DBObj = db + objID;

 // Allocate environment handle
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
       SQLDriverConnect(
         hdbc,    /* Connection handle */
         NULL,    /* Window handle */
         NULL,    /* Connection string */
         SQL_NTS, /* This is a null-terminated string */
         (SQLCHAR *)
           options.sql_database, /* Output (result) connection string */
         SQL_NTS,                /* This is a null-terminated string */
         0,                      /* Length of output connect string */
         SQL_DRIVER_NOPROMPT);   /* Don’t display a prompt window */

       // Allocate statement handle
       if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
         retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

         retcode = SQLPrepare(
           hstmt,
           (SQLCHAR
              *) L"select id, name, locationObjId, contentObjId, exitObjId,\
              nextObjId, parentObjId,\
               type, powers, warnings, flags, created, modified, pennies from\
               object where id=?",
           SQL_NTS);
         if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
           SQLBindParameter(hstmt,           /* Statement handle */
                            objID,           /* Column number 1 */
                            SQL_PARAM_INPUT, /* This is an input parameter */
                            SQL_C_LONG,      /* This is an integer in C */
                            SQL_INTEGER,   /* Destination column is varchar */
                            11, /* Length of the parameter */ 
                            0, /* Noscale specifier */ 
                            &objID,         /* The data itself */ 
                            0,             /* Maximum length(default 0) */
                            NULL);      /* Null-terminated string */


           retcode = SQLExecute(hstmt);
           // Fetch and print each row of data. On an error, display a
           // message and exit.
           for (int i = 0;; i++) {
            
            retcode = SQLFetch(hstmt);
             if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
               // show_error();
               HandleDiagnosticRecord(henv, SQL_HANDLE_STMT, retcode);
             if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
             {
               // replace wprintf with printf
               //%S with %ls
               // warning C4477: 'wprintf' : format string '%S' requires an
               // argument of type 'char *' but variadic argument 2 has type
               // 'SQLWCHAR *' wprintf(L"%d: %S %S %S\n", i + 1, sCustID,
               // szName, szPhone);

             } else
               break;
           }
         }

         // Process data
         if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
           SQLCancel(hstmt);
           SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
         }

         SQLDisconnect(hdbc);
       }

       SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
     }
   }
   SQLFreeHandle(SQL_HANDLE_ENV, henv);
 }

 return 1;
}

int
ODBC_Set_Object(dbref objID, struct object *DBObj)
{
  // ODBC_Init();
  SQLCHAR buff[] =
    "INSERT INTO object (id, name, locationObjId, contentObjId, exitObjId, nextObjId, parentObjId, zoneObjId, type, powers, warnings,\
flags, created, modified, pennies) VALUES (?, ?, ?, ?,?, ?, ?,?,?,?,?,?,?,?,?) \
ON DUPLICATE KEY UPDATE name=Values(name), locationObjId=Values(locationObjId), contentObjId=Values(contentObjId), exitObjId=Values(exitObjId), nextObjId=Values(nextObjId), \
parentObjId=Values(parentObjId), zoneObjId=Values(zoneObjId), type=Values(type), powers=Values(powers), warnings=Values(warnings), flags=Values(flags), created=Values(created), \
modified=Values(modified), pennies=Values(pennies)";
  SQLHSTMT hstmt = 0;
  SQLHDESC hIpd = NULL;

  SQLINTEGER id, location, content, exit, next, parent, zone, type, created,
    modified, pennies;

  id = objID;
  location = DBObj->location;
  content = DBObj->contents;
  exit = DBObj->exits;
  next = DBObj->next;
  parent = DBObj->parent;
  zone = DBObj->zone;
  type = Typeof(objID);
  created = DBObj->creation_time;
  modified = DBObj->modification_time;
  pennies = DBObj->penn;

  SQLCHAR *name = strdup(Name(objID));
  SQLCHAR *flags = strdup(bits_to_string("FLAG", DBObj->flags, GOD, NOTHING));
  SQLCHAR *powers = strdup(bits_to_string("POWER", DBObj->powers, GOD, NOTHING));
  SQLCHAR *warnings = strdup(unparse_warnings(DBObj->warnings));
  SQLLEN lenName = strlen(name), lenFlags = strlen(flags),
      lenPowers = strlen(powers), lenWarnings = strlen(warnings);

  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

  ODBC_ERROR(retcode, hstmt);
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
    SQLPrepareA(hstmt, buff, strlen(buff));
    SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &id, sizeof(id), NULL);
    SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0,
                     name, sizeof(name), &lenName);
    SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &location, sizeof(location), NULL);
    SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &content, sizeof(content), NULL);
    SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &exit, sizeof(exit), NULL);
    SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &next, sizeof(next), NULL);
    SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &parent, sizeof(parent), NULL);
    SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &zone, sizeof(zone), NULL);
    SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11, 0,
                     &type, sizeof(type), NULL);
    SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255,
                     0, powers, sizeof(powers), &lenPowers);
    SQLBindParameter(hstmt, 11, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255,
                     0, warnings, sizeof(warnings), &lenWarnings);
    SQLBindParameter(hstmt, 12, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255,
                     0, flags, sizeof(flags), &lenFlags);
    SQLBindParameter(hstmt, 13, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11,
                     0, &created, sizeof(created), NULL);
    SQLBindParameter(hstmt, 14, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11,
                     0, &modified, sizeof(modified), NULL);
    SQLBindParameter(hstmt, 15, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11,
                     0, &pennies, sizeof(pennies), NULL);

    ODBC_ERROR(retcode, hstmt);


    ODBC_ExecuteStatement(hstmt);

    ODBC_dump_locks(objID, DBObj->locks);
    ODBC_dump_attrs(objID);
  }
}

void
ODBC_dump_attrs(dbref objID)
{

  ALIST *list;
  SQLHSTMT hstmt = 0;
  SQLCHAR *name;
  SQLINTEGER *creator;
  SQLCHAR *flags;
  SQLINTEGER *derefs;
  SQLCHAR *attrval;
  SQLLEN lenName = 0, lenFlags = 0, lenAttrval = 0;
  SQLCHAR *buff = "INSERT INTO objectattrib (name, ownerId, flags, \
      derefs, objectId, value) VALUES ( ?, ? , ?, ? , ?, ?) \
ON DUPLICATE KEY UPDATE ownerId=?, flags=?, derefs=?, value=?";



    ATTR_FOR_EACH (objID, list) {
      name = AL_NAME(list);
      lenName = strlen(name);
      creator = Owner(AL_CREATOR(list));
      flags = atrflag_to_string(AL_FLAGS(list));
      lenFlags = strlen(flags);
      derefs = AL_DEREFS(list);
      attrval = atr_value(list);
      lenAttrval = strlen(attrval);
      retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

      retcode = SQLPrepare(hstmt, buff, strlen(buff));

      retcode =
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         lenName, 0, name, sizeof(name), &lenName);
      ODBC_ERROR(retcode, hstmt);

      retcode =
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         11, 0, &creator, sizeof(creator), NULL);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         lenFlags, 0, flags, sizeof(flags), &lenFlags);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         11, 0, &derefs, sizeof(derefs), NULL);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         11, 0, &objID, sizeof(objID), NULL);
      ODBC_ERROR(retcode, hstmt);

      retcode =
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         lenAttrval, 0, attrval, sizeof(attrval), &lenAttrval);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         11, 0, &creator, sizeof(creator), NULL);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         BUFFER_LEN, 0, flags, sizeof(flags), &lenFlags);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         11, 0, &derefs, sizeof(derefs), NULL);
      ODBC_ERROR(retcode, hstmt);
      retcode =
        SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                         BUFFER_LEN, 0, attrval, sizeof(attrval), &lenAttrval);
      ODBC_ERROR(retcode, hstmt);
      ODBC_ExecuteStatement(hstmt);

    }
  }
}

void
ODBC_dump_locks(dbref objID, lock_list *l)
{
  char buff[BUFFER_LEN];
  memset(buff, 0, sizeof(buff));
  lock_list *ll;

  for (ll = l; ll; ll = ll->next) {

    snprintf(
      buff, BUFFER_LEN,
      "INSERT INTO objectlock (type,creatorId,flags,derefs,objectId, boolexp) VALUES (\"%s\",%i, \"%s\",%i, %i, \"%s\") \
  ON DUPLICATE KEY UPDATE creatorId=%i, flags=\"%s\", derefs=%i, boolexp=\"%s\"",
      ll->type, L_CREATOR(ll), lock_flags_long(ll), chunk_derefs(L_KEY(ll)),
      objID, unparse_boolexp(GOD, ll->key, UB_DBREF), L_CREATOR(ll),
      lock_flags_long(ll), chunk_derefs(L_KEY(ll)),
      unparse_boolexp(GOD, ll->key, UB_DBREF));
    ODBC_InsertQuery(buff);
    memset(buff, 0, sizeof(buff));
  }
}
