#include "config.h"
#include "combat.h"

CATRTAB cattr = {.health = "COMBAT`STATS`HEALTH",
                 .max_health = "COMBAT`STATS`HEALTH`MAX",
                 .stamina = "COMBAT`STATS`STAMINA",
                 .max_stamina = "COMBAT`STATS`STAMINA`MAX",
                 .action_stamina = "COMBAT`STATS`STAMINA`ACTIONS",
                 .bleed = "COMBAT`STATS`BLEED",
                 .equippedby = "COMBAT`EQUIPPEDBY",
                 .next_attack_time = "COMBAT`STATS`NEXTATTACKTIME",
                 .attack_speed = "COMBAT`STATS`ATTACKSPEED",
                 .skill_attack = "COMBAT`SKILLS`ATTACK",
                 .skill_dodge = "COMBAT`SKILLS`DODGE",
                 .ohit = "COMBAT`MESSAGES`OTHER`HIT",
                 .hit = "COMBAT`MESSAGES`ATTACKER`HIT",
                 .vhit = "COMBAT`MESSAGES`VICTIM`HIT",
                 .omiss = "COMBAT`MESSAGES`OTHER`MISS",
                 .miss = "COMBAT`MESSAGES`ATTACKER`MISS",
                 .vmiss = "COMBAT`MESSAGES`VICTIM`MISS",
                 .ododge = "COMBAT`MESSAGES`OTHER`DODGE",
                 .dodge = "COMBAT`MESSAGES`ATTACKER`DODGE",
                 .vdodge = "COMBAT`MESSAGES`VICTIM`DODGE",
                 .oblock = "COMBAT`MESSAGES`OTHER`BLOCK",
                 .block = "COMBAT`MESSAGES`ATTACKER`BLOCK",
                 .vblock = "COMBAT`MESSAGES`VICTIM`BLOCK",
                 .equip = "COMBAT`MESSAGES`EQUIP",
                 .oequip = "COMBAT`MESSAGES`OEQUIP",
                 .unequip = "COMBAT`MESSAGES`UNEQUIP",
                 .ounequip = "COMBAT`MESSAGES`OUNEQUIP"};

char *oAction[4];
char *action[4];
char *vAction[4];

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

  dbref weapon = equippedWeapon(executor);

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

    notify_combat(executor, target, CA_MISS, weapon);
  }

  // Regardless of what happened, assign next attack time
  setAtrValue(executor, cattr.next_attack_time, (timeNow + aStats.attackSpeed));
  // Decrease attacker stamina
  setAtrValue(executor, cattr.stamina,
              aStats.stamina - aModStats.actionStamina);

  return;
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

COMMAND(cmd_local_equip)
{

  // Should only check the players inventory for the items, no need for
  // additional checks.
  dbref target =
    noisy_match_result(executor, arg_left, TYPE_THING, MAT_POSSESSION);

  // Verify it's a real object.
  if (!GoodObject(target)) {
    notify(executor, format_combat("cannot find item."));
    return;
  }
  // Is it combat flagged?
  if (!isCombatItem(target)) {
    notify(executor, format_combat("You can't use that item."));
    return;
  }
  // Check if the person already has it equipped. We don't care if someone
  // else's dbref is there because that means something went wrong.
  if (parse_dbref(getAtrValue(target, cattr.equippedby)) == executor) {
    notify(executor, format_combat("You're already using that item."));
    return;
  }

  do_lock(GOD, unparse_dbref(target), "#FALSE", "DROP");
  do_set_atr(target, cattr.equippedby, unparse_dbref(executor), GOD, 0);
  notify_except(executor, Location(executor), executor,
                get_eval_attr(target, cattr.oequip, executor, -1), 0);
  notify(executor, get_eval_attr(target, cattr.equip, executor, -1));
}
COMMAND(cmd_local_unequip)
{
  // Should only check the players inventory for the items, no need for
  // additional checks.
  dbref target =
    noisy_match_result(executor, arg_left, TYPE_THING, MAT_POSSESSION);

  if (!GoodObject(target)) {
    notify(executor, format_combat("cannot find item."));
    return;
  }
  // Is it combat flagged?
  if (!isCombatItem(target)) {
    notify(executor, format_combat("You can't use that item."));
    return;
  }
  // Check if the person already has it equipped. This is the one we care about.
  if (parse_dbref(getAtrValue(target, cattr.equippedby)) == executor) {
    do_lock(GOD, unparse_dbref(target), NULL, "DROP");
    atr_clr(target, cattr.equippedby, GOD);
    notify_except(executor, Location(executor), executor,
                  get_eval_attr(target, cattr.ounequip, executor, -1), 0);
    notify(executor, get_eval_attr(target, cattr.unequip, executor, -1));
  } else
  {
    notify(executor, format_combat("You're not using that."));
  }
}

FUNCTION(local_fun_attack) { safe_format(buff, bp, "Attack%sAttack", args[0]); }

// Approximately every 250ms, handle combat updates.
void
do_combat_iterate()
{
  // ~250ms
  // Update Stamina
  // - increase stamina by a point or so
  // - increase stamina by bonuses on equipped items
  // Update Health
  // - increase health by a miniscule amount regardless
  // - increase health by bonuses on equipped items.
  // Update Bleed
  // - decrease health by the Bleeding attribute.
  // - decrease bleeding amount by a point or so.
  // Update Unconscious
  // - If a player's health has dropped below 10% of their maxhealth, and they
  // are not UNCONSCIOUS.
  // --  Set them UNCONSCIOUS
  // --  Notify them they have fallen unconscious
  // --  UNCONSCIOUS players should not be able to perform any actions
  // - If they are unconscious and their health has risen above 10%,
  // --  set them !UNCONSCIOUS.
  // --  Notify them they have reawakened.
  // Update Death
  // - If a player's health has dropped to <= 0,
  // --  set their health to 0 and set them DEAD
  // --  Notify the player that they have died.
  // --  Increment their death count
  // --  @boot the player from the game.
  // --  DEAD players should not be able to perform any actions until they are
  // respawned or whatever.
}

void
initCombat()
{

  setupDefaultsOrConfig();

  setupCombatFlags();
  setupCombatPowers();
  setupCombatCmds();
  setupCombatFunc();
  setupCombatAttr();
}

void
setupDefaultsOrConfig()
{
  dbref conf = options.combat_config;
  if (GoodObject(conf)) {

    cattr.health = getAtrValue(conf, cattr.health);
    cattr.max_health = getAtrValue(conf, cattr.max_health);
    cattr.stamina = getAtrValue(conf, cattr.stamina);
    cattr.max_stamina = getAtrValue(conf, cattr.max_stamina);
    cattr.action_stamina = getAtrValue(conf, cattr.action_stamina);
    cattr.bleed = getAtrValue(conf, cattr.bleed);
    cattr.equippedby = getAtrValue(conf, cattr.equippedby);
    cattr.next_attack_time = getAtrValue(conf, cattr.next_attack_time);
    cattr.attack_speed = getAtrValue(conf, cattr.attack_speed);
    cattr.skill_attack = getAtrValue(conf, cattr.skill_attack);
    cattr.skill_dodge = getAtrValue(conf, cattr.skill_dodge);
    cattr.ohit = getAtrValue(conf, cattr.ohit);
    cattr.hit = getAtrValue(conf, cattr.hit);
    cattr.vhit = getAtrValue(conf, cattr.vhit);
    cattr.omiss = getAtrValue(conf, cattr.omiss);
    cattr.miss = getAtrValue(conf, cattr.miss);
    cattr.vmiss = getAtrValue(conf, cattr.vmiss);
    cattr.ododge = getAtrValue(conf, cattr.ododge);
    cattr.dodge = getAtrValue(conf, cattr.dodge);
    cattr.vdodge = getAtrValue(conf, cattr.vdodge);
    cattr.oblock = getAtrValue(conf, cattr.oblock);
    cattr.block = getAtrValue(conf, cattr.block);
    cattr.vblock = getAtrValue(conf, cattr.vblock);
    cattr.equip = getAtrValue(conf, cattr.equip);
    cattr.oequip = getAtrValue(conf, cattr.oequip);
    cattr.unequip = getAtrValue(conf, cattr.unequip);
    cattr.ounequip = getAtrValue(conf, cattr.ounequip);
  }

  oAction[0] = cattr.ohit;
  oAction[1] = cattr.omiss;
  oAction[2] = cattr.ododge;
  oAction[3] = cattr.oblock;
  action[0] = cattr.hit;
  action[1] = cattr.miss;
  action[2] = cattr.dodge;
  action[3] = cattr.block;
  vAction[0] = cattr.vhit;
  vAction[1] = cattr.vmiss;
  vAction[2] = cattr.vdodge;
  vAction[3] = cattr.vblock;
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
    "WEAPON", '*', TYPE_THING, F_ANY,
    F_ANY); // signifies an object that provides attack damage and stamina
  add_flag("ARMOR", 'a', TYPE_THING, F_ANY,
           F_ANY); // Signifies an object that provides armor and stamina and
                   // defense bonuses.
  add_flag("BLEEDING", 'b', TYPE_PLAYER, F_ANY,
           F_ANY); // player will loose health each combat tick.
  add_flag("UNCONSCIOUS", 'u', TYPE_PLAYER, F_ANY,
           F_ANY); // player will be unable to move/speak/hear/use commands.
                   // Will eventually recover over time
  add_flag("DEAD", 'd', TYPE_PLAYER, F_ANY,
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

  add_new_attr(cattr.health, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.max_health, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.stamina, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.max_stamina, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.damage, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.bleed, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.equippedby, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.next_attack_time, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.attack_speed, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.skill_attack, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.skill_dodge, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.action_stamina, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.ohit, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.hit, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.vhit, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.omiss, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.miss, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.vmiss, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.ododge, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.dodge, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.vdodge, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.oblock, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.block, AF_WIZARD & AF_VEILED);
  add_new_attr(cattr.vblock, AF_WIZARD & AF_VEILED);

  alias_attribute("Health", "Life");
}

combatStats
getStats(dbref obj)
{
  combatStats cstat;

  cstat.health = parse_number(getAtrValue(obj, cattr.health));
  cstat.maxHealth = parse_number(getAtrValue(obj, cattr.max_health));
  cstat.stamina = parse_number(getAtrValue(obj, cattr.stamina));
  cstat.maxStamina = parse_number(getAtrValue(obj, cattr.max_stamina));
  cstat.attackSkill = parse_number(getAtrValue(obj, cattr.skill_attack));
  cstat.dodgeSkill = parse_number(getAtrValue(obj, cattr.skill_dodge));
  cstat.equippedBy = parse_number(getAtrValue(obj, cattr.equippedby));
  cstat.damage = parse_number(getAtrValue(obj, cattr.damage));

  cstat.attackSpeed = parse_number(getAtrValue(obj, cattr.attack_speed));
  cstat.nextAttackTime = parse_number(getAtrValue(obj, cattr.next_attack_time));
  cstat.actionStamina = parse_number(getAtrValue(obj, cattr.action_stamina));

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
  if (name) {
    ATTR *a = atr_get(obj, name);
    if (!a)
      return "0";

    return atr_value(a);
  }
  return "0";
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
  return parse_dbref(getAtrValue(obj, cattr.equippedby));
}

dbref
equippedWeapon(dbref player)
{
  dbref invItem;
  DOLIST (invItem, Contents(player)) {
    if (has_flag_by_name(invItem, CF_WEAPON, NOTYPE) &&
        equippedBy(invItem) == player) {
      return invItem;
    }
  }

  return -1;
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
  setAtrValue(defender, cattr.health, dStats.health - aModStats.damage);
  setAtrValue(defender, cattr.stamina,
              dStats.stamina - dModStats.actionStamina);
  notify_combat(attacker, defender, CA_HIT, equippedWeapon(attacker));
}

void
do_defend(dbref attacker, dbref defender)
{
  combatStats dStats = getStats(defender);
  combatStats dModStats = getModifiedStats(defender);
  setAtrValue(defender, cattr.stamina,
              dStats.stamina - dModStats.actionStamina);
  notify_combat(attacker, defender, CA_DODGE, equippedWeapon(attacker));
}

void
notify_combat(dbref attacker, dbref defender, int state, dbref equipped)
{
  dbref invoke = attacker;
  if (GoodObject(equipped))
    invoke = equipped;
  notify_except2(attacker, Location(attacker), attacker, defender,
                 get_eval_attr(invoke, oAction[state], attacker, defender), 0);
  notify(attacker, get_eval_attr(invoke, action[state], attacker, defender));
  notify(defender, get_eval_attr(invoke, vAction[state], attacker, defender));
}

char *
get_eval_attr(dbref obj, char *atr, dbref attacker, dbref defender)
{
  ufun_attrib ufun;
  NEW_PE_INFO *pe_info;
  PE_REGS *pe_regs = NULL;
  pe_info = make_pe_info("pe_info-atr_ufun_cbt");

  char tbuf1[BUFFER_LEN];
  *tbuf1 = '\0';

  if (fetch_ufun_attrib(atr, obj, &ufun, UFUN_IGNORE_PERMS)) {

    pe_regs = pe_regs_create(PE_REGS_ARG, "cmd_attack");

    pe_regs_setenv(pe_regs, 0, unparse_dbref(attacker)); // %0 is the attacker
    pe_regs_setenv(pe_regs, 1, unparse_dbref(defender)); // %1 is the victim
    pi_regs_setq(pe_info, "A", unparse_dbref(attacker)); // %qA is the attacker
    pi_regs_setq(pe_info, "V", unparse_dbref(defender)); // %qV is the victim

    // %qD damage
    // %qB bleed
    // %qP poison
    // %qS extra stamina loss?

    // call_attrib(attacker, CO_ATTACKMSG, tbuf1, attacker, pe_info, pe_regs);
    call_ufun(&ufun, tbuf1, obj, obj, pe_info, pe_regs);

    free_pe_info(pe_info);
    free_pe_regs_trees(pe_regs);

    //

    // return msg;
  }

  return format_combat(tbuf1);
}
char *
format_combat(char *msg)
{
  char tbuf1[BUFFER_LEN];
  *tbuf1 = '\0';
  char *message = mush_malloc(sizeof(BUFFERQ), "bufferq");
  char *bp = message;
  *bp = '\0';

  safe_format(message, &bp, "%sGAME:%s %s%s%s", ANSI_HIGREEN, ANSI_END, ANSI_CYAN, msg, ANSI_ENDALL);
  return message;
}
