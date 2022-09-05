

#include "boolexp.h"
#include "log.h"
#include "mushtype.h"
#include "odbc.h"

#include <sql.h>
#include <sqlext.h>

#include <sqltypes.h>
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
#include "sqlite3.h"
#include "sqlite3ext.h"
#include "mushsql.h"

extern PRIV lock_privs[];
extern PRIV attr_privs_view[];

void
ODBC_get_attribs(dbref objID)
{
  SQLHSTMT hstmt = 0;

  // Allocate environment handle
  char query[BUFFER_LEN];
  snprintf(query, BUFFER_LEN,
           "SELECT name, ownerId, flags, derefs, value FROM objectattrib where "
           "objectid=%i",
           objID);
  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

  SQLExecDirect(hstmt, (SQLCHAR *) query, SQL_NTS);

  while (1) {
    retcode = SQLFetch(hstmt);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

      const char flags[BUFFER_LEN], name[BUFFER_LEN],
        value[BUFFER_LEN];
      SQLINTEGER owner, derefs;
      SQLLEN n;

      retcode =
        SQLGetData(hstmt, 1, SQL_C_CHAR, (SQLCHAR *) name, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 2, SQL_C_ULONG, &owner, 0, &n);
      retcode =
        SQLGetData(hstmt, 3, SQL_C_CHAR, (SQLCHAR *) flags, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 4, SQL_C_ULONG, &derefs, 0, &n);
      retcode =
        SQLGetData(hstmt, 5, SQL_C_CHAR, (SQLCHAR *) value, BUFFER_LEN, &n);

      atr_new_add(objID, name, value, owner,
                  string_to_privs(attr_privs_view, flags, 0), derefs, 1);

    } else if (SQL_NO_DATA == retcode)
      break;
    else {
      do_rawlog(LT_TRACE, "%s\n", "fail to fetch data");
      break;
    }
  }
  for (int i = 0;; i++) {
    if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
      // show_error();
      do_rawlog(LT_TRACE, "getattribs");
      HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
    } else {
      break;
    }
  }
  // Process data
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  }
  SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void
ODBC_get_locks(dbref objID)
{
  SQLHSTMT hstmt = 0;
  char query[BUFFER_LEN];
  snprintf(query, BUFFER_LEN,
           "SELECT type, creatorId, flags, derefs, value FROM "
           "objectlock where objectId=%i",
           objID);
  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  SQLExecDirect(hstmt, (SQLCHAR *) query, SQL_NTS);

  while (1) {
    retcode = SQLFetch(hstmt);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

      const char type[BUFFER_LEN], flags[BUFFER_LEN], bexp[BUFFER_LEN];
      SQLINTEGER creator, derefs;
      boolexp b;
      SQLLEN n;

      retcode =
        SQLGetData(hstmt, 1, SQL_C_CHAR, (SQLCHAR *) type, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 2, SQL_C_ULONG, &creator, 0, &n);
      retcode =
        SQLGetData(hstmt, 3, SQL_C_CHAR, (SQLCHAR *) flags, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 4, SQL_C_ULONG, &derefs, 0, &n);
      retcode =
        SQLGetData(hstmt, 5, SQL_C_CHAR, (SQLCHAR *) bexp, BUFFER_LEN, &n);

      b = parse_boolexp(creator, strdup(bexp), type);
      add_lock_raw(creator, objID, type, b,
                   string_to_privs(lock_privs, strdup(flags), 0));

    } else if (SQL_NO_DATA == retcode)
      break;
    else {
      
      break;
    }
    for (int i = 0;; i++) {
      if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
        // show_error();
        
        HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
      } else {
        break;
      }
    }
  }
  // Process data
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  }
}

int
ODBC_DBRead(void)
{
  sqlite3 *sqldb;
  sqldb = get_shared_db();
  sqlite3_stmt *adder;
  struct object *DBObj;
  SQLHSTMT hstmt = 0;
  retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
  HandleDiagnosticRecord(hdbc, SQL_HANDLE_STMT, retcode);
  int dbsize = 0, status;
  retcode = SQLExecDirect(
    hstmt,
    (SQLCHAR *) "SELECT id, name, locationObjId, contentObjId, exitObjId, "
                "nextObjId, parentObjId, zoneObjId, type, powers, warnings, "
                "flags, created, modified, pennies, ownerObjId FROM object ORDER BY id",
    SQL_NTS);
  HandleDiagnosticRecord(henv, SQL_HANDLE_STMT, retcode);
  do_rawlog(LT_TRACE, "150");
  sqlite3_exec(sqldb, "BEGIN TRANSACTION", NULL, NULL, NULL);
  adder = prepare_statement(sqldb, "INSERT INTO objects(dbref) VALUES (?)",
                                "objects.add");

  while (1) {

    retcode = SQLFetch(hstmt);
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {


      const char name[BUFFER_LEN], powers[BUFFER_LEN], warnings[BUFFER_LEN],
        flags[BUFFER_LEN];
      SQLLEN n;
      SQLINTEGER newDBRef, location, content, exit, next, parent, zone, type,
        created, modified, pennies, owner;

      retcode = SQLGetData(hstmt, 1, SQL_C_ULONG, &newDBRef, 0, &n);
      retcode = SQLGetData(hstmt, 2, SQL_C_CHAR, (SQLCHAR*) &name, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 3, SQL_C_ULONG, &location, 0, &n);
      retcode = SQLGetData(hstmt, 4, SQL_C_ULONG, &content, 0, &n);
      retcode = SQLGetData(hstmt, 5, SQL_C_ULONG, &exit, 0, &n);
      retcode = SQLGetData(hstmt, 6, SQL_C_ULONG, &next, 0, &n);
      retcode = SQLGetData(hstmt, 7, SQL_C_ULONG, &parent, 0, &n);
      retcode = SQLGetData(hstmt, 8, SQL_C_ULONG, &zone, 0, &n);
      retcode = SQLGetData(hstmt, 9, SQL_C_ULONG, &type, 0, &n);
      retcode = SQLGetData(hstmt, 10, SQL_C_CHAR, (SQLCHAR*)powers, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 11, SQL_C_CHAR, (SQLCHAR*)warnings, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 12, SQL_C_CHAR, (SQLCHAR*)flags, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 13, SQL_C_ULONG, &created, 0, &n);
      retcode = SQLGetData(hstmt, 14, SQL_C_ULONG, &modified, 0, &n);
      retcode = SQLGetData(hstmt, 15, SQL_C_ULONG, &pennies, 0, &n);
      retcode = SQLGetData(hstmt, 16, SQL_C_ULONG, &owner, 0, &n);

      // Grow the database for the new object
      db_grow(dbsize + 1);
      // Pull a DB Object out of the database
      DBObj = db + newDBRef;
      // increment for next run
      dbsize++;
      // set object values

      set_name(newDBRef,&name);
      DBObj->location = location;
      DBObj->contents = content;
      DBObj->exits = exit;
      DBObj->next = next;
      DBObj->parent = parent;
      DBObj->zone = zone;
      DBObj->type = type;
      DBObj->powers = string_to_bits("POWER", powers);
      DBObj->warnings = parse_warnings(NOTHING, warnings);
      DBObj->flags = string_to_bits("FLAG", flags);
      DBObj->creation_time = created;
      DBObj->modification_time = modified;
      DBObj->penn = pennies;
      DBObj->owner = owner;

      ODBC_get_locks(newDBRef);
      ODBC_get_attribs(newDBRef);



      switch (Typeof(newDBRef)) {
      case TYPE_PLAYER:
        // add_player(newDBRef);
        current_state.players++;
        current_state.garbage--;
        break;
      case TYPE_THING:
        current_state.things++;
        current_state.garbage--;
        break;
      case TYPE_EXIT:
        current_state.exits++;
        current_state.garbage--;
        break;
      case TYPE_ROOM:
        current_state.rooms++;
        current_state.garbage--;
        break;
      }
        if (globals.new_indb_version < 2) {
          add_new_attr("MONIKER",
                       AF_WIZARD | AF_NOPROG | AF_VISUAL | AF_LOCKED);
        }

        sqlite3_bind_int(adder, 1, newDBRef);
        do {
          status = sqlite3_step(adder);
        } while (is_busy_status(status));
        if (status != SQLITE_DONE) {
          do_rawlog(LT_ERR, "Unable to add #%d to objects table: %s", newDBRef,
                    sqlite3_errstr(status));
        }
        sqlite3_reset(adder);

        if (IsPlayer(newDBRef)) {
          add_player(newDBRef);
          clear_flag_internal(newDBRef, "CONNECTED");
          /* If it has the MONITOR flag and the db predates HEAR_CONNECT, swap
           * them over */
          if (!(globals.indb_flags & DBF_HEAR_CONNECT) &&
              has_flag_by_name(newDBRef, "MONITOR", NOTYPE)) {
            clear_flag_internal(newDBRef, "MONITOR");
            set_flag_internal(newDBRef, "HEAR_CONNECT");
          }
        }


    } else if (SQL_NO_DATA == retcode) {
      
      break;
    } else {
      HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
      HandleDiagnosticRecord(henv, SQL_HANDLE_DBC, retcode);
      
      break;
    }



  }

  return db_top;

  for (int i = 0;; i++) {
    if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
      // show_error();
      do_rawlog(LT_TRACE, "getattribs");
      HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
      return -1;
    } else {
      break;
    }
  }

  // Process data
  if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ||
      retcode == SQL_NO_DATA) {
    sqlite3_exec(sqldb, "COMMIT TRANSACTION", NULL, NULL, NULL);
    fix_free_list();
    dbck();
    SQLCancel(hstmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return db_top;
  }
}

int
ODBC_Write_Object(dbref objID, struct object *DBObj)
{
  ODBC_Query q;
  q.type = ODBC_PUT;
  q.table = "object";
  q.field_count = 16;
  ODBC_Field *fields = malloc(sizeof(ODBC_Field) * q.field_count);
  q.fields = fields;
  q.fields[0].name = "id";
  q.fields[0].type = ODBC_INT;
  q.fields[0].iValue = objID;
  q.fields[1].name = "name";
  q.fields[1].type = ODBC_CHAR;
  q.fields[1].sValue = (SQLCHAR*)Name(objID);
  q.fields[2].name = "locationObjId";
  q.fields[2].type = ODBC_INT;
  q.fields[2].iValue = DBObj->location;
  q.fields[3].name = "contentObjId";
  q.fields[3].type = ODBC_INT;
  q.fields[3].iValue = DBObj->contents;
  q.fields[4].name = "exitObjId";
  q.fields[4].type = ODBC_INT;
  q.fields[4].iValue = DBObj->exits;
  q.fields[5].name = "nextObjId";
  q.fields[5].type = ODBC_INT;
  q.fields[5].iValue = DBObj->next;
  q.fields[6].name = "parentObjId";
  q.fields[6].type = ODBC_INT;
  q.fields[6].iValue = DBObj->parent;
  q.fields[7].name = "zoneObjId";
  q.fields[7].type = ODBC_INT;
  q.fields[7].iValue = DBObj->zone;
  q.fields[8].name = "type";
  q.fields[8].type = ODBC_INT;
  q.fields[8].iValue = Typeof(objID);
  q.fields[9].name = "powers";
  q.fields[9].type = ODBC_CHAR;
  q.fields[9].sValue = (SQLCHAR*)bits_to_string("POWER", DBObj->powers, GOD, NOTHING);
  q.fields[10].name = "warnings";
  q.fields[10].type = ODBC_CHAR;
  q.fields[10].sValue = (SQLCHAR*)unparse_warnings(DBObj->warnings);
  q.fields[11].name = "flags";
  q.fields[11].type = ODBC_CHAR;
  q.fields[11].sValue = (SQLCHAR*)bits_to_string("FLAG", DBObj->flags, GOD, NOTHING);
  q.fields[12].name = "created";
  q.fields[12].type = ODBC_INT;
  q.fields[12].iValue = DBObj->creation_time;
  q.fields[13].name = "modified";
  q.fields[13].type = ODBC_INT;
  q.fields[13].iValue = DBObj->modification_time;
  q.fields[14].name = "pennies";
  q.fields[14].type = ODBC_INT;
  q.fields[14].iValue = DBObj->penn;
  q.fields[15].name = "ownerObjId";
  q.fields[15].type = ODBC_INT;
  q.fields[15].iValue = DBObj->owner;
  ODBC_ExecuteQuery(&q);
  free(q.fields);
  return 1;
}

void
ODBC_dump_attrs(dbref objID)
{
  char buff[BUFFER_LEN];
  ALIST *list;
  ODBC_Query da;
  da.type = ODBC_DELETE;
  snprintf(buff, BUFFER_LEN, "objectId=%d", objID);
  da.where = buff;
  da.table = "objectattrib";
  ODBC_ExecuteQuery(&da);


  ATTR_FOR_EACH (objID, list) {
    ODBC_Query q;
    q.type = ODBC_PUT;
    q.table = "objectattrib";
    q.field_count = 6;
    ODBC_Field *fields = malloc(sizeof(ODBC_Field) * 6);
    fields[0].name = "name";
    fields[0].type = ODBC_CHAR;
    fields[0].sValue = (SQLCHAR*)AL_NAME(list);
    fields[1].name = "ownerId";
    fields[1].type = ODBC_INT;
    fields[1].iValue = Owner(AL_CREATOR(list));
    fields[2].name = "flags";
    fields[2].type = ODBC_CHAR;
    fields[2].sValue = (SQLCHAR*)atrflag_to_string(AL_FLAGS(list));
    fields[3].name = "derefs";
    fields[3].type = ODBC_INT;
    fields[3].iValue = AL_DEREFS(list);
    fields[4].name = "objectId";
    fields[4].type = ODBC_INT;
    fields[4].iValue = objID;
    fields[5].name = "value";
    fields[5].type = ODBC_CHAR;
    fields[5].sValue = (SQLCHAR*)atr_value(list);
    q.fields = fields;
    ODBC_ExecuteQuery(&q);

    free(fields);
  }
}

void
ODBC_dump_locks(dbref objID, lock_list *l)
{
  char buff[BUFFER_LEN];
  ODBC_Query dl;
  memset(buff, 0, sizeof(buff));
  lock_list *ll;
  dl.type = ODBC_DELETE;
  dl.table = "objectlock";
  dl.field_count = 0;
  snprintf(buff, BUFFER_LEN, "objectId=%d", objID);
  dl.where = buff;
  ODBC_ExecuteQuery(&dl);  

  
  for (ll = l; ll; ll = ll->next) {
    ODBC_Query q;
    q.type = ODBC_PUT;
    q.field_count = 6;
    ODBC_Field *fields = malloc(sizeof(ODBC_Field) * 6);

    fields[0].name = "type";
    fields[0].type = ODBC_CHAR;
    fields[0].sValue = (SQLCHAR*)ll->type;
    fields[1].name = "creatorId";
    fields[1].type = ODBC_INT;
    fields[1].iValue = Owner(ll->creator);
    fields[2].name = "flags";
    fields[2].type = ODBC_CHAR;
    fields[2].sValue = (SQLCHAR*)lock_flags_long(ll);
    fields[3].name = "derefs";
    fields[3].type = ODBC_INT;
    fields[3].iValue = chunk_derefs(L_KEY(ll));
    fields[4].name = "objectId";
    fields[4].type = ODBC_INT;
    fields[4].iValue = objID;
    fields[5].name = "boolexp";
    fields[5].type = ODBC_CHAR;
    fields[5].sValue = (SQLCHAR*)unparse_boolexp(GOD, ll->key, UB_DBREF);
    q.fields = fields;
    q.table = "objectlock";

    ODBC_ExecuteQuery(&q);
    free(fields);   
  }
}
