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
    int Health;
    int MaxHealth;
    int Stamina;
    int MaxStamina;
    int AttackSkill;
    int DodgeSkill;
    int Damage;
    dbref EquippedBy;
} combatStats;

extern void do_combat_iterate();
extern void initCombat();
extern void setupCombatFlags();
extern void setupCombatPowers();
extern void setupCombatCmds();
extern void setupCombatFunc();
extern void setupCombatAttr();
extern combatStats getStats(dbref Obj);
extern bool isCombatItem(dbref obj);
extern dbref equippedBy(dbref obj);
extern combatStats getModifiedStats(dbref player);
extern bool do_attack(dbref attacker, dbref defender);
extern bool do_defend(dbref attacker, dbref defender);
extern char * getAtrValue(dbref obj, char * name);


#endif // __COMBAT_H


