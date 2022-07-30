#include "config.h"
#include "combat.h"



COMMAND(cmd_local_attack)
{
  if (SW_ISSET(sw, SWITCH_NOISY))
    notify_format(executor, "Noisy attack with %s", arg_left);
  if (SW_BY_NAME(sw, "VERY"))
    notify(executor, "The following line will be very attack indeed.");
  notify_format(executor, "AttackCommand %s", arg_left);
  
    dbref target = noisy_match_result(executor, arg_left, TYPE_PLAYER, MAT_EVERYTHING);
  
    
    if (target == NOTHING)
    {
        // notify the player
        return;
    }
    combatStats aStats = getStats(executor);
    if(aStats.Stamina < 1)
    {
        // notify the player
        return;
    }

    aStats = getModifiedStats(executor);

    combatStats tStats = getModifiedStats(target);



    
/*
 
 - Determine Defender dbref - Done
   - Get his health, and stamina, and dodge skill - done

   - Check his contents for equipped armor/weapons and increment/decrement his health/stamina/dodge for each item that affects it. - done
  - Determine Attacker's dbref - done
    - if stamina is zero, call EXIT -done
   - Get his health, stamina, and attack skill-done 
   - Check his contents for equipped armor/weapons and increment/decrement his health/stamina/dodge for each item that affects it. - done

ATTACK: 
  - pick a random number between 0 and 100 for the attacker's roll.
  - Get the attacker's attack skill
    - add in any attack bonuses and subtract any attack penalties provided by weapons/armor, but limit between 1 and 99.
  - IF attack skill plus bonuses is greater than the random roll, call DEFEND
    - ELSE call MISS

DEFEND:
  - pick a random number between 1 and 99 for the defender's roll.
  - Get the defender's dodge skill
    - add in any dodge bonuses and subtract any dodge penalties provided by weapons/armor, but limit between 1 and 99.
    - if stamina is zero, call HIT
 - IF dodge skill plus bonuses is greater than random roll, call DODGE
 - ELSE call HIT  

MISS:
  - decrement attacker stamina
  - Display miss messages for weapon
DODGE:
  - decrement attacker stamina
  - decrement defender stamina
  - Display dodge messages for weapon
HIT:
 - decrement attacker stamina
  - display hit messages for weapon
EXIT:






  


 */

}


COMMAND(cmd_local_equip)
{

}
COMMAND(cmd_local_unequip)
{

}


FUNCTION(local_fun_attack) { safe_format(buff, bp, "Attack%sAttack", args[0]); }




// ~250ms
void do_combat_iterate() 
{
    // Update Health
    // Update Bleed
    // Undate Unconscious
    // Update Stamina

}

void initCombat()
{
    setupCombatFlags();
    setupCombatPowers();
    setupCombatCmds();
    setupCombatFunc();
    setupCombatAttr();

}


void setupCombatCmds()
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
void setupCombatFlags()
{
 
  add_flag("WEAPON", "*", TYPE_THING, F_ANY, F_ANY); // signifies an object that provides attack damage and stamina 
  add_flag("ARMOR", "A", TYPE_THING, F_ANY, F_ANY); //Signifies an object that provides armor and stamina and defense bonuses.
  add_flag("BLEEDING", "b", TYPE_PLAYER, F_ANY, F_ANY); // player will loose health each combat tick.
  add_flag("UNCONSCIOUS", "b", TYPE_PLAYER, F_ANY, F_ANY);  // player will be unable to move/speak/hear/use commands. Will eventually recover over time
  add_flag("DEAD", "D", TYPE_PLAYER, F_ANY, F_ANY); // player will be unable to move/speak/hear/use commands.  Will not recover.
  

}

void setupCombatPowers()
{
   #ifdef EXAMPLE
   add_power("CanUseAirlock",1, TYPE_PLAYER, F_ANY, F_ANY);
   #endif
}

void setupCombatFunc()
{

  function_add("ATTACK", local_fun_attack, 1, 1, FN_REG);
}

void setupCombatAttr()
{

    add_new_attr("Health",AF_WIZARD); // The current health of the player/armor/weapon
    add_new_attr("MaxHealth",AF_WIZARD); // The most health a player can heal up to or armor/weapon can be repaired to.
    add_new_attr("Stamina",AF_WIZARD); // The current stamina of the player or the bonus/penalty to player stamina caused by the armor/weapon
    add_new_attr("MaxStamina",AF_WIZARD); // The most stamina a player can recover, or an bonus/penalty to base stamina caused by armor/weapon
    
    add_new_attr("Skill`Attack",AF_WIZARD); // How likely the player is to land a hit.  For weapon or armor, bonus or penalty to player base stat
    add_new_attr("Skill`Dodge",AF_WIZARD); // How likely the player is to dodge an attack.  For weapon or armor, bonus or penalty to player base stat.
    add_new_attr("Damage",AF_WIZARD); // How much damage the weapon (or player) can dish out
    add_new_attr("EquippedBy",AF_WIZARD); //dbref of player using weapon/armor

    add_new_attr("NextAttackTime",AF_WIZARD & AF_INTERNAL); //TImestamp at which time the next attack can be performed
    add_new_attr("AttackSpeed", AF_WIZARD); //number of seconds to increment current timestamp when performing an attack.
    
    alias_attribute("Health","Life");

}


combatStats getStats(dbref obj)
{
        combatStats cstat;
        cstat.Health = parse_number(atr_value(atr_get(obj, "Health")));
        cstat.MaxHealth = parse_number(atr_value(atr_get(obj, "MaxHealth")));
        cstat.Stamina = parse_number(atr_value(atr_get(obj, "Stamina")));
        cstat.MaxStamina = parse_number(atr_value(atr_get(obj, "MaxStamina")));
        cstat.AttackSkill = parse_number(atr_value(atr_get(obj, "Skill`Attack")));
        cstat.DodgeSkill = parse_number(atr_value(atr_get(obj, "Skill`Dodge")));
        cstat.EquippedBy =parse_dbref(atr_value(atr_get(obj,"EquippedBy")));
        cstat.Damage = parse_number(atr_value(atr_get(obj, "Damage")));
    

    return cstat;
}

bool isCombatItem(dbref obj)
{
    return (has_flag_by_name(obj,"ARMOR",NOTYPE) || has_flag_by_name(obj,"ARMOR",NOTYPE)) && IsThing(obj);
}

dbref equippedBy(dbref obj)
{
    return parse_dbref(atr_value(atr_get(obj,"EquippedBy")));
}

combatStats getModifiedStats(dbref player)
{
        combatStats pStats = getStats(player);
        dbref invItem = NOTHING;
        DOLIST (invItem, Contents(player)) {
        combatStats oStats = getStats(invItem);
        if(isCombatItem(invItem) && equippedBy(invItem) == player)
        {
            pStats.Health += oStats.Health;
            pStats.Stamina += oStats.Stamina;
            pStats.AttackSkill += oStats.AttackSkill;
            pStats.DodgeSkill += oStats.DodgeSkill;
            pStats.Damage = oStats.Damage;
        }        
    }
    return pStats;
}

bool do_attack(dbref attacker, dbref defender)
{

    return false;
}

bool do_defend(debref attacker, dbref defender)
{
    return false;
}