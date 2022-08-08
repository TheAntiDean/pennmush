/* space_read.c */

#include "config.h"
#include "space.h"

/* ------------------------------------------------------------------------ */
double
do_space_read_attr(dbref ship, char *baseName, char *atrName)
{
  ATTR *a;
  double ret;
  char buff[BUFFER_LEN];
  memset(buff, 0, sizeof(buff));

  snprintf(buff, BUFFER_LEN, "%s`%s", baseName, atrName);
  a = atr_get(ship, buff);
  if (a) {
    return parse_number(atr_value(a));
  }

  return 1;
}


int
do_space_db_read(dbref ship, dbref executor)
{
  ATTR *a;
  char **array;
  register int i, x;
  int result;
  static char buffer[10];
  // dbref object;

  /* SDB */

  x = 0;
  for (i = MIN_SPACE_OBJECTS; i <= max_space_objects; ++i)
    if (sdb[i].object == ship) {
      x = i;
      break;
    }

  if (x == 0) {
    a = atr_get(ship, SDB_ATTR_NAME);
    if (a != NULL) { // See if the object has a SDB written. If so, let's try to
                     // use it.
      x = parse_integer(atr_value(a));
      if (sdb[x]
            .structure.type) { // This means it's already in use. Throw an error
        write_spacelog(executor, ship,
                       tprintf("READ: SDB (%d) already in use.", x));
        return 0;
      }
    } else {
      write_spacelog(executor, ship, tprintf("READ: Ship has no SDB set."));
      return 0;
    }
    if (max_space_objects < x)
      max_space_objects = x;
  }

  /* OBJECT */

  if (!SpaceObj(ship) || !GoodObject(ship)) {
    write_spacelog(executor, ship, "READ: unable to validate SPACE_OBJECT.");
    return 0;
  } else
    sdb[x].object = ship;

  /* SPACE */

  a = atr_get(ship, SPACE_ATTR_NAME);
  if (a == NULL) {
    write_spacelog(executor, ship, "READ: Unable to Read SPACE Attribute.");
    return 0;
  } else {
    sdb[x].space = parse_integer(atr_value(a));
  }

  /* IFF */

  a = atr_get(ship, IFF_ATTR_NAME);
  /* If we don't have an IFF Set, generate a random one */
  if (a == NULL) {
    snprintf(buffer, sizeof(buffer), "%d.%d", get_random_long(1, 999),
             get_random_long(1, 999));
    atr_add(ship, IFF_ATTR_NAME, buffer, GOD, 0);
    a = atr_get(ship, IFF_ATTR_NAME);

    if (a == NULL) {
      write_spacelog(
        executor, ship,
        "READ: Unable to Read IFF Attribute. Could not set default value.");
      return 0;
    }
  }

  array = mush_calloc(IFF_DATA_NUMBER + 1, sizeof(char *), "arrayarray");
  result = list2arr(array, IFF_DATA_NUMBER + 1, atr_value(a), ' ', 1);

  if (result == 0) {
    write_spacelog(executor, ship, "READ: Unable to Crack IFF Attribute.");
    mush_free(array, "arrayarray");
    return 0;
  }

  if (result != IFF_DATA_NUMBER) {
    mush_free(array, "arrayarray");
    write_spacelog(executor, ship,
                   "READ: Unable to Crack IFF Attribute Format.");
    return 0;
  }

  result += convert_double(array[0], 0.0, &sdb[x].iff.frequency);
  mush_free(array, "arrayarray");

  if (result == 0) {
    write_spacelog(executor, ship, "READ: Unable to convert IFF Attribute.");
    return 0;
  }

  if (result != IFF_DATA_NUMBER) {
    write_spacelog(executor, ship,
                   "READ: Unable to convert IFF Attribute format.");
    return 0;
  }

  /* ALLOCATE */
  sdb[x].alloc.version =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "VERSION");
  sdb[x].alloc.helm = do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "HELM");
  sdb[x].alloc.tactical =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "TACTICAL");
  sdb[x].alloc.operations =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "OPERATIONS");
  sdb[x].alloc.movement =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "MOVEMENT");
  sdb[x].alloc.shields =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS");
  sdb[x].alloc.shield[0] =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS`0");
  sdb[x].alloc.shield[1] =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS`1");
  sdb[x].alloc.shield[2] =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS`2");
  sdb[x].alloc.shield[3] =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS`3");
  sdb[x].alloc.shield[4] =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS`4");
  sdb[x].alloc.shield[5] =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SHIELDS`5");
  sdb[x].alloc.cloak = do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "CLOAK");
  sdb[x].alloc.beams = do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "BEAMS");
  sdb[x].alloc.missiles =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "MISSILES");
  sdb[x].alloc.sensors =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "SENSORS");
  sdb[x].alloc.ecm = do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "ECM");
  sdb[x].alloc.eccm = do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "ECCM");
  sdb[x].alloc.transporters =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "TRANSPORTERS");
  sdb[x].alloc.tractors =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "TRACTORS");
  sdb[x].alloc.miscellaneous =
    do_space_read_attr(ship, ALLOCATE_ATTR_NAME, "MISCELLANEOUS");

  /* BEAM */

  sdb[x].beam.in = do_space_read_attr(ship, BEAM_ATTR_NAME, "IN");
  sdb[x].beam.out = do_space_read_attr(ship, BEAM_ATTR_NAME, "OUT");
  sdb[x].beam.freq = do_space_read_attr(ship, BEAM_ATTR_NAME, "FREQ");
  sdb[x].beam.exist = do_space_read_attr(ship, BEAM_ATTR_NAME, "EXIST");
  sdb[x].beam.banks = do_space_read_attr(ship, BEAM_ATTR_NAME, "BANKS");

  /* BEAM_ACTIVE */

  if (sdb[x].beam.banks > 0) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
      sdb[x].blist.active[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_NAME */

  if (sdb[x].beam.banks > 0) {

    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "NAME`%d", i);
      sdb[x].blist.name[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }

    /* BEAM_DAMAGE */

    if (sdb[x].beam.banks > 0) {
      for (i = 0; i < sdb[x].beam.banks; ++i) {
        snprintf(buffer, sizeof(buffer), "DAMAGE`%d", i);
        sdb[x].blist.damage[i] =
          do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
      }
    }

    /* BEAM_BONUS */

    if (sdb[x].beam.banks > 0) {

      if (result < sdb[x].beam.banks) {
        write_spacelog(executor, ship,
                       "READ: Unable to Crack BEAM_BONUS Attribute Format.");
        mush_free(array, "arrayarray");
        return 0;
      }
      snprintf(buffer, sizeof(buffer), "BONUS`%d", i);
      sdb[x].blist.bonus[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_COST */

  if (sdb[x].beam.banks > 0) {
    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "COST`%d", i);
      sdb[x].blist.cost[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_RANGE */

  if (sdb[x].beam.banks > 0) {

    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "RANGE`%d", i);
      sdb[x].blist.range[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_ARCS */

  if (sdb[x].beam.banks > 0) {

    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "ARCS`%d", i);
      sdb[x].blist.arcs[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_LOCK */

  if (sdb[x].beam.banks > 0) {

    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "LOCK`%d", i);
      sdb[x].blist.lock[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_LOAD */

  if (sdb[x].beam.banks > 0) {

    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "LOAD`%d", i);
      sdb[x].blist.load[i] = do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }

  /* BEAM_RECYCLE */

  if (sdb[x].beam.banks > 0) {

    for (i = 0; i < sdb[x].beam.banks; ++i) {
      snprintf(buffer, sizeof(buffer), "RECYCLE`%d", i);
      sdb[x].blist.recycle[i] =
        do_space_read_attr(ship, BEAM_ATTR_NAME, buffer);
    }
  }
  /* MISSILE */

  sdb[x].missile.in = do_space_read_attr(ship, MISSILE_ATTR_NAME, "IN");
  sdb[x].missile.out = do_space_read_attr(ship, MISSILE_ATTR_NAME, "OUT");
  sdb[x].missile.freq = do_space_read_attr(ship, MISSILE_ATTR_NAME, "FREQ");
  sdb[x].missile.exist = do_space_read_attr(ship, MISSILE_ATTR_NAME, "EXIST");
  sdb[x].missile.tubes = do_space_read_attr(ship, MISSILE_ATTR_NAME, "TUBES");

  /* MISSILE_ACTIVE */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
      sdb[x].mlist.active[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_NAME */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
      sdb[x].mlist.active[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_DAMAGE */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "DAMAGE`%d", i);
      sdb[x].mlist.damage[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_WARHEAD */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "WARHEAD`%d", i);
      sdb[x].mlist.warhead[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_COST */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "COST`%d", i);
      sdb[x].mlist.cost[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_RANGE */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "RANGE`%d", i);
      sdb[x].mlist.range[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_ARCS */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "ARCS`%d", i);
      sdb[x].mlist.arcs[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_LOCK */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "LOCK`%d", i);
      sdb[x].mlist.lock[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_LOAD */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "LOAD`%d", i);
      sdb[x].mlist.load[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }

  /* MISSILE_RECYCLE */

  if (sdb[x].missile.tubes > 0) {

    for (i = 0; i < sdb[x].missile.tubes; ++i) {
      snprintf(buffer, sizeof(buffer), "RECYCLE`%d", i);
      sdb[x].mlist.recycle[i] =
        do_space_read_attr(ship, MISSILE_ATTR_NAME, buffer);
    }
  }
  /* ENGINE */

  sdb[x].engine.version = do_space_read_attr(ship, ENGINE_ATTR_NAME, "VERSION");
  sdb[x].engine.warp_damage =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "WARP`DAMAGE");
  sdb[x].engine.warp_max =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "WARP`MAX");
  sdb[x].engine.warp_exist =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "WARP`EXIST");
  sdb[x].engine.warp_cruise =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "WARP`CRUISE");
  sdb[x].engine.impulse_damage =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`DAMAGE");
  sdb[x].engine.impulse_max =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`MAX");
  sdb[x].engine.impulse_cruise =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`CRUISE");
  sdb[x].engine.impulse_exist =
    do_space_read_attr(ship, ENGINE_ATTR_NAME, "IMPULSE`EXIST");

  /* STRUCTURE */

  sdb[x].structure.type = do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "TYPE");
  sdb[x].structure.displacement =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "DISPLACEMENT");
  sdb[x].structure.cargo_hold =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "CARGO`HOLD");
  sdb[x].structure.cargo_mass =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "CARGO`MASS");
  sdb[x].structure.superstructure =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "SUPERSTRUCTURE");
  sdb[x].structure.max_structure =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "SUPERSTRUCTURE`MAX");
  sdb[x].structure.has_docking_bay =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "DOCKINGBAY`EXIST");
  sdb[x].structure.has_landing_pad =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "LANDINGBAY`EXIST");
  sdb[x].structure.can_land =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "CANLAND");
  sdb[x].structure.can_dock =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "CANDOCK");
  sdb[x].structure.repair =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "REPAIR");
  sdb[x].structure.max_repair =
    do_space_read_attr(ship, STRUCTURE_ATTR_NAME, "REPAIR`MAX");

  /* POWER */

  sdb[x].power.version = do_space_read_attr(ship, POWER_ATTR_NAME, "VERSION");
  sdb[x].power.main = do_space_read_attr(ship, POWER_ATTR_NAME, "MAIN");
  sdb[x].power.aux = do_space_read_attr(ship, POWER_ATTR_NAME, "AUX");
  sdb[x].power.batt = do_space_read_attr(ship, POWER_ATTR_NAME, "BATT");
  sdb[x].power.total = do_space_read_attr(ship, POWER_ATTR_NAME, "TOTAL");

  /* SENSOR */

  sdb[x].sensor.version = do_space_read_attr(ship, SENSOR_ATTR_NAME, "VERSION");
  sdb[x].sensor.lrs_damage =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "LRS`DAMAGE");
  sdb[x].sensor.lrs_active =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "LRS`ACTIVE");
  sdb[x].sensor.lrs_exist =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "LRS`EXIST");
  sdb[x].sensor.lrs_resolution =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "LRS`RESOLUTION");
  sdb[x].sensor.lrs_signature =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "LRS`SIGNATURE");
  sdb[x].sensor.srs_damage =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "SRS`DAMAGE");
  sdb[x].sensor.srs_active =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "SRS`ACTIVE");
  sdb[x].sensor.srs_exist =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "SRS`EXIST");
  sdb[x].sensor.srs_resolution =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "SRS`RESOLUTION");
  sdb[x].sensor.srs_signature =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "SRS`SIGNATURE");
  sdb[x].sensor.ew_damage =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "EW`DAMAGE");
  sdb[x].sensor.ew_active =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "EW`ACTIVE");
  sdb[x].sensor.ew_exist =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "EW`EXIST");
  sdb[x].sensor.visibility =
    do_space_read_attr(ship, SENSOR_ATTR_NAME, "VISIBILITY");

  sdb[x].sensor.contacts = 0;
  sdb[x].sensor.counter = 0;

  /* SHIELD */

  sdb[x].shield.ratio = do_space_read_attr(ship, SHIELD_ATTR_NAME, "RATIO");
  sdb[x].shield.maximum = do_space_read_attr(ship, SHIELD_ATTR_NAME, "MAXIMUM");
  sdb[x].shield.freq = do_space_read_attr(ship, SHIELD_ATTR_NAME, "FREQ");
  sdb[x].shield.exist = do_space_read_attr(ship, SHIELD_ATTR_NAME, "EXIST");


  for (i = 0; i < 6; ++i) {
    snprintf(buffer, sizeof(buffer), "ACTIVE`%d", i);
    sdb[x].shield.active[i] =
      do_space_read_attr(ship, SHIELD_ATTR_NAME, buffer);
  }

  for (i = 0; i < 6; ++i) {
    snprintf(buffer, sizeof(buffer), "DAMAGE`%d", i);
    sdb[x].shield.damage[i] =
      do_space_read_attr(ship, SHIELD_ATTR_NAME, buffer);
  }

  /* TECHNOLOGY */

  a = atr_get(ship, TECHNOLOGY_ATTR_NAME);
  sdb[x].tech.firing = do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "FIRING");
  sdb[x].tech.fuel = do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "FUEL");
  sdb[x].tech.stealth =
    do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "STEALTH");
  sdb[x].tech.cloak = do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "CLOAK");
  sdb[x].tech.sensors =
    do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "SENSORS");
  sdb[x].tech.aux_max =
    do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "AUX`MAX");
  sdb[x].tech.main_max =
    do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "MAIN`MAX");
  sdb[x].tech.armor = do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "ARMOR");
  sdb[x].tech.ly_range =
    do_space_read_attr(ship, TECHNOLOGY_ATTR_NAME, "LY`RANGE");

  /* MOVEMENT */

  sdb[x].move.time = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "TIME");
  sdb[x].move.dt = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "DT");
  sdb[x].move.in = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "IN");
  sdb[x].move.out = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "OUT");
  sdb[x].move.ratio = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "RATIO");
  sdb[x].move.cochranes =
    do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "COCHRANES");
  sdb[x].move.v = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "V");
  sdb[x].move.empire = do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "EMPIRE");
  sdb[x].move.quadrant =
    do_space_read_attr(ship, MOVEMENT_ATTR_NAME, "QUADRANT");

  /* CLOAK */

  sdb[x].cloak.version = do_space_read_attr(ship, CLOAK_ATTR_NAME, "VERSION");
  sdb[x].cloak.cost = do_space_read_attr(ship, CLOAK_ATTR_NAME, "COST");
  sdb[x].cloak.freq = do_space_read_attr(ship, CLOAK_ATTR_NAME, "FREQ");
  sdb[x].cloak.exist = do_space_read_attr(ship, CLOAK_ATTR_NAME, "EXIST");
  sdb[x].cloak.active = do_space_read_attr(ship, CLOAK_ATTR_NAME, "ACTIVE");
  sdb[x].cloak.damage = do_space_read_attr(ship, CLOAK_ATTR_NAME, "DAMAGE");

  /* TRANS */

  sdb[x].trans.cost = do_space_read_attr(ship, TRANS_ATTR_NAME, "COST");
  sdb[x].trans.freq = do_space_read_attr(ship, TRANS_ATTR_NAME, "FREQ");
  sdb[x].trans.exist = do_space_read_attr(ship, TRANS_ATTR_NAME, "EXIST");
  sdb[x].trans.active = do_space_read_attr(ship, TRANS_ATTR_NAME, "ACTIVE");
  sdb[x].trans.damage = do_space_read_attr(ship, TRANS_ATTR_NAME, "DAMAGE");
  sdb[x].trans.d_lock = do_space_read_attr(ship, TRANS_ATTR_NAME, "LOCK`DEST");
  sdb[x].trans.s_lock = do_space_read_attr(ship, TRANS_ATTR_NAME, "LOCK`SOURCE");
  sdb[x].trans.cost = do_space_read_attr(ship, TRANS_ATTR_NAME, "COST");

  /* TRACT */

  sdb[x].tract.cost = do_space_read_attr(ship, TRACT_ATTR_NAME, "COST");
  sdb[x].tract.freq = do_space_read_attr(ship, TRACT_ATTR_NAME, "FREQ");
  sdb[x].tract.exist = do_space_read_attr(ship, TRACT_ATTR_NAME, "EXIST");
  sdb[x].tract.active = do_space_read_attr(ship, TRACT_ATTR_NAME, "ACTIVE");
  sdb[x].tract.damage = do_space_read_attr(ship, TRACT_ATTR_NAME, "DAMAGE");
  sdb[x].tract.lock = do_space_read_attr(ship, TRACT_ATTR_NAME, "LOCK");

  /* COORDS */

  sdb[x].coords.x = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`X");
  sdb[x].coords.y = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`Y");
  sdb[x].coords.z = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`Z");
  sdb[x].coords.xo = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`XO");
  sdb[x].coords.yo = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`YO");
  sdb[x].coords.zo = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`ZO");
  sdb[x].coords.xd = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`XD");
  sdb[x].coords.yd = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`YD");
  sdb[x].coords.zd = do_space_read_attr(ship, COORDS_ATTR_NAME, "COORDS`ZD");

  /* COURSE */

  sdb[x].course.version = do_space_read_attr(ship, COURSE_ATTR_NAME, "VERSION");
  sdb[x].course.yaw_in = do_space_read_attr(ship, COURSE_ATTR_NAME, "YAW`IN");
  sdb[x].course.yaw_out = do_space_read_attr(ship, COURSE_ATTR_NAME, "YAW`OUT");
  sdb[x].course.pitch_in =
    do_space_read_attr(ship, COURSE_ATTR_NAME, "PITCH`IN");
  sdb[x].course.pitch_out =
    do_space_read_attr(ship, COURSE_ATTR_NAME, "PITCH`OUT");
  sdb[x].course.roll_in = do_space_read_attr(ship, COURSE_ATTR_NAME, "ROLL`IN");
  sdb[x].course.roll_out =
    do_space_read_attr(ship, COURSE_ATTR_NAME, "ROLL`OUT");

  sdb[x].course.d[0][0] = do_space_read_attr(ship, COURSE_ATTR_NAME, "X`X");
  sdb[x].course.d[0][1] = do_space_read_attr(ship, COURSE_ATTR_NAME, "X`Y");
  sdb[x].course.d[0][2] = do_space_read_attr(ship, COURSE_ATTR_NAME, "X`Z");
  sdb[x].course.d[1][0] = do_space_read_attr(ship, COURSE_ATTR_NAME, "Y`X");
  sdb[x].course.d[1][1] = do_space_read_attr(ship, COURSE_ATTR_NAME, "Y`Y");
  sdb[x].course.d[1][2] = do_space_read_attr(ship, COURSE_ATTR_NAME, "Y`Z");
  sdb[x].course.d[2][0] = do_space_read_attr(ship, COURSE_ATTR_NAME, "Z`X");
  sdb[x].course.d[2][1] = do_space_read_attr(ship, COURSE_ATTR_NAME, "Z`Y");
  sdb[x].course.d[2][2] = do_space_read_attr(ship, COURSE_ATTR_NAME, "Z`Z");
  sdb[x].course.rate = do_space_read_attr(ship, COURSE_ATTR_NAME, "RATE");

  /* MAIN */

  sdb[x].main.in = do_space_read_attr(ship, MAIN_ATTR_NAME, "IN");
  sdb[x].main.out = do_space_read_attr(ship, MAIN_ATTR_NAME, "OUT");
  sdb[x].main.damage = do_space_read_attr(ship, MAIN_ATTR_NAME, "DAMAGE");
  sdb[x].main.gw = do_space_read_attr(ship, MAIN_ATTR_NAME, "GW");
  sdb[x].main.exist = do_space_read_attr(ship, MAIN_ATTR_NAME, "EXIST");

  /* AUX */

  sdb[x].aux.in = do_space_read_attr(ship, AUX_ATTR_NAME, "IN");
  sdb[x].aux.out = do_space_read_attr(ship, AUX_ATTR_NAME, "OUT");
  sdb[x].aux.damage = do_space_read_attr(ship, AUX_ATTR_NAME, "DAMAGE");
  sdb[x].aux.gw = do_space_read_attr(ship, AUX_ATTR_NAME, "GW");
  sdb[x].aux.exist = do_space_read_attr(ship, AUX_ATTR_NAME, "EXIST");

  /* BATT */

  sdb[x].batt.in = do_space_read_attr(ship, BATT_ATTR_NAME, "IN");
  sdb[x].batt.out = do_space_read_attr(ship, BATT_ATTR_NAME, "OUT");
  sdb[x].batt.damage = do_space_read_attr(ship, BATT_ATTR_NAME, "DAMAGE");
  sdb[x].batt.gw = do_space_read_attr(ship, BATT_ATTR_NAME, "GW");
  sdb[x].batt.exist = do_space_read_attr(ship, BATT_ATTR_NAME, "EXIST");


  /* FUEL */

  sdb[x].fuel.antimatter =
    do_space_read_attr(ship, FUEL_ATTR_NAME, "ANTIMATTER");
  sdb[x].fuel.deuterium = do_space_read_attr(ship, FUEL_ATTR_NAME, "DEUTERIUM");
  sdb[x].fuel.reserves = do_space_read_attr(ship, FUEL_ATTR_NAME, "RESERVES");

  /* STATUS */

  sdb[x].status.active = do_space_read_attr(ship, STATUS_ATTR_NAME, "ACTIVE");
  sdb[x].status.docked = do_space_read_attr(ship, STATUS_ATTR_NAME, "DOCKED");
  sdb[x].status.landed = do_space_read_attr(ship, STATUS_ATTR_NAME, "LANDED");
  sdb[x].status.connected =
    do_space_read_attr(ship, STATUS_ATTR_NAME, "CONNECTED");
  sdb[x].status.crippled =
    do_space_read_attr(ship, STATUS_ATTR_NAME, "CRIPPLED");
  sdb[x].status.tractoring =
    do_space_read_attr(ship, STATUS_ATTR_NAME, "TRACTORING");
  sdb[x].status.tractored =
    do_space_read_attr(ship, STATUS_ATTR_NAME, "TRACTORED");
  sdb[x].status.open_docking =
    do_space_read_attr(ship, STATUS_ATTR_NAME, "DOCKING`OPEN");
  sdb[x].status.open_landing =
    do_space_read_attr(ship, STATUS_ATTR_NAME, "LANDING`OPEN");
  sdb[x].status.link = do_space_read_attr(ship, STATUS_ATTR_NAME, "LINK");

  /* DEBUGGING */

  result = debug_space(x);
  if (result == 0) {
    write_spacelog(executor, ship, "READ: Bugs Found and Corrected.");
  }

  return 1;
}

/* -------------------------------------------------------------------- */
