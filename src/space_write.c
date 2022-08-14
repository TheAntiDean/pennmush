/* space_write.c */

#include "config.h"
#include "space.h"

/* ------------------------------------------------------------------------ */

void
do_space_write_attr(dbref ship, char *baseName, char *attrName, double value)
{
  char buffer[BUFFER_LEN];
  memset(buffer, 0, sizeof(buffer));

  snprintf(buffer, BUFFER_LEN, "%s`%s", baseName, attrName);

  do_set_atr(ship, buffer, unparse_number(value), GOD,
          0);
}

int
do_space_db_write(dbref ship, dbref executor)
{
  register int i;
  int x, result;
  static char buffer[BUFFER_LEN];
  ATTR *a;

  /* SDB */


  x = 0;
  for (i = MIN_SPACE_OBJECTS; i <= max_space_objects; ++i)
    if (sdb[i].object == ship) {
      x = i;
      break;
    }
  if (!GoodSDB(x)) {
    a = atr_get(ship, SDB_ATTR_NAME);
    if (a == NULL) {
      write_spacelog(executor, ship, "WRITE: unable to read SDB attribute.");
      return 0;
    }
    result = parse_integer(atr_value(a));
    if (!GoodSDB(result)) {
      write_spacelog(executor, ship, "WRITE: unable to validate SDB.");
      return 0;
    } else if (sdb[result].object != ship) {
      write_spacelog(executor, ship, "WRITE: unable to verify SDB.");
      return 0;
    } else if (sdb[result].structure.type == 0) {
      write_spacelog(executor, ship, "WRITE: unable to verify STRUCTURE.");
      return 0;
    } else
      x = result;
  }
  snprintf(buffer, sizeof(buffer), "%d", x);
  result = atr_add(ship, SDB_ATTR_NAME, buffer, GOD,
                   (AF_MDARK + AF_WIZARD + AF_NOPROG));
  if (result != 0) {
    write_spacelog(executor, ship, "WRITE: unable to write SDB attribute.");
    return 0;
  } else if (max_space_objects < x)
    max_space_objects = x;

  /* OBJECT */

  if (!SpaceObj(ship) || !GoodObject(ship)) {
    write_spacelog(executor, ship, "WRITE: unable to validate SPACE_OBJECT.");
    return 0;
  }

  /* DEBUGGING */

  n = x;
  result = debug_space(x);
  if (result == 0) {
    write_spacelog(executor, ship, "WRITE: Bugs found and corrected.");
  }

  /* LOCATION */

  strncpy(buffer, unparse_integer(sdb[x].location), sizeof(buffer) - 1);
  result = atr_add(ship, LOCATION_ATTR_NAME, buffer, GOD,
                   (AF_MDARK + AF_WIZARD + AF_NOPROG));
  if (result != 0) {
    write_spacelog(executor, ship,
                   "WRITE: unable to write LOCATION attribute.");
    return 0;
  }

  /* SPACE */

  strncpy(buffer, unparse_integer(sdb[x].space), sizeof(buffer) - 1);
  result = atr_add(ship, SPACE_ATTR_NAME, buffer, GOD,
                   (AF_MDARK + AF_WIZARD + AF_NOPROG));
  if (result != 0) {
    write_spacelog(executor, ship, "WRITE: unable to write SPACE attribute.");
    return 0;
  }

  /* IFF */

  strncpy(buffer, unparse_number(sdb[x].iff.frequency), sizeof(buffer) - 1);
  result = atr_add(ship, IFF_ATTR_NAME, buffer, GOD,
                   (AF_MDARK + AF_WIZARD + AF_NOPROG));

  if (result != 0) {
    write_spacelog(executor, ship, "WRITE: unable to write IFF attribute.");
    return 0;
  }

  /* ALLOCATE */

  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "VERSION",
                      sdb[x].alloc.version);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "HELM", sdb[x].alloc.helm);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "TACTICAL",
                      sdb[x].alloc.tactical);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "OPERATIONS",
                      sdb[x].alloc.operations);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "MOVEMENT",
                      sdb[x].alloc.movement);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS",
                      sdb[x].alloc.shields);

  for (i = 0; i < 6; ++i) {

    snprintf(buffer, sizeof(buffer), "SHIELDS`%d", i);
    do_space_write_attr(ship, ALLOCATE_ATTR_NAME, buffer,
                        sdb[x].alloc.shield[i]);
  }

  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "CLOAK", sdb[x].alloc.cloak);
    do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "BEAMS",
                      sdb[x].alloc.beams);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "MISSILES",
                      sdb[x].alloc.missiles);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "SENSORS",
                      sdb[x].alloc.sensors);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "ECM", sdb[x].alloc.ecm);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "ECCM", sdb[x].alloc.eccm);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "TRANSPORTERS",
                      sdb[x].alloc.transporters);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "TRACTORS",
                      sdb[x].alloc.tractors);
  do_space_write_attr(ship, ALLOCATE_ATTR_NAME, "MISCELLANEOUS",
                      sdb[x].alloc.miscellaneous);

  /* BEAM */
  do_space_write_attr(ship, BEAM_ATTR_NAME, "IN", sdb[x].beam.in);
  do_space_write_attr(ship, BEAM_ATTR_NAME, "OUT", sdb[x].beam.out);
  do_space_write_attr(ship, BEAM_ATTR_NAME, "FREQ", sdb[x].beam.freq);
  do_space_write_attr(ship, BEAM_ATTR_NAME, "EXIST", sdb[x].beam.exist);
  do_space_write_attr(ship, BEAM_ATTR_NAME, "BANKS", sdb[x].beam.banks);

  /* BEAM_ACTIVE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.active[i]);
    }
  }

  /* BEAM_NAME */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "NAME`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.name[i]);
    }
  }

  /* BEAM_DAMAGE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "DAMAGE`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.damage[i]);
    }
  }

  /* BEAM_BONUS */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "BONUS`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.bonus[i]);
    }
  }

  /* BEAM_COST */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "COST`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.cost[i]);
    }
  }

  /* BEAM_RANGE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
	
      snprintf(buffer, sizeof(buffer), "RANGE`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.range[i]);
    }
  }

  /* BEAM_ARCS */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "ARCS`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.arcs[i]);
    }
  }

  /* BEAM_LOCK */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "LOCK`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.lock[i]);
    }
  }

  /* BEAM_LOAD */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "LOAD`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer, sdb[x].blist.load[i]);
    }
  }

  /* BEAM_RECYCLE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].beam.exist) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "RECYCLE`%d", i);
      do_space_write_attr(ship, BEAM_ATTR_NAME, buffer,
                          sdb[x].blist.recycle[i]);
    }
  }

  /* MISSILE */

  do_space_write_attr(ship, MISSILE_ATTR_NAME, "IN", sdb[x].missile.in);
  do_space_write_attr(ship, MISSILE_ATTR_NAME, "OUT", sdb[x].missile.out);
  do_space_write_attr(ship, MISSILE_ATTR_NAME, "FREQ", sdb[x].missile.freq);
  do_space_write_attr(ship, MISSILE_ATTR_NAME, "EXIST", sdb[x].missile.exist);
  do_space_write_attr(ship, MISSILE_ATTR_NAME, "TUBES", sdb[x].missile.tubes);

  /* MISSILE_ACTIVE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.active[i]);
    }
  }

  /* MISSILE_NAME */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "NAME`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.name[i]);
    }
  }

  /* MISSILE_DAMAGE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "DAMAGE`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.damage[i]);
    }
  }

  /* MISSILE_WARHEAD */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "WARHEAD`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.warhead[i]);
    }
  }

  /* MISSILE_COST */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "COST`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.cost[i]);
    }
  }

  /* MISSILE_RANGE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "RANGE`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.range[i]);
    }
  }

  /* MISSILE_ARCS */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "ARCS`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.arcs[i]);
    }
  }

  /* MISSILE_LOCK */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "LOCK`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.lock[i]);
    }
  }

  /* MISSILE_LOAD */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "LOAD`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.load[i]);
    }
  }

  /* MISSILE_RECYCLE */

  strncpy(buffer, "", sizeof(buffer) - 1);
  if (sdb[x].missile.exist) {
    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "RECYCLE`%d", i);
      do_space_write_attr(ship, MISSILE_ATTR_NAME, buffer,
                          sdb[x].mlist.recycle[i]);
    }
  }

  /* ENGINE */
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "VERSION", sdb[x].engine.version);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "WARP`DAMAGE",
                      sdb[x].engine.warp_damage);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "WARP`MAX",
                      sdb[x].engine.warp_max);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "WARP`EXIST",
                      sdb[x].engine.warp_exist);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "WARP`CRUISE",
                      sdb[x].engine.warp_cruise);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`DAMAGE",
                      sdb[x].engine.impulse_damage);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`MAX",
                      sdb[x].engine.impulse_max);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`CRUISE",
                      sdb[x].engine.impulse_cruise);
  do_space_write_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`EXIST",
                      sdb[x].engine.impulse_exist);

  /* STRUCTURE */
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "TYPE", sdb[x].structure.type);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "DISPLACEMENT",
                      sdb[x].structure.displacement);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "CARGO`HOLD",
                      sdb[x].structure.cargo_hold);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "CARGO`MASS",
                      sdb[x].structure.cargo_mass);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "SUPERSTRUCTURE",
                      sdb[x].structure.superstructure);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "SUPERSTRUCTURE`MAX",
                      sdb[x].structure.max_structure);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "LANDINGBAY`EXISTS",
                      sdb[x].structure.has_landing_pad);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "DOCKINGBAY`EXISTS",
                      sdb[x].structure.has_docking_bay);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "CANLAND",
                      sdb[x].structure.can_land);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "CANDOCK",
                      sdb[x].structure.can_dock);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "REPAIR",
                      sdb[x].structure.repair);
  do_space_write_attr(ship, STRUCTURE_ATTR_NAME, "REPAIR`MAX",
                      sdb[x].structure.max_repair);

  /* POWER */
  do_space_write_attr(ship, POWER_ATTR_NAME, "VERSION", sdb[x].power.version);
  do_space_write_attr(ship, POWER_ATTR_NAME, "MAIN", sdb[x].power.main);
  do_space_write_attr(ship, POWER_ATTR_NAME, "AUX", sdb[x].power.aux);
  do_space_write_attr(ship, POWER_ATTR_NAME, "BATT", sdb[x].power.batt);
  do_space_write_attr(ship, POWER_ATTR_NAME, "TOTAL", sdb[x].power.total);

  /* SENSOR */

  do_space_write_attr(ship, SENSOR_ATTR_NAME, "VERSION", sdb[x].sensor.version);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "LRS`DAMAGE",
                      sdb[x].sensor.lrs_damage);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "LRS`ACTIVE",
                      sdb[x].sensor.lrs_active);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "LRS`EXIST",
                      sdb[x].sensor.lrs_exist);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "LRS`SIGNATURE",
                      sdb[x].sensor.lrs_signature);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "LRS`RESOLUTION",
                      sdb[x].sensor.lrs_resolution);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "SRS`DAMAGE",
                      sdb[x].sensor.srs_damage);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "SRS`ACTIVE",
                      sdb[x].sensor.srs_active);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "SRS`EXIST",
                      sdb[x].sensor.srs_exist);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "SRS`SIGNATURE",
                      sdb[x].sensor.srs_signature);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "SRS`RESOLUTION",
                      sdb[x].sensor.srs_resolution);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "EW`DAMAGE",
                      sdb[x].sensor.ew_damage);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "EW`ACTIVE",
                      sdb[x].sensor.ew_active);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "EW`EXIST",
                      sdb[x].sensor.ew_exist);
  do_space_write_attr(ship, SENSOR_ATTR_NAME, "VISIBILITY",
                      sdb[x].sensor.visibility);

  /* SHIELD */

  do_space_write_attr(ship, SHIELD_ATTR_NAME, "RATIO", sdb[x].shield.ratio);
  do_space_write_attr(ship, SHIELD_ATTR_NAME, "MAXIMUM", sdb[x].shield.maximum);
  do_space_write_attr(ship, SHIELD_ATTR_NAME, "FREQ", sdb[x].shield.freq);
  do_space_write_attr(ship, SHIELD_ATTR_NAME, "EXIST", sdb[x].shield.exist);

  for (i = 0; i < 6; ++i) {
    snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
    do_space_write_attr(ship, SHIELD_ATTR_NAME, buffer,
                        sdb[x].shield.active[i]);
  }
  for (i = 0; i < 6; ++i) {
    snprintf(buffer, sizeof(buffer), "DAMAGE`%d", i);
    do_space_write_attr(ship, SHIELD_ATTR_NAME, buffer,
                        sdb[x].shield.damage[i]);
  }

  /* TECHNOLOGY */

  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "FIRING", sdb[x].tech.firing);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "FUEL", sdb[x].tech.fuel);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "STEALTH",
                      sdb[x].tech.stealth);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "CLOAK", sdb[x].tech.cloak);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "SENSORS",
                      sdb[x].tech.sensors);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "AUX`MAX",
                      sdb[x].tech.aux_max);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "MAIN`MAX",
                      sdb[x].tech.main_max);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "ARMOR", sdb[x].tech.armor);
  do_space_write_attr(ship, TECHNOLOGY_ATTR_NAME, "LY`RANGE",
                      sdb[x].tech.ly_range);

  /* MOVEMENT */

  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "TIME", sdb[x].move.time);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "DT", sdb[x].move.dt);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "IN", sdb[x].move.in);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "OUT", sdb[x].move.out);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "RATIO", sdb[x].move.ratio);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "COCHRANES",
                      sdb[x].move.cochranes);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "V", sdb[x].move.v);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "EMPIRE", sdb[x].move.empire);
  do_space_write_attr(ship, MOVEMENT_ATTR_NAME, "QUADRANT",
                      sdb[x].move.quadrant);

  /* CLOAK */

  do_space_write_attr(ship, CLOAK_ATTR_NAME, "VERSION", sdb[x].cloak.version);
  do_space_write_attr(ship, CLOAK_ATTR_NAME, "COST", sdb[x].cloak.cost);
  do_space_write_attr(ship, CLOAK_ATTR_NAME, "FREQ", sdb[x].cloak.freq);
  do_space_write_attr(ship, CLOAK_ATTR_NAME, "EXIST", sdb[x].cloak.exist);
  do_space_write_attr(ship, CLOAK_ATTR_NAME, "ACTIVE", sdb[x].cloak.active);
  do_space_write_attr(ship, CLOAK_ATTR_NAME, "DAMAGE", sdb[x].cloak.damage);

  /* TRANS */

  do_space_write_attr(ship, TRANS_ATTR_NAME, "COST", sdb[x].trans.cost);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "FREQ", sdb[x].trans.freq);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "EXIST", sdb[x].trans.exist);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "ACTIVE", sdb[x].trans.active);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "DAMAGE", sdb[x].trans.damage);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "LOCK`DEST", sdb[x].trans.d_lock);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "LOCK`SOURCE",
                      sdb[x].trans.s_lock);
  do_space_write_attr(ship, TRANS_ATTR_NAME, "COST", sdb[x].trans.cost);

  /* TRACT */
  do_space_write_attr(ship, TRACT_ATTR_NAME, "COST", sdb[x].tract.cost);
  do_space_write_attr(ship, TRACT_ATTR_NAME, "FREQ", sdb[x].tract.freq);
  do_space_write_attr(ship, TRACT_ATTR_NAME, "EXIST", sdb[x].tract.exist);
  do_space_write_attr(ship, TRACT_ATTR_NAME, "ACTIVE", sdb[x].tract.active);
  do_space_write_attr(ship, TRACT_ATTR_NAME, "DAMAGE", sdb[x].tract.damage);
  do_space_write_attr(ship, TRACT_ATTR_NAME, "LOCK", sdb[x].tract.lock);

  /* COORDS */

  do_space_write_attr(ship, COORDS_ATTR_NAME, "X", sdb[x].coords.x);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "Y", sdb[x].coords.y);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "Z", sdb[x].coords.z);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "XO", sdb[x].coords.xd);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "YO", sdb[x].coords.yd);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "ZO", sdb[x].coords.zd);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "XD", sdb[x].coords.xo);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "YD", sdb[x].coords.yo);
  do_space_write_attr(ship, COORDS_ATTR_NAME, "ZD", sdb[x].coords.zo);

  /* COURSE */
  do_space_write_attr(ship, COURSE_ATTR_NAME, "VERSION", sdb[x].course.version);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "YAW`IN", sdb[x].course.yaw_in);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "YAW`OUT", sdb[x].course.yaw_out);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "PITCH`IN",
                      sdb[x].course.pitch_in);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "PITCH`OUT",
                      sdb[x].course.pitch_out);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "ROLL`IN", sdb[x].course.roll_in);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "ROLL`OUT",
                      sdb[x].course.roll_out);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "X`X", sdb[x].course.d[0][0]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "X`Y", sdb[x].course.d[0][1]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "X`Z", sdb[x].course.d[0][2]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "Y`X", sdb[x].course.d[1][0]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "Y`Y", sdb[x].course.d[1][1]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "Y`Z", sdb[x].course.d[1][2]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "Z`X", sdb[x].course.d[2][0]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "Z`Y", sdb[x].course.d[2][1]);
  do_space_write_attr(ship, COURSE_ATTR_NAME, "Z`Z", sdb[x].course.d[2][2]);

  /* MAIN */

  do_space_write_attr(ship, MAIN_ATTR_NAME, "IN", sdb[x].main.in);
  do_space_write_attr(ship, MAIN_ATTR_NAME, "OUT", sdb[x].main.out);
  do_space_write_attr(ship, MAIN_ATTR_NAME, "DAMAGE", sdb[x].main.damage);
  do_space_write_attr(ship, MAIN_ATTR_NAME, "GW", sdb[x].main.gw);
  do_space_write_attr(ship, MAIN_ATTR_NAME, "EXIST", sdb[x].main.exist);

  /* AUX */

  do_space_write_attr(ship, AUX_ATTR_NAME, "IN", sdb[x].aux.in);
  do_space_write_attr(ship, AUX_ATTR_NAME, "OUT", sdb[x].aux.out);
  do_space_write_attr(ship, AUX_ATTR_NAME, "DAMAGE", sdb[x].aux.damage);
  do_space_write_attr(ship, AUX_ATTR_NAME, "GW", sdb[x].aux.gw);
  do_space_write_attr(ship, AUX_ATTR_NAME, "EXIST", sdb[x].aux.exist);

  /* BATT */

  do_space_write_attr(ship, BATT_ATTR_NAME, "IN", sdb[x].batt.in);
  do_space_write_attr(ship, BATT_ATTR_NAME, "OUT", sdb[x].batt.out);
  do_space_write_attr(ship, BATT_ATTR_NAME, "DAMAGE", sdb[x].batt.damage);
  do_space_write_attr(ship, BATT_ATTR_NAME, "GW", sdb[x].batt.gw);
  do_space_write_attr(ship, BATT_ATTR_NAME, "EXIST", sdb[x].batt.exist);

  /* FUEL */
  do_space_write_attr(ship, FUEL_ATTR_NAME, "ANTIMATTER",
                      sdb[x].fuel.antimatter);
  do_space_write_attr(ship, FUEL_ATTR_NAME, "DEUTERIUM", sdb[x].fuel.deuterium);
  do_space_write_attr(ship, FUEL_ATTR_NAME, "RESERVES", sdb[x].fuel.reserves);

  /* STATUS */

  do_space_write_attr(ship, STATUS_ATTR_NAME, "ACTIVE", sdb[x].status.active);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "DOCKED", sdb[x].status.docked);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "LANDED", sdb[x].status.landed);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "CONNECTED",
                      sdb[x].status.connected);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "CRIPPLED",
                      sdb[x].status.crippled);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "TRACTORING",
                      sdb[x].status.tractoring);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "TRACTORED",
                      sdb[x].status.tractored);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "LANDING`OPEN",
                      sdb[x].status.open_landing);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "DOCKING`OPEN",
                      sdb[x].status.open_docking);
  do_space_write_attr(ship, STATUS_ATTR_NAME, "LINK", sdb[x].status.link);

  return 1;
}

/* ------------------------------------------------------------------------ */
