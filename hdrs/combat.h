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
#include "confmagic.h"
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

typedef struct
{
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
} combatStats;

#define CE_HEALTH "HEALTH"
#define CE_MAXHEALTH "MAXHEALTH"
#define CE_STAMINA "STAMINA"
#define CE_MAXSTAMINA "MAXSTAMINA"
#define CE_DAMAGE "DAMAGE"
#define CE_SKILLATTACK "SKILL`ATTACK"
#define CE_SKILLDODGE "SKILL`DODGE"
#define CE_BLEED "BLEED"
#define CE_EQUIPPEDBY "EQUIPPEDBY"
#define CE_NEXTATTACKTIME "NEXTATTACKTIME"
#define CE_ATTACKSPEED "ATTACKSPEED"
#define CE_ACTIONSTAMINA "ACTIONSTAMINA"

#define CF_ARMOR "ARMOR"
#define CF_WEAPON "WEAPON"

// Main Loop
extern void do_combat_iterate();

// Setup
extern void initCombat();
extern void setupCombatFlags();
extern void setupCombatPowers();
extern void setupCombatCmds();
extern void setupCombatFunc();
extern void setupCombatAttr();

// Attrib Helpers
extern combatStats getStats(dbref Obj);
extern combatStats getModifiedStats(dbref player);
extern char * getAtrValue(dbref obj, char * name);
extern bool setAtrValue(dbref player, char *attr, int value);

// Item Helpers
extern bool isCombatItem(dbref obj);
extern dbref equippedBy(dbref obj);

// Combat do things
extern void do_attack(dbref attacker, dbref defender);
extern void do_defend(dbref attacker, dbref defender);

// Announce
extern void notify_dodge(dbref attacker, dbref defender);
extern void notify_hit(dbref attacker, dbref defender);
extern void notify_miss(dbref attacker, dbref defender);


#endif // __COMBAT_H


