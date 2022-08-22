#ifndef __COMBAT_H
#define __COMBAT_H
#include <ctype.h>
#include <string.h>
#include <math.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "copyrite.h"
#include "compile.h"
#include "conf.h"
#include "externs.h"
#include "ansi.h"
#include "intrface.h"
#include "strutil.h"
#include "parse.h"
#include "dbdefs.h"
#include "flags.h"
#include "match.h"
#include "attrib.h"
#include "log.h"
#include "extchat.h"
#include "lock.h"
#include "game.h"
#include "function.h"
#include "funs.h"
#include "mushdb.h"
#include "command.h"
#include "intmap.h"
#include "version.h"
#include "mymalloc.h"
#include "notify.h"

struct combStats {
  int health;
  int maxHealth;
  int stamina;
  int maxStamina;
  int attackSkill;
  int dodgeSkill;
  int damage;
  dbref equippedBy;
  int attackSpeed;
  int nextAttackTime;
  int actionStamina;
};

typedef struct combStats combatStats;

typedef union combat_attrib_table {
  struct {
    char *health;
    char *max_health;
    char *stamina;
    char *max_stamina;
    char *damage;
    char *bleed;
    char *equippedby;
    char *next_attack_time;
    char *attack_speed;
    char *skill_attack;
    char *skill_dodge;
    char *action_stamina;
    char *ohit;
    char *hit;
    char *vhit;
    char *omiss;
    char *miss;
    char *vmiss;
    char *ododge;
    char *dodge;
    char *vdodge;
    char *oblock;
    char *block;
    char *vblock;
    char *equip;
    char *oequip;
    char *unequip;
    char *ounequip;
  };
} CATRTAB;

// ItemTypes
// Flag to indicate this item is armor.
#define CF_ARMOR "ARMOR"
// FLag to indicate this item is a weapon.
#define CF_WEAPON "WEAPON"
#define cformat(str)                                                           \
  safe_format(msg, &bp, "%sGAME:%s %s", ANSI_HIGREEN, ANSI_END, str)

// actions
#define CA_HIT 0
#define CA_MISS 1
#define CA_DODGE 2 // move out of the way of the attack
#define CA_BLOCK 3 // Use weapon to block attack

// Main Loop
extern void do_combat_iterate();

// Setup
extern void initCombat();
extern void setupCombatFlags();
extern void setupCombatPowers();
extern void setupCombatCmds();
extern void setupCombatFunc();
extern void setupCombatAttr();
extern void setupDefaultsOrConfig();

// Attrib Helpers
extern combatStats getStats(dbref Obj);
extern combatStats getModifiedStats(dbref player);
extern char *getAtrValue(dbref obj, char *name);
extern bool setAtrValue(dbref player, char *attr, int value);

// Item Helpers
extern bool isCombatItem(dbref obj);
extern dbref equippedBy(dbref obj);
extern dbref equippedWeapon(dbref player);
// Combat do things
extern void do_attack(dbref attacker, dbref defender);
extern void do_defend(dbref attacker, dbref defender);

// Announce

extern void notify_combat(dbref attacker, dbref defender, int state,
                          dbref equipped);
extern char *cmessage_format(char *message);
extern char *get_eval_attr(dbref obj, char *atr, dbref attacker,
                           dbref defender);
extern char *format_combat(char *msg);

#endif // __COMBAT_H
