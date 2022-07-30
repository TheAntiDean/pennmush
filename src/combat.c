#include "config.h"
#include "combat.h"

COMMAND(cmd_local_attack)
{
  time_t t;

  combatStats aStats = getStats(executor); // store player data for later

  // get the current time for attack timing.

  int timeNow = time(NULL);
  
  dbref target =
    noisy_match_result(executor, arg_left, TYPE_PLAYER, MAT_EVERYTHING);
  //  - Determine Defender dbref - Done

  if (target == NOTHING) {
    notify_format(executor, "Target not found: %s", arg_left);
    return;
  }
  if (aStats.nextAttackTime > timeNow) {
    notify_format(executor,
                  "You can't attack yet. %ds remaining until your next attack.",
                  (timeNow - aStats.nextAttackTime) + '0');
    return;
  }

  notify_format(executor, "Target found: %s", Name(target));
  if (aStats.stamina < 1) {
    notify_format(executor, "%sGAME: %s%sYou have no stamina: %d %s",
                  ANSI_HIGREEN, ANSI_END, ANSI_CYAN, aStats.stamina,
                  ANSI_ENDALL);
    return;
  }
  notify_format(executor, "%sGAME: %s%sstamina: %d %s", ANSI_HIGREEN, ANSI_END,
                ANSI_CYAN, aStats.stamina, ANSI_ENDALL);
  // store target data for later.
  combatStats tStats = getStats(target);

  combatStats aModStats = getModifiedStats(executor);
  //  - Get his health, and stamina, and dodge skill
  combatStats tModStats = getModifiedStats(target);
  srand((unsigned) time(&t));

  if (aModStats.attackSkill > 99) {
    aModStats.attackSkill = 99;
  } else if (aModStats.attackSkill < 1) {
    aModStats.attackSkill = 1;
  }

  if (aModStats.attackSkill > 99) {
    aModStats.attackSkill = 99;
  } else if (aModStats.attackSkill < 1) {
    aModStats.attackSkill = 1;
  }

  if (rand() % 99 < aModStats.attackSkill) { // AIM
                                             //   ATTACK:
    //   - pick a random number between 0 and 100 for the attacker's roll. -
    //   done
    //   - Get the attacker's attack skill - done
    //   - add in any attack bonuses and subtract any attack penalties provided
    //   by weapons/armor, but limit between 1 and 99. - done
    //   - IF attack skill plus bonuses is greater than the random roll, call D
    //   EFEND - done
    //   - ELSE call MISS - done
    if (tModStats.dodgeSkill > 99) {
      tModStats.dodgeSkill = 99;
    } else if (tModStats.dodgeSkill < 1) {
      tModStats.dodgeSkill = 1;
    }
    if (rand() % 99 < tModStats.dodgeSkill && tModStats.stamina > 0) { // DODGE
      //   DEFEND:
      //   - pick a random number between 1 and 99 for the defender's roll.
      //   - Get the defender's dodge skill
      //   - add in any dodge bonuses and subtract any dodge penalties provided
      //   by weapons/armor, but limit between 1 and 99.
      //   - if stamina is zero, call HIT
      //   - IF dodge skill plus bonuses is greater than random roll, call DODGE
      //   - ELSE call HIT
      do_defend(executor, target);

    } else { // HIT
      do_attack(executor, target);
      notify_format(executor, "ERR: HEALTH ATRVAL:  %s",
                    unparse_integer((tStats.health - aModStats.damage)));
    }

  } else { // MISS
    //   MISS:
    // - decrement attacker stamina
    // - Display miss messages for weapon

    notify_miss(executor, target);
  }

  // Regardless of what happened, assign next attack time
  setAtrValue(executor, CE_NEXTATTACKTIME, (timeNow + aStats.attackSpeed));
  // Decrease attacker stamina
  setAtrValue(executor, CE_STAMINA, aStats.stamina - aModStats.actionStamina);

  return 1;
  /*

  DODGE:
    - decrement attacker stamina
    - decrement defender stamina
    - Display dodge messages for weapon
  HIT:
   - decrement defender health (by a random amount from 0 to damage?)
   - decrement attacker stamina
    - display hit messages for weapon
  EXIT:


   */
}

COMMAND(cmd_local_equip) {}
COMMAND(cmd_local_unequip) {}

FUNCTION(local_fun_attack) { safe_format(buff, bp, "Attack%sAttack", args[0]); }

// ~250ms
void
do_combat_iterate()
{
  // Update Health
  // Update Bleed
  // Undate Unconscious
  // Update Stamina
}

void
initCombat()
{
  setupCombatFlags();
  setupCombatPowers();
  setupCombatCmds();
  setupCombatFunc();
  setupCombatAttr();
}

void
setupCombatCmds()
{

  command_add("ATTACK", CMD_T_ANY, "WIZARD ROYALTY", "SEE_ALL",
              "NOISY NOEVAL VERY", cmd_local_attack);
  alias_command("ATTACK", "HIT");
  alias_command("ATTACK", "STAB");
  alias_command("ATTACK", "CUT");
  alias_command("ATTACK", "SHOOT");
  command_add("EQUIP", CMD_T_ANY, "WIZARD ROYALTY", "SEE_ALL",
              "NOISY NOEVAL VERY", cmd_local_equip);
  command_add("UNEQUIP", CMD_T_ANY, "WIZARD ROYALTY", "SEE_ALL",
              "NOISY NOEVAL VERY", cmd_local_unequip);
}
void
setupCombatFlags()
{

  add_flag(
    "WEAPON", "*", TYPE_THING, F_ANY,
    F_ANY); // signifies an object that provides attack damage and stamina
  add_flag("ARMOR", "A", TYPE_THING, F_ANY,
           F_ANY); // Signifies an object that provides armor and stamina and
                   // defense bonuses.
  add_flag("BLEEDING", "b", TYPE_PLAYER, F_ANY,
           F_ANY); // player will loose health each combat tick.
  add_flag("UNCONSCIOUS", "b", TYPE_PLAYER, F_ANY,
           F_ANY); // player will be unable to move/speak/hear/use commands.
                   // Will eventually recover over time
  add_flag("DEAD", "D", TYPE_PLAYER, F_ANY,
           F_ANY); // player will be unable to move/speak/hear/use commands.
                   // Will not recover.
}

void
setupCombatPowers()
{
#ifdef EXAMPLE
  add_power("CanUseAirlock", 1, TYPE_PLAYER, F_ANY, F_ANY);
#endif
}

void
setupCombatFunc()
{

  function_add("ATTACK", local_fun_attack, 1, 1, FN_REG);
}

void
setupCombatAttr()
{

  add_new_attr(CE_HEALTH,
               AF_WIZARD); // The current health of the player/armor/weapon
  add_new_attr(CE_MAXHEALTH,
               AF_WIZARD); // The most health a player can heal up to or
                           // armor/weapon can be repaired to.
  add_new_attr(
    CE_STAMINA,
    AF_WIZARD); // The current stamina of the player or the bonus/penalty to
                // player stamina caused by the armor/weapon
  add_new_attr(
    CE_MAXSTAMINA,
    AF_WIZARD); // The most stamina a player can recover, or an bonus/penalty to
                // base stamina caused by armor/weapon

  add_new_attr(
    CE_SKILLATTACK,
    AF_WIZARD); // How likely the player is to land a hit.  For weapon or armor,
                // bonus or penalty to player base stat
  add_new_attr(
    CE_SKILLDODGE,
    AF_WIZARD); // How likely the player is to dodge an attack.  For weapon or
                // armor, bonus or penalty to player base stat.
  add_new_attr(
    CE_DAMAGE,
    AF_WIZARD); // How much damage the weapon (or player) can dish out
  add_new_attr(CE_EQUIPPEDBY, AF_WIZARD); // dbref of player using weapon/armor

  add_new_attr(
    CE_NEXTATTACKTIME,
    AF_WIZARD &
      AF_INTERNAL); // TImestamp at which time the next attack can be performed
  add_new_attr(CE_ATTACKSPEED,
               AF_WIZARD); // number of seconds to increment current timestamp
                           // when performing an attack.
  add_new_attr(CE_ACTIONSTAMINA, AF_WIZARD);

  alias_attribute("Health", "Life");
}

combatStats
getStats(dbref obj)
{
  combatStats cstat;
  cstat.health = parse_number(getAtrValue(obj, CE_HEALTH));
  cstat.maxHealth = parse_number(getAtrValue(obj, CE_MAXHEALTH));
  cstat.stamina = parse_number(getAtrValue(obj, CE_STAMINA));
  cstat.maxStamina = parse_number(getAtrValue(obj, CE_MAXSTAMINA));
  cstat.attackSkill = parse_number(getAtrValue(obj, CE_SKILLATTACK));
  cstat.dodgeSkill = parse_number(getAtrValue(obj, CE_SKILLDODGE));
  cstat.equippedBy = parse_number(getAtrValue(obj, CE_EQUIPPEDBY));
  cstat.damage = parse_number(getAtrValue(obj, CE_DAMAGE));
  cstat.attackSpeed = parse_number(getAtrValue(obj, CE_ATTACKSPEED));
  cstat.nextAttackTime = parse_number(getAtrValue(obj, CE_NEXTATTACKTIME));
  cstat.actionStamina = parse_number(getAtrValue(obj, CE_ACTIONSTAMINA));

  return cstat;
}

bool
setAtrValue(dbref player, char *attr, int value)
{
  return do_set_atr(player, attr, unparse_integer(value), GOD, 0);
}

char *
getAtrValue(dbref obj, char *name)
{

  ATTR *a = atr_get_noparent(obj, name);
  if (!a)
    return "";

  return atr_value(a);
}

bool
isCombatItem(dbref obj)
{
  return (has_flag_by_name(obj, CF_ARMOR, NOTYPE) ||
          has_flag_by_name(obj, CF_WEAPON, NOTYPE)) &&
         IsThing(obj);
}

dbref
equippedBy(dbref obj)
{
  // If an object is dropped, it should get this attribute cleared automatically
  // somehow
  return parse_dbref(atr_value(atr_get(obj, CE_EQUIPPEDBY)));
}

combatStats
getModifiedStats(dbref player)
{
  // - Check his contents for equipped armor/weapons and
  // increment/decrement his
  // health/stamina/dodge for each item that affects it. - done
  // - Determine Attacker's dbref - done
  // - if stamina is zero, call EXIT -done
  // - Get his health, stamina, and attack skill-done
  // - Check his contents for equipped armor/weapons and increment/decrement
  //  his
  //  health/stamina/dodge for each item that affects it. - done
  combatStats pStats = getStats(player);
  dbref invItem = NOTHING;
  DOLIST (invItem, Contents(player)) {
    combatStats oStats = getStats(invItem);
    if (isCombatItem(invItem) && equippedBy(invItem) == player) {
      pStats.health += oStats.health;
      pStats.stamina += oStats.stamina;
      pStats.attackSkill += oStats.attackSkill;
      pStats.dodgeSkill += oStats.dodgeSkill;
      pStats.damage += oStats.damage;
      pStats.attackSpeed -= oStats.attackSpeed;
      pStats.actionStamina += oStats.actionStamina;
    }
  }
  return pStats;
}

void
do_attack(dbref attacker, dbref defender)
{
  combatStats dStats = getStats(defender);
  combatStats aModStats = getModifiedStats(attacker),
              dModStats = getModifiedStats(defender);
  setAtrValue(defender, CE_HEALTH, dStats.health - aModStats.damage);
  setAtrValue(defender, CE_STAMINA, dStats.stamina - dModStats.actionStamina);
  notify_hit(attacker, defender);
}

void
do_defend(dbref attacker, dbref defender)
{
  combatStats aStats = getStats(attacker), dStats = getStats(defender);
  combatStats aModStats = getModifiedStats(attacker),
              dModStats = getModifiedStats(defender);
  setAtrValue(defender, CE_STAMINA, dStats.stamina - dModStats.actionStamina);
  notify_dodge(attacker, defender);
}

void
notify_dodge(dbref attacker, dbref defender)
{
  static char msg[BUFFER_LEN];
  char *bp = msg;
  safe_format(msg, &bp, "%sGAME:%s %s%s%s%s attacks %s%s%s%s but they dodge.%s",
              ANSI_HIGREEN, ANSI_END, ANSI_HIWHITE, Name(attacker), ANSI_END,
              ANSI_CYAN, ANSI_HIWHITE, Name(defender), ANSI_END, ANSI_CYAN,
              ANSI_ENDALL);
  notify_except(attacker, Location(attacker), attacker, msg, 0);
  notify_format(attacker, "%sGAME: %s%sYou attack %s%s%s%s but they dodge.%s",
                ANSI_HIGREEN, ANSI_END, ANSI_CYAN, ANSI_HIWHITE, Name(defender),
                ANSI_END, ANSI_CYAN, ANSI_ENDALL);
  *bp = '\0';
}

void
notify_hit(dbref attacker, dbref defender)
{
  static char msg[BUFFER_LEN];
  char *bp = msg;
  safe_format(
    msg, &bp, "%sGAME:%s %s%s%s%s attacks %s%s%s%s and scores a hit.%s",
    ANSI_HIGREEN, ANSI_END, ANSI_HIWHITE, Name(attacker), ANSI_END, ANSI_CYAN,
    ANSI_HIWHITE, Name(defender), ANSI_END, ANSI_CYAN, ANSI_ENDALL);
  notify_except(attacker, Location(attacker), attacker, msg, 0);
  notify_format(attacker, "%sGAME: %s%sYou attack %s%s%s%s and you hit.%s",
                ANSI_HIGREEN, ANSI_END, ANSI_CYAN, ANSI_HIWHITE, Name(defender),
                ANSI_END, ANSI_CYAN, ANSI_ENDALL);
  *bp = '\0';
}

void
notify_miss(dbref attacker, dbref defender)
{
  static char msg[BUFFER_LEN];
  char *bp = msg;
  safe_format(msg, &bp, "%sGAME:%s %s%s%s%s attacks %s%s%s%s but misses.%s",
              ANSI_HIGREEN, ANSI_END, ANSI_HIWHITE, Name(attacker), ANSI_END,
              ANSI_CYAN, ANSI_HIWHITE, Name(defender), ANSI_END, ANSI_CYAN,
              ANSI_ENDALL);
  notify_except(attacker, Location(attacker), attacker, msg, 0);
  notify_format(attacker, "%sGAME: %s%sYou attack %s%s%s%s but you miss.%s",
                ANSI_HIGREEN, ANSI_END, ANSI_CYAN, ANSI_HIWHITE, Name(defender),
                ANSI_END, ANSI_CYAN, ANSI_ENDALL);
  *bp = '\0';
}