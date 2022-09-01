

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

      const char type[BUFFER_LEN], flags[BUFFER_LEN], name[BUFFER_LEN],
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
      do_rawlog(LT_TRACE, "%s\n", "fail to fetch data");
      break;
    }
    for (int i = 0;; i++) {
      if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
        // show_error();
        do_rawlog(LT_TRACE, "getlocks");
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
ODBC_Get_Object(dbref objID)
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


      const char name[BUFFER_LEN], *powers= malloc(BUFFER_LEN), *warnings= malloc(BUFFER_LEN),
        *flags= malloc(BUFFER_LEN);
      SQLLEN n;
      SQLINTEGER newDBRef, location, content, exit, next, parent, zone, type,
        created, modified, pennies, owner;

      retcode = SQLGetData(hstmt, 1, SQL_C_ULONG, &newDBRef, 0, &n);
      retcode = SQLGetData(hstmt, 2, SQL_C_CHAR, name, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 3, SQL_C_ULONG, &location, 0, &n);
      retcode = SQLGetData(hstmt, 4, SQL_C_ULONG, &content, 0, &n);
      retcode = SQLGetData(hstmt, 5, SQL_C_ULONG, &exit, 0, &n);
      retcode = SQLGetData(hstmt, 6, SQL_C_ULONG, &next, 0, &n);
      retcode = SQLGetData(hstmt, 7, SQL_C_ULONG, &parent, 0, &n);
      retcode = SQLGetData(hstmt, 8, SQL_C_ULONG, &zone, 0, &n);
      retcode = SQLGetData(hstmt, 9, SQL_C_ULONG, &type, 0, &n);
      retcode = SQLGetData(hstmt, 10, SQL_C_CHAR, powers, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 11, SQL_C_CHAR, warnings, BUFFER_LEN, &n);
      retcode = SQLGetData(hstmt, 12, SQL_C_CHAR, flags, BUFFER_LEN, &n);
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

      DBObj->name = strdup(name);
      set_name(newDBRef, DBObj->name);
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
      do_rawlog(LT_TRACE, "Fetch");
      break;
    } else {
      HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
      HandleDiagnosticRecord(henv, SQL_HANDLE_DBC, retcode);
      do_rawlog(LT_TRACE, "%s\n", "fail to fetch data");
      break;
    }



  }

  return db_top;

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
ODBC_Set_Object(dbref objID, struct object *DBObj)
{
  // ODBC_Init();
  SQLCHAR buff[] =
    "INSERT INTO object (id, name, locationObjId, contentObjId, exitObjId, nextObjId, parentObjId, zoneObjId, type, powers, warnings,\
flags, created, modified, pennies, ownerObjId) VALUES (?, ?, ?, ?,?, ?, ?,?,?,?,?,?,?,?,?,?) \
ON DUPLICATE KEY UPDATE name=Values(name), locationObjId=Values(locationObjId), contentObjId=Values(contentObjId), exitObjId=Values(exitObjId), nextObjId=Values(nextObjId), \
parentObjId=Values(parentObjId), zoneObjId=Values(zoneObjId), type=Values(type), powers=Values(powers), warnings=Values(warnings), flags=Values(flags), created=Values(created), \
modified=Values(modified), pennies=Values(pennies), ownerObjId=Values(ownerObjId)";
  SQLHSTMT hstmt = 0;

  SQLINTEGER id, location, content, exit, next, parent, zone, type, created,
    modified, pennies, owner;

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
  owner = DBObj->owner;

  SQLCHAR *name = (SQLCHAR *) strdup(Name(objID));
  SQLCHAR *flags =
    (SQLCHAR *) strdup(bits_to_string("FLAG", DBObj->flags, GOD, NOTHING));
  SQLCHAR *powers =
    (SQLCHAR *) strdup(bits_to_string("POWER", DBObj->powers, GOD, NOTHING));
  SQLCHAR *warnings = (SQLCHAR *) strdup(unparse_warnings(DBObj->warnings));
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
    SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                     sizeof(powers), 0, powers, sizeof(powers), &lenPowers);
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
    SQLBindParameter(hstmt, 16, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11,
                     0, &owner, sizeof(owner), NULL);

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
ON DUPLICATE KEY UPDATE ownerId=VALUES(ownerId), flags=Values(flags), derefs=Values(derefs), value=Values(value)";

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
