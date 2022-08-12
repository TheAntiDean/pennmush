#include "conf.h"
#include "space.h"

cJSON *get_ship_power(int shipSDB);
cJSON *get_sensor_report(int shipSDB);
void get_space_status(char *system, char *subsystem, int shipSDB, char *buff,
                        char **bp);
bool build_number(cJSON *json_element, int data);

FUNCTION(fun_aspace_jsondata)
{
  int shipSDB = parse_number(args[0]);
  if (!GoodSDB(parse_number(args[0]))) {
    safe_str("#-1 INVALID SDB", buff, bp);
    return;
  }
  get_space_status(args[1], args[2], shipSDB, buff, bp);
}

void
get_space_status(char *system, char *subsystem, int shipSDB, char *buff,
                 char **bp)
{
  

  cJSON *ship = NULL;

  ship = cJSON_CreateObject();
  if (ship) {

    cJSON_AddNumberToObject(ship, "SDB", shipSDB);
    cJSON_AddNumberToObject(ship, "shipobj", sdb[shipSDB].object);
    if (strcmp(system, "power") == 0) {
      cJSON_AddItemToObject(ship, "Allocations", get_ship_power(shipSDB));
    } else if (strcmp(system, "senrep") == 0) {
      cJSON_AddItemToObject(ship, "Contacts", get_sensor_report(shipSDB));
    }
  
    safe_format(buff, bp,"%s", remove_markup(cJSON_PrintUnformatted(ship),NULL));

  } else {
    safe_format(buff, bp, "#-1 Error parsing SDB information");
  }
}

cJSON *
get_sensor_report(int shipSDB)
{
  cJSON *contacts = NULL;

  contacts = cJSON_CreateObject();
  if (contacts) {
    int contID = 0;
    for (contID = 0; contID < sdb[shipSDB].sensor.contacts; ++contID) {
      cJSON *contact = NULL;
      contact = cJSON_CreateObject();
      int contSDB = sdb[shipSDB].slist.sdb[contID];
      cJSON_AddNumberToObject(contact, "SDB", contSDB);
      cJSON_AddNumberToObject(contact, "dbref", sdb[contSDB].object);
      cJSON_AddNumberToObject(contact, "Num", sdb[n].slist.num[contSDB]+1);
      cJSON_AddStringToObject(contact, "Type", unparse_type(contSDB));

      double resolution = sdb[shipSDB].slist.lev[contID] * 100.0;

      if (resolution < 0.0) {
        resolution = 0.0;
      } else if (resolution > 100.0) {
        resolution = 100.0;
      }

      if (resolution < 25) {
        cJSON_AddStringToObject(contact, "Name", "Unresolved");
      } else if (resolution < 50) {
        cJSON_AddStringToObject(
          contact, "Class",
          (sdb[contSDB].cloak.active ? "(cloaked)" : unparse_class(contSDB)));
        cJSON_AddStringToObject(
          contact, "Name",
          (sdb[contSDB].cloak.active ? "(cloaked)" : unparse_class(contSDB)));
      } else {
        cJSON_AddStringToObject(
          contact, "Class",
          (sdb[contSDB].cloak.active ? "(cloaked)" : unparse_class(contSDB)));
        cJSON_AddStringToObject(contact, "Name",
                                (sdb[contSDB].cloak.active
                                   ? "(cloaked)"
                                   : Name(sdb[contSDB].object)));
      }

      cJSON_AddNumberToObject(contact, "Resolution", resolution);
      cJSON_AddNumberToObject(contact, "Bearing",
                              sdb2bearing(shipSDB, contSDB));
      cJSON_AddNumberToObject(contact, "Elevation",
                              sdb2elevation(shipSDB, contSDB));
      cJSON_AddStringToObject(contact, "Range",
                              unparse_range(sdb2range(shipSDB, contSDB)));
      cJSON_AddStringToObject(contact, "Arc",
                              unparse_arc(sdb2arc(shipSDB, contSDB)));
      cJSON_AddStringToObject(contact, "FacingArc",
                              unparse_arc(sdb2arc(contSDB, shipSDB)));
      cJSON_AddNumberToObject(contact, "HeadingX", sdb[contSDB].course.yaw_out);
      cJSON_AddNumberToObject(contact, "HeadingY",
                              sdb[contSDB].course.pitch_out);
      cJSON_AddStringToObject(contact, "Speed",
                              unparse_speed(sdb[contSDB].move.out));
      cJSON_AddNumberToObject(contact, "IFFMatch",
                              sdb[shipSDB].iff.frequency ==
                                sdb[contSDB].iff.frequency);
      cJSON_AddStringToObject(contact, "Flags",
                              contact_flags(contSDB));
      

      cJSON_AddItemToObject(contacts, unparse_number(contID), contact);

    }
  }
  return contacts;
}

cJSON *
get_ship_power(int shipSDB)
{
  cJSON *power = NULL;
  // Eng
  cJSON *engPower = NULL;
  // Helm
  cJSON *helmPower = NULL;
  // Tactical
  cJSON *tactPower = NULL;
  // Operations
  cJSON *opsPower = NULL;

  // JSON struct
  power = cJSON_CreateObject();
  if (power) {
    // Engineering
    engPower = cJSON_CreateObject();
    if (engPower == NULL) {
      return NULL;
    }
    cJSON_AddNumberToObject(engPower, "Total", sdb[shipSDB].power.total);
    cJSON_AddNumberToObject(engPower, "Helm", sdb[shipSDB].alloc.helm);
    cJSON_AddNumberToObject(engPower, "Tactical", sdb[shipSDB].alloc.tactical);
    cJSON_AddNumberToObject(engPower, "Operations",
                            sdb[shipSDB].alloc.operations);
    cJSON_AddNumberToObject(engPower, "Helm", sdb[shipSDB].alloc.helm);
    // Add the result back to the main object
    cJSON_AddItemToObject(power, "Engineering", engPower);

    helmPower = cJSON_CreateObject();
    if (helmPower) {

      cJSON_AddNumberToObject(helmPower, "Movement",
                              sdb[shipSDB].alloc.movement);
      if(sdb[shipSDB].shield.exist)
      {
      cJSON_AddNumberToObject(helmPower, "Shields", sdb[shipSDB].alloc.shields);

      // Iterate shields
      cJSON *shields;
      shields = cJSON_CreateObject();
      if (shields) {
        cJSON_AddNumberToObject(shields, "Power", sdb[shipSDB].alloc.shields);
        int iterS = 0;
        for (iterS = 0; iterS < MAX_SHIELD_NAME; iterS++) {
          cJSON_AddNumberToObject(shields, shield_name[iterS],
                                  sdb[shipSDB].alloc.shield[iterS]);
        }
        cJSON_AddItemToObject(helmPower, "ShieldAlloc", shields);
      }
      }
      // Cloak or "Other"
      cJSON_AddNumberToObject(helmPower, cloak_name[sdb[n].cloak.exist],
                              sdb[shipSDB].alloc.cloak);
      // Add the result back to the main object
      cJSON_AddItemToObject(power, "Helm", helmPower);
    }

    // Tactical

    tactPower = cJSON_CreateObject();
    if (tactPower) {
      cJSON_AddNumberToObject(tactPower, "Beams", sdb[shipSDB].alloc.beams);
      cJSON_AddNumberToObject(tactPower, "Missiles",
                              sdb[shipSDB].alloc.missiles);
      cJSON_AddNumberToObject(tactPower, "Sensors", sdb[shipSDB].alloc.sensors);
      cJSON *sensors = NULL;
      sensors = cJSON_CreateObject();
      if (sensors) {
        cJSON_AddNumberToObject(sensors, "ecm", sdb[shipSDB].alloc.ecm);
        cJSON_AddNumberToObject(sensors, "eccm", sdb[shipSDB].alloc.eccm);
      }
      cJSON_AddItemToObject(tactPower, "SensorAlloc", sensors);

      cJSON_AddItemToObject(power, "Tactical", tactPower);
    }

    // Ops

    opsPower = cJSON_CreateObject();
    if (opsPower) {
      cJSON_AddNumberToObject(opsPower, "Transporters",
                              sdb[shipSDB].alloc.transporters);
      cJSON_AddNumberToObject(opsPower, "Tractors",
                              sdb[shipSDB].alloc.tractors);
      cJSON_AddNumberToObject(opsPower, "Miscellaneous",
                              sdb[shipSDB].alloc.miscellaneous);
      cJSON_AddItemToObject(power, "Operations", opsPower);
    }

  } // End if power
  return power;
}

bool
build_number(cJSON *json_element, int data)
{
  json_element = cJSON_CreateNumber(data);
  if (json_element) {
    return true;
  }
  return false;
}

/*
// --------------------------

// void report_tact_power (void)


// --------------------------




*/
