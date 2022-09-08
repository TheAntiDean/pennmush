

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


// odbc_get_attribs
// Get all attributes for a given object
// Parameters:
// dbref obj - the object to get attributes for
// int *num - the number of attributes returned
// Returns:
// NOTHING
void
odbc_get_attribs(dbref objID)
{
  char buff[BUFFER_LEN];
  memset(buff, 0, BUFFER_LEN);
  snprintf(buff, BUFFER_LEN, "objectId = %d", objID);

  ODBC_Query *q = ODBC_new_query("objectattrib", 5, buff, ODBC_GET);
  q->fields[0].name = "name";
  q->fields[0].type = ODBC_CHAR;
  q->fields[1].name = "ownerId";
  q->fields[1].type = ODBC_INT;
  q->fields[2].name = "flags";
  q->fields[2].type = ODBC_CHAR;
  q->fields[3].name = "derefs";
  q->fields[3].type = ODBC_INT;
  q->fields[4].name = "value";
  q->fields[4].type = ODBC_CHAR;

  ODBC_Result *res = NULL;
  ODBC_Result *tmp = NULL;
  res = ODBC_ExecuteQuery(q);

  while(res)
  {
    atr_new_add(objID, strdup((char *)res->fields[0].sValue), (char*)res->fields[4].sValue, res->fields[1].iValue,
    string_to_privs(attr_privs_view, strdup((char*)res->fields[2].sValue), 0), res->fields[3].iValue, 1);
    // get next result
    tmp = res;
    res = res->next;
    ODBC_free_result(tmp);

  }
  
}
// odbc_get_locks
// Get all locks for a given object
// Parameters:
// dbref obj - the object to get locks for
// int *num - the number of locks returned
// Returns:
// NOTHING
void
odbc_get_locks(dbref objID)
{

  char buff[BUFFER_LEN];
  memset(buff, 0, BUFFER_LEN);
  snprintf(buff, BUFFER_LEN, "objectId = %d", objID);
  ODBC_Query *q = ODBC_new_query("objectlock", 5, buff, ODBC_GET);
  q->fields[0].name = "type";
  q->fields[0].type = ODBC_CHAR;
  q->fields[1].name = "creatorId";
  q->fields[1].type = ODBC_INT;
  q->fields[2].name = "flags";
  q->fields[2].type = ODBC_CHAR;
  q->fields[3].name = "derefs";
  q->fields[3].type = ODBC_INT;
  q->fields[4].name = "boolexp";
  q->fields[4].type = ODBC_CHAR;

  ODBC_Result *res = NULL;
  ODBC_Result *tmp = NULL;
  res = ODBC_ExecuteQuery(q);

  while(res)
  {
    //b = parse_boolexp_d(GOD, key, type, derefs);
    
    boolexp b; // = parse_boolexp(res->fields[1].iValue, strdup((char*)&res->fields[4].sValue), res->fields[3].iValue);
    b = parse_boolexp_d(objID, (char*)res->fields[4].sValue, (char*)res->fields[0].sValue, res->fields[3].iValue);
      add_lock_raw(res->fields[1].iValue, objID, (char*)&res->fields[0].sValue[0], b,
                   string_to_privs(lock_privs, (char*)&res->fields[2].sValue[0], 0));
    // get next result
    tmp = res;
    res = res->next;
    ODBC_free_result(tmp);

  }

}

// odbc_read_object
// Read an object from the database
// Parameters:
// dbref obj - the object to read
// Returns:
// int - -1 if fail, dbmax if successful
int
odbc_read_object(void)
{
  ODBC_Query *q;

  sqlite3 *sqldb;
  sqldb = get_shared_db();
  sqlite3_stmt *adder;
  int status = 0;

  q = ODBC_new_query("object",16, NULL,ODBC_GET);
  q->reverse = 1;
  q->sort_field = (SQLCHAR*)"id";
  q->fields[0].name = "id";
  q->fields[0].type = ODBC_INT;
  q->fields[1].name = "name";
  q->fields[1].type = ODBC_CHAR;
  q->fields[2].name = "locationObjId";
  q->fields[2].type = ODBC_INT;
  q->fields[3].name = "contentObjId";
  q->fields[3].type = ODBC_INT;
  q->fields[4].name = "exitObjId";
  q->fields[4].type = ODBC_INT;
  q->fields[5].name = "nextObjId";
  q->fields[5].type = ODBC_INT;
  q->fields[6].name = "parentObjId";
  q->fields[6].type = ODBC_INT;
  q->fields[7].name = "zoneObjId";
  q->fields[7].type = ODBC_INT;
  q->fields[8].name = "type";
  q->fields[8].type = ODBC_INT;
  q->fields[9].name = "powers";
  q->fields[9].type = ODBC_CHAR;
  q->fields[10].name = "warnings";
  q->fields[10].type = ODBC_CHAR;
  q->fields[11].name = "flags";
  q->fields[11].type = ODBC_CHAR;
  q->fields[12].name = "created";
  q->fields[12].type = ODBC_INT;
  q->fields[13].name = "modified";
  q->fields[13].type = ODBC_INT;
  q->fields[14].name = "pennies";
  q->fields[14].type = ODBC_INT;
  q->fields[15].name = "ownerObjId";
  q->fields[15].type = ODBC_INT;

  ODBC_Result *res = ODBC_ExecuteQuery(q);
  ODBC_Result *tmp = NULL;
  ODBC_free_query(q);
  if (res == NULL) {
    return -1;
  return 1;
  }

  sqlite3_exec(sqldb, "BEGIN TRANSACTION", NULL, NULL, NULL);
  adder = prepare_statement(sqldb, "INSERT INTO objects(dbref) VALUES (?)",
                            "objects.add");
  int dbsize = 0;
  while(res)
  {
    struct object *o;
    dbref objId = res->fields[0].iValue;
    db_grow(objId + 1);
    o = db + objId;

    o->name = (char*)res->fields[1].sValue;
    set_name(objId, (char*)res->fields[1].sValue);
    o->location = res->fields[2].iValue;
    o->contents = res->fields[3].iValue;
    o->exits = res->fields[4].iValue;
    o->next = res->fields[5].iValue;
    o->parent = res->fields[6].iValue;
    o->zone = res->fields[7].iValue;
    o->type = res->fields[8].iValue;
    o->powers = string_to_bits("POWER", (char *)res->fields[9].sValue);
    o->warnings = parse_warnings(objId, (char *)res->fields[10].sValue);
    o->flags = string_to_bits("FLAG", (char *)res->fields[11].sValue);
    o->creation_time = res->fields[12].iValue;
    o->modification_time = res->fields[13].iValue;
    o->penn = res->fields[14].iValue;
    o->owner = res->fields[15].iValue;

        odbc_get_locks(objId);
    odbc_get_attribs(objId);

    switch (Typeof(objId)) {
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
      add_new_attr("MONIKER", AF_WIZARD | AF_NOPROG | AF_VISUAL | AF_LOCKED);
    }

    sqlite3_bind_int(adder, 1, objId);
    do {
      status = sqlite3_step(adder);
    } while (is_busy_status(status));
    if (status != SQLITE_DONE) {
      do_rawlog(LT_ERR, "Unable to add #%d to objects table: %s", objId,
                sqlite3_errstr(status));
    }
    sqlite3_reset(adder);

    if (IsPlayer(objId)) {
      add_player(objId);
      clear_flag_internal(objId, "CONNECTED");
      /* If it has the MONITOR flag and the db predates HEAR_CONNECT, swap
       * them over */
      if (!(globals.indb_flags & DBF_HEAR_CONNECT) &&
          has_flag_by_name(objId, "MONITOR", NOTYPE)) {
        clear_flag_internal(objId, "MONITOR");
        set_flag_internal(objId, "HEAR_CONNECT");
      }
    }
    tmp = res;
    res = (ODBC_Result *)res->next;
    ODBC_free_result(tmp);
  }
    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO ||
        retcode == SQL_NO_DATA) {
      sqlite3_exec(sqldb, "COMMIT TRANSACTION", NULL, NULL, NULL);
      fix_free_list();
      dbck();
    }
    
    return db_top;
  
}

// odbc_write_object
// Write an object to the database
// Returns 1 on success, 0 on failure
// Parameters:
//   dbref objId - the object to write
int
odbc_write_object(dbref objID)
{
  struct object *DBObj;
  DBObj = db + objID;
  ODBC_Query *q;
  q = ODBC_new_query("object", 16, NULL, ODBC_PUT);

  q->fields[0].name = "id";
  q->fields[0].type = ODBC_INT;
  q->fields[0].iValue = objID;
  q->fields[1].name = "name";
  q->fields[1].type = ODBC_CHAR;
  q->fields[1].sValue = (SQLCHAR *) Name(objID);
  q->fields[2].name = "locationObjId";
  q->fields[2].type = ODBC_INT;
  q->fields[2].iValue = DBObj->location;
  q->fields[3].name = "contentObjId";
  q->fields[3].type = ODBC_INT;
  q->fields[3].iValue = DBObj->contents;
  q->fields[4].name = "exitObjId";
  q->fields[4].type = ODBC_INT;
  q->fields[4].iValue = DBObj->exits;
  q->fields[5].name = "nextObjId";
  q->fields[5].type = ODBC_INT;
  q->fields[5].iValue = DBObj->next;
  q->fields[6].name = "parentObjId";
  q->fields[6].type = ODBC_INT;
  q->fields[6].iValue = DBObj->parent;
  q->fields[7].name = "zoneObjId";
  q->fields[7].type = ODBC_INT;
  q->fields[7].iValue = DBObj->zone;
  q->fields[8].name = "type";
  q->fields[8].type = ODBC_INT;
  q->fields[8].iValue = Typeof(objID);
  q->fields[9].name = "powers";
  q->fields[9].type = ODBC_CHAR;
  q->fields[9].sValue =
    (SQLCHAR *) bits_to_string("POWER", DBObj->powers, GOD, NOTHING);
  q->fields[10].name = "warnings";
  q->fields[10].type = ODBC_CHAR;
  q->fields[10].sValue = (SQLCHAR *) unparse_warnings(DBObj->warnings);
  q->fields[11].name = "flags";
  q->fields[11].type = ODBC_CHAR;
  q->fields[11].sValue =
    (SQLCHAR *) bits_to_string("FLAG", DBObj->flags, GOD, NOTHING);
  q->fields[12].name = "created";
  q->fields[12].type = ODBC_INT;
  q->fields[12].iValue = DBObj->creation_time;
  q->fields[13].name = "modified";
  q->fields[13].type = ODBC_INT;
  q->fields[13].iValue = DBObj->modification_time;
  q->fields[14].name = "pennies";
  q->fields[14].type = ODBC_INT;
  q->fields[14].iValue = DBObj->penn;
  q->fields[15].name = "ownerObjId";
  q->fields[15].type = ODBC_INT;
  q->fields[15].iValue = DBObj->owner;
  ODBC_ExecuteQuery(q);
  odbc_write_attribs(objID);
  odbc_write_locks(objID, DBObj->locks);
  ODBC_free_query(q);

  return 1;
}
// odbc_write_attribs
// Write an object's attributes to the database
// Returns 1 on success, 0 on failure
// Parameters:
//   dbref objId - the object to write
void
odbc_write_attribs(dbref objID)
{
  char buff[BUFFER_LEN];
  ALIST *list;
  ODBC_Query *da;
  snprintf(buff, BUFFER_LEN, "objectId=%d", objID);
  da = ODBC_new_query("objectattrib", 0, buff, ODBC_DELETE);
  ODBC_ExecuteQuery(da);
  ODBC_free_query(da);
  ODBC_Query *q;
  

  ATTR_FOR_EACH (objID, list) {

    q = ODBC_new_query("objectattrib", 6, NULL, ODBC_PUT);
    q->fields[0].name = "name";
    q->fields[0].type = ODBC_CHAR;
    q->fields[0].sValue = (SQLCHAR *) AL_NAME(list);
    q->fields[1].name = "ownerId";
    q->fields[1].type = ODBC_INT;
    q->fields[1].iValue = Owner(AL_CREATOR(list));
    q->fields[2].name = "flags";
    q->fields[2].type = ODBC_CHAR;
    q->fields[2].sValue = (SQLCHAR *) atrflag_to_string(AL_FLAGS(list));
    q->fields[3].name = "derefs";
    q->fields[3].type = ODBC_INT;
    q->fields[3].iValue = AL_DEREFS(list);
    q->fields[4].name = "objectId";
    q->fields[4].type = ODBC_INT;
    q->fields[4].iValue = objID;
    q->fields[5].name = "value";
    q->fields[5].type = ODBC_CHAR;
    q->fields[5].sValue = (SQLCHAR *) atr_value(list);

    ODBC_ExecuteQuery(q);
    ODBC_free_query(q);

  }
  
}

// odbc_write_locks
// Write an object's locks to the database
// Returns 1 on success, 0 on failure
// Parameters:
//   dbref objId - the object to write
//   LOCK *locks - the locks to write

void
odbc_write_locks(dbref objID, lock_list *l)
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
    ODBC_Query *q = ODBC_new_query("objectlock", 6, NULL, ODBC_PUT);

    q->fields[0].name = "type";
    q->fields[0].type = ODBC_CHAR;
    q->fields[0].sValue = (SQLCHAR *) ll->type;
    q->fields[1].name = "creatorId";
    q->fields[1].type = ODBC_INT;
    q->fields[1].iValue = Owner(ll->creator);
    q->fields[2].name = "flags";
    q->fields[2].type = ODBC_CHAR;
    q->fields[2].sValue = (SQLCHAR *) lock_flags_long(ll);
    q->fields[3].name = "derefs";
    q->fields[3].type = ODBC_INT;
    q->fields[3].iValue = chunk_derefs(L_KEY(ll));
    q->fields[4].name = "objectId";
    q->fields[4].type = ODBC_INT;
    q->fields[4].iValue = objID;
    q->fields[5].name = "boolexp";
    q->fields[5].type = ODBC_CHAR;
    q->fields[5].sValue = (SQLCHAR *) unparse_boolexp(GOD, L_KEY(ll), UB_DBREF);

    ODBC_ExecuteQuery(q);
    ODBC_free_query(q);
  }
}
