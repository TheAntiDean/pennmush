#include "conf.h"
#include "space.h"


cJSON *
get_ship_power(int shipID);
cJSON *
get_space_status(char *system, char *subsystem, int shipID);
bool build_number(cJSON *json_element, int data);

FUNCTION(fun_aspace_jsondata)
{
  if (!GoodSDB(parse_number(args[0]))) {
    safe_str("#-1 INVALID SDB", buff, bp);
    return;
  }
  int shipID = parse_number(args[0]);
  cJSON *ship = NULL;
  cJSON *id = NULL;
  cJSON *name = NULL;
  cJSON *allocs = NULL;

  ship = cJSON_CreateObject();
  if(ship)
  {
    
    cJSON_AddNumberToObject(ship, "SDB", shipID);
    cJSON_AddNumberToObject(ship, "shipobj", sdb[shipID].object);
    cJSON_AddItemToObject(ship, "Allocations",get_ship_power(shipID));
    
    safe_format(buff, bp, cJSON_Print(ship));

  } else{
    safe_format(buff, bp, "#-1 Error parsing SDB information");
  }

  


}

cJSON *
get_space_status(char *system, char *subsystem, int shipID)
{
  if (strcmp(system, "power")) {
    cJSON *power = NULL;

    power = get_ship_power(shipID);
    if(power)
    {
        return power;
    }
  }

}

cJSON *
get_ship_power(int shipID)
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
      return;
    }
    cJSON_AddNumberToObject(engPower, "Total", sdb[shipID].power.total);
    cJSON_AddNumberToObject(engPower, "Helm", sdb[shipID].alloc.helm);
    cJSON_AddNumberToObject(engPower, "Tactical", sdb[shipID].alloc.tactical);
    cJSON_AddNumberToObject(engPower, "Operations", sdb[shipID].alloc.operations);
    cJSON_AddNumberToObject(engPower, "Helm", sdb[shipID].alloc.helm);
    // Add the result back to the main object
    cJSON_AddItemToObject(power, "Engineering", engPower);
    
  



  helmPower = cJSON_CreateObject();
  if (helmPower)
  {
   
    cJSON_AddNumberToObject(helmPower, "Movement", sdb[shipID].alloc.movement);
    cJSON_AddNumberToObject(helmPower, "Shields", sdb[shipID].alloc.shields);

    // Iterate shields
    cJSON *shields;
    shields = cJSON_CreateObject();
    if(shields)
    {
      cJSON_AddNumberToObject(shields, "Power", sdb[shipID].alloc.shields);
        int iterS = 0;
        for(iterS = 0; iterS < MAX_SHIELD_NAME; iterS++)
        {
            cJSON_AddNumberToObject(shields, shield_name[iterS], sdb[shipID].alloc.shield[iterS]);
        }
        cJSON_AddItemToObject(helmPower, "ShieldAlloc", shields);
    }
    
    // Cloak or "Other"
    cJSON_AddNumberToObject(helmPower, cloak_name[sdb[n].cloak.exist], sdb[shipID].alloc.cloak);
    // Add the result back to the main object
    cJSON_AddItemToObject(power, "Helm", helmPower);
  }
  
  // Tactical
  
  tactPower = cJSON_CreateObject();
  if(tactPower)
  {
    cJSON_AddNumberToObject(tactPower, "Beams", sdb[shipID].alloc.beams);
    cJSON_AddNumberToObject(tactPower, "Missiles", sdb[shipID].alloc.missiles);
    cJSON_AddNumberToObject(tactPower, "Sensors", sdb[shipID].alloc.sensors);
    cJSON *sensors = NULL;
    sensors = cJSON_CreateObject();
    if (sensors) {
      cJSON_AddNumberToObject(sensors, "ecm", sdb[shipID].alloc.ecm);
      cJSON_AddNumberToObject(sensors, "eccm", sdb[shipID].alloc.eccm);
    }
    cJSON_AddItemToObject(tactPower, "SensorAlloc", sensors);

    cJSON_AddItemToObject(power, "Tactical", tactPower);

  }

  // Ops

  opsPower = cJSON_CreateObject();
  if(opsPower)
  {
    cJSON_AddNumberToObject(opsPower, "Transporters", sdb[shipID].alloc.transporters);
    cJSON_AddNumberToObject(opsPower, "Tractors", sdb[shipID].alloc.tractors);
    cJSON_AddNumberToObject(opsPower, "Miscellaneous", sdb[shipID].alloc.miscellaneous);
    cJSON_AddItemToObject(power, "Operations", opsPower);
  }


  } // End if power
  return power;

}

bool build_number(cJSON *json_element, int data)
{
      json_element = cJSON_CreateNumber(data);
    if(json_element)
    {
        return true;
    }
    return false;
}



  
                                      /*
                                      // --------------------------

                                      // void report_tact_power (void)


                                      // --------------------------




                                      */

