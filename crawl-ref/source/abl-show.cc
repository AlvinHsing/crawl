/*
 *  File:       abl-show.cc
 *  Summary:    Functions related to special abilities.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *  <6>    19mar2000     jmf    added elvish Glamour
 *  <5>     11/06/99     cdl    reduced power of minor destruction
 *
 *  <4>      9/25/99     cdl    linuxlib -> liblinux
 *
 *  <3>      5/20/99     BWR    Now use scan_randarts to
 *                              check for flags, rather than
 *                              only checking the weapon.
 *
 *  <2>      5/20/99     BWR    Extended screen line support
 *
 *  <1>      -/--/--     LRH             Created
 */

#include "AppHdr.h"
#include "abl-show.h"

#include <sstream>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "beam.h"
#include "effects.h"
#include "food.h"
#include "it_use2.h"
#include "macro.h"
#include "message.h"
#include "misc.h"
#include "mon-util.h"
#include "monplace.h"
#include "monstuff.h"
#include "notes.h"
#include "ouch.h"
#include "player.h"
#include "religion.h"
#include "skills.h"
#include "skills2.h"
#include "spl-cast.h"
#include "spl-util.h"
#include "spells1.h"
#include "spells2.h"
#include "spells3.h"
#include "spells4.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"


#ifdef UNIX
#include "libunix.h"
#endif

// this all needs to be split into data/util/show files
// and the struct mechanism here needs to be rewritten (again)
// along with the display routine to piece the strings
// together dynamically ... I'm getting to it now {dlb}

// it makes more sense to think of them as an array
// of structs than two arrays that share common index
// values -- well, doesn't it? {dlb}
struct talent
{
    int which;
    int fail;
    bool is_invocation;
};

static FixedVector< talent, 52 >  Curr_abil;

static bool insert_ability( int which_ability, bool check_conf );
static void lugonu_bends_space();

// declaring this const messes up externs later, so don't do it
ability_type god_abilities[MAX_NUM_GODS][MAX_GOD_ABILITIES] =
{
    // no god
    { ABIL_NON_ABILITY, ABIL_NON_ABILITY, ABIL_NON_ABILITY,
      ABIL_NON_ABILITY, ABIL_NON_ABILITY },
    // Zin
    { ABIL_ZIN_REPEL_UNDEAD, ABIL_ZIN_HEALING, ABIL_ZIN_PESTILENCE,
      ABIL_ZIN_HOLY_WORD, ABIL_ZIN_SUMMON_GUARDIAN },
    // TSO
    { ABIL_TSO_REPEL_UNDEAD, ABIL_TSO_SMITING, ABIL_TSO_ANNIHILATE_UNDEAD,
      ABIL_TSO_CLEANSING_FLAME, ABIL_TSO_SUMMON_DAEVA },
    // Kikubaaqudgha
    { ABIL_KIKU_RECALL_UNDEAD_SLAVES, ABIL_NON_ABILITY,
      ABIL_KIKU_ENSLAVE_UNDEAD, ABIL_NON_ABILITY,
      ABIL_KIKU_INVOKE_DEATH },
    // Yredelemnul
    { ABIL_YRED_ANIMATE_CORPSE, ABIL_YRED_RECALL_UNDEAD,
      ABIL_YRED_ANIMATE_DEAD, ABIL_YRED_DRAIN_LIFE,
      ABIL_YRED_CONTROL_UNDEAD },
    // Xom
    { ABIL_NON_ABILITY, ABIL_NON_ABILITY, ABIL_NON_ABILITY,
      ABIL_NON_ABILITY, ABIL_NON_ABILITY },
    // Vehumet
    { ABIL_NON_ABILITY, ABIL_NON_ABILITY, ABIL_NON_ABILITY,
      ABIL_NON_ABILITY, ABIL_NON_ABILITY },
    // Okawaru
    { ABIL_OKAWARU_MIGHT, ABIL_OKAWARU_HEALING, ABIL_NON_ABILITY,
      ABIL_NON_ABILITY, ABIL_OKAWARU_HASTE },
    // Makhleb
    { ABIL_NON_ABILITY, ABIL_MAKHLEB_MINOR_DESTRUCTION,
      ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB,
      ABIL_MAKHLEB_MAJOR_DESTRUCTION,
      ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB },
    // Sif Muna
    { ABIL_SIF_MUNA_CHANNEL_ENERGY,
      ABIL_SIF_MUNA_FORGET_SPELL, ABIL_NON_ABILITY,
      ABIL_NON_ABILITY, ABIL_NON_ABILITY },
    // Trog
    { ABIL_TROG_BERSERK, ABIL_TROG_MIGHT, ABIL_NON_ABILITY,
      ABIL_TROG_HASTE_SELF, ABIL_NON_ABILITY },
    // Nemelex
    { ABIL_NON_ABILITY, ABIL_NON_ABILITY, ABIL_NON_ABILITY,
      ABIL_NON_ABILITY, ABIL_NON_ABILITY },
    // Elyvilon
    { ABIL_ELYVILON_LESSER_HEALING, ABIL_ELYVILON_PURIFICATION,
      ABIL_ELYVILON_HEALING, ABIL_ELYVILON_RESTORATION,
      ABIL_ELYVILON_GREATER_HEALING },
    // Lugonu
    { ABIL_LUGONU_ABYSS_EXIT, ABIL_LUGONU_BEND_SPACE,
      ABIL_LUGONU_SUMMON_DEMONS, ABIL_NON_ABILITY,
      ABIL_LUGONU_ABYSS_ENTER }
};

// The description screen was way out of date with the actual costs.
// This table puts all the information in one place... -- bwr
// 
// The four numerical fields are: MP, HP, food, and piety.
// Note:  food_cost  = val + random2avg( val, 2 )
//        piety_cost = val + random2( (val + 1) / 2 + 1 );
static const struct ability_def Ability_List[] =
{
    // NON_ABILITY should always come first
    { ABIL_NON_ABILITY, "No ability", 0, 0, 0, 0, ABFLAG_NONE },
    { ABIL_SPIT_POISON, "Spit Poison", 0, 0, 40, 0, ABFLAG_BREATH },
    { ABIL_GLAMOUR, "Glamour", 5, 0, 40, 0, ABFLAG_DELAY },

    { ABIL_MAPPING, "Sense Surroundings", 0, 0, 30, 0, ABFLAG_NONE },
    { ABIL_TELEPORTATION, "Teleportation", 3, 0, 200, 0, ABFLAG_NONE },
    { ABIL_BLINK, "Blink", 1, 0, 50, 0, ABFLAG_NONE },

    { ABIL_BREATHE_FIRE, "Breathe Fire", 0, 0, 125, 0, ABFLAG_BREATH },
    { ABIL_BREATHE_FROST, "Breathe Frost", 0, 0, 125, 0, ABFLAG_BREATH },
    { ABIL_BREATHE_POISON, "Breathe Poison Gas", 0, 0, 125, 0, ABFLAG_BREATH },
    { ABIL_BREATHE_LIGHTNING, "Breathe Lightning", 0, 0, 125, 0, ABFLAG_BREATH },
    { ABIL_BREATHE_POWER, "Breathe Power", 0, 0, 125, 0, ABFLAG_BREATH },
    { ABIL_BREATHE_STICKY_FLAME, "Breathe Sticky Flame", 0, 0, 125, 0, ABFLAG_BREATH },
    { ABIL_BREATHE_STEAM, "Breathe Steam", 0, 0, 75, 0, ABFLAG_BREATH },

    { ABIL_SPIT_ACID, "Spit Acid", 0, 0, 125, 0, ABFLAG_BREATH },

    { ABIL_FLY, "Fly", 3, 0, 100, 0, ABFLAG_NONE },
    { ABIL_SUMMON_MINOR_DEMON, "Summon Minor Demon", 3, 3, 75, 0, ABFLAG_NONE },
    { ABIL_SUMMON_DEMONS, "Summon Demons", 5, 5, 150, 0, ABFLAG_NONE },
    { ABIL_HELLFIRE, "Hellfire", 8, 8, 200, 0, ABFLAG_NONE },
    { ABIL_TORMENT, "Torment", 9, 0, 250, 0, ABFLAG_PAIN },
    { ABIL_RAISE_DEAD, "Raise Dead", 5, 5, 150, 0, ABFLAG_NONE },
    { ABIL_CONTROL_DEMON, "Control Demon", 4, 4, 100, 0, ABFLAG_NONE },
    { ABIL_TO_PANDEMONIUM, "Gate Yourself to Pandemonium", 7, 0, 200, 0, ABFLAG_NONE },
    { ABIL_CHANNELING, "Channeling", 1, 0, 30, 0, ABFLAG_NONE },
    { ABIL_THROW_FLAME, "Throw Flame", 1, 1, 50, 0, ABFLAG_NONE },
    { ABIL_THROW_FROST, "Throw Frost", 1, 1, 50, 0, ABFLAG_NONE },
    { ABIL_BOLT_OF_DRAINING, "Bolt of Draining", 4, 4, 100, 0, ABFLAG_NONE },

    // FLY_II used to have ABFLAG_EXHAUSTION, but that's somewhat meaningless
    // as exhaustion's only (and designed) effect is preventing Berserk. -- bwr
    { ABIL_FLY_II, "Fly", 0, 0, 25, 0, ABFLAG_NONE },
    { ABIL_DELAYED_FIREBALL, "Release Delayed Fireball", 0, 0, 0, 0, ABFLAG_INSTANT },
    { ABIL_MUMMY_RESTORATION, "Restoration", 1, 0, 0, 0, ABFLAG_PERMANENT_MP },

    // EVOKE abilities use Evocations and come from items:
    // Mapping, Teleportation, and Blink can also come from mutations 
    // so we have to distinguish them (see above).  The off items 
    // below are labeled EVOKE because they only work now if the
    // player has an item with the evocable power (not just because
    // you used a wand, potion, or miscast effect).  I didn't see 
    // any reason to label them as "Evoke" in the text, they don't 
    // use or train Evocations (the others do).  -- bwr
    { ABIL_EVOKE_MAPPING, "Evoke Sense Surroundings", 0, 0, 30, 0, ABFLAG_NONE },
    { ABIL_EVOKE_TELEPORTATION, "Evoke Teleportation", 3, 0, 200, 0, ABFLAG_NONE },
    { ABIL_EVOKE_BLINK, "Evoke Blink", 1, 0, 50, 0, ABFLAG_NONE },

    { ABIL_EVOKE_BERSERK, "Evoke Berserk Rage", 0, 0, 0, 0, ABFLAG_NONE },

    { ABIL_EVOKE_TURN_INVISIBLE, "Evoke Invisibility", 2, 0, 250, 0, ABFLAG_NONE },
    { ABIL_EVOKE_TURN_VISIBLE, "Turn Visible", 0, 0, 0, 0, ABFLAG_NONE },
    { ABIL_EVOKE_LEVITATE, "Evoke Levitation", 1, 0, 100, 0, ABFLAG_NONE },
    { ABIL_EVOKE_STOP_LEVITATING, "Stop Levitating", 0, 0, 0, 0, ABFLAG_NONE },

    { ABIL_END_TRANSFORMATION, "End Transformation", 0, 0, 0, 0, ABFLAG_NONE },

    // INVOCATIONS:
    // Zin
    { ABIL_ZIN_REPEL_UNDEAD, "Repel Undead", 1, 0, 100, 0, ABFLAG_NONE },
    { ABIL_ZIN_HEALING, "Minor Healing", 2, 0, 50, 1, ABFLAG_NONE },
    { ABIL_ZIN_PESTILENCE, "Pestilence", 3, 0, 100, 2, ABFLAG_NONE },
    { ABIL_ZIN_HOLY_WORD, "Holy Word", 6, 0, 150, 3, ABFLAG_NONE },
    { ABIL_ZIN_SUMMON_GUARDIAN, "Summon Guardian", 7, 0, 150, 4, ABFLAG_NONE },

    // The Shining One
    { ABIL_TSO_REPEL_UNDEAD, "Repel Undead", 1, 0, 100, 0, ABFLAG_NONE },
    { ABIL_TSO_SMITING, "Smiting", 3, 0, 50, 2, ABFLAG_NONE },
    { ABIL_TSO_ANNIHILATE_UNDEAD, "Annihilate Undead", 3, 0, 50, 2, ABFLAG_NONE },
    { ABIL_TSO_CLEANSING_FLAME, "Cleansing Flame", 5, 0, 100, 2, ABFLAG_NONE },
    { ABIL_TSO_SUMMON_DAEVA, "Summon Daeva", 8, 0, 150, 4, ABFLAG_NONE },

    // Kikubaaqudgha
    { ABIL_KIKU_RECALL_UNDEAD_SLAVES, "Recall Undead Slaves", 2, 0, 50, 0, ABFLAG_NONE },
    { ABIL_KIKU_ENSLAVE_UNDEAD, "Enslave Undead", 4, 0, 150, 3, ABFLAG_NONE },
    { ABIL_KIKU_INVOKE_DEATH, "Invoke Death", 4, 0, 250, 3, ABFLAG_NONE },

    // Yredelemnul
    { ABIL_YRED_ANIMATE_CORPSE, "Animate Corpse", 1, 0, 50, 0, ABFLAG_NONE },
    { ABIL_YRED_RECALL_UNDEAD, "Recall Undead Slaves", 2, 0, 50, 0, ABFLAG_NONE },
    { ABIL_YRED_ANIMATE_DEAD, "Animate Dead", 3, 0, 100, 1, ABFLAG_NONE },
    { ABIL_YRED_DRAIN_LIFE, "Drain Life", 6, 0, 200, 2, ABFLAG_NONE },
    { ABIL_YRED_CONTROL_UNDEAD, "Control Undead", 5, 0, 150, 2, ABFLAG_NONE },

    // Okawaru
    { ABIL_OKAWARU_MIGHT, "Might", 2, 0, 50, 1, ABFLAG_NONE },
    { ABIL_OKAWARU_HEALING, "Healing", 2, 0, 75, 1, ABFLAG_NONE },
    { ABIL_OKAWARU_HASTE, "Haste", 5, 0, 100, 3, ABFLAG_NONE },

    // Makhleb
    { ABIL_MAKHLEB_MINOR_DESTRUCTION, "Minor Destruction", 1, 0, 20, 0, ABFLAG_NONE },
    { ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB, "Lesser Servant of Makhleb", 2, 0, 50, 1, ABFLAG_NONE },
    { ABIL_MAKHLEB_MAJOR_DESTRUCTION, "Major Destruction", 4, 0, 100, 2, ABFLAG_NONE },
    { ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB, "Greater Servant of Makhleb", 6, 0, 100, 3, ABFLAG_NONE },

    // Sif Muna
    { ABIL_SIF_MUNA_CHANNEL_ENERGY, "Channel Energy", 0, 0, 100, 0, ABFLAG_NONE },
    { ABIL_SIF_MUNA_FORGET_SPELL, "Forget Spell", 5, 0, 0, 8, ABFLAG_NONE },

    // Trog
    { ABIL_TROG_BERSERK, "Berserk", 0, 0, 200, 0, ABFLAG_NONE },
    { ABIL_TROG_MIGHT, "Might", 0, 0, 200, 1, ABFLAG_NONE },
    { ABIL_TROG_HASTE_SELF, "Haste Self", 0, 0, 250, 3, ABFLAG_NONE },

    // Elyvilon
    { ABIL_ELYVILON_LESSER_HEALING, "Lesser Healing", 1, 0, 100, 0,
      ABFLAG_CONF_OK },
    { ABIL_ELYVILON_PURIFICATION, "Purification", 2, 0, 150, 1,
      ABFLAG_CONF_OK },
    { ABIL_ELYVILON_HEALING, "Healing", 2, 0, 250, 2, ABFLAG_CONF_OK },
    { ABIL_ELYVILON_RESTORATION, "Restoration", 3, 0, 400, 3, ABFLAG_CONF_OK },
    { ABIL_ELYVILON_GREATER_HEALING, "Greater Healing", 6, 0, 600, 4,
      ABFLAG_CONF_OK },

    // Lugonu
    { ABIL_LUGONU_ABYSS_EXIT, "Depart the Abyss", 0, 0, 100, 10, ABFLAG_PAIN },
    { ABIL_LUGONU_BEND_SPACE, "Bend Space", 1, 0, 50, 0, ABFLAG_PAIN },
    { ABIL_LUGONU_SUMMON_DEMONS, "Summon Abyssal Servants", 7, 0, 100, 5, ABFLAG_NONE },
    { ABIL_LUGONU_ABYSS_ENTER, "Enter the Abyss", 9, 0, 200, 40, ABFLAG_NONE },

    // These six are unused "evil" god abilities:
    { ABIL_CHARM_SNAKE, "Charm Snake", 6, 0, 200, 5, ABFLAG_NONE },
    { ABIL_TRAN_SERPENT_OF_HELL, "Turn into Demonic Serpent", 16, 0, 600, 8, ABFLAG_NONE },
    { ABIL_BREATHE_HELLFIRE, "Breathe Hellfire", 0, 8, 200, 0, ABFLAG_BREATH },

    { ABIL_ROTTING, "Rotting", 4, 4, 0, 2, ABFLAG_NONE },
    { ABIL_TORMENT_II, "Call Torment", 9, 0, 0, 3, ABFLAG_PAIN },

    { ABIL_RENOUNCE_RELIGION, "Renounce Religion", 0, 0, 0, 0, ABFLAG_NONE },
};


const struct ability_def & get_ability_def( int abil )
/****************************************************/
{
    for (unsigned int i = 0; i < sizeof( Ability_List ); i++) 
    {
        if (Ability_List[i].ability == abil)
            return (Ability_List[i]);
    }

    return (Ability_List[0]);
}


const char * get_ability_name_by_index( char index )
/**************************************************/
{
    const struct ability_def &abil = get_ability_def( Curr_abil[index].which );

    return (abil.name);
}

std::string print_abilities()
{
    std::string text = "\n<w>a:</w> ";

    bool have_any = false;
    if (generate_abilities(false))
    {
        for (int i = 0; i < 52; ++i)
            if (Curr_abil[i].which != ABIL_NON_ABILITY)
            {
                if (have_any)
                    text += ", ";
                text += get_ability_name_by_index(i);
                have_any = true;
            }
    }
    if (!have_any)
        text +=  "no special abilities";

    return text;
}
 
const std::string make_cost_description( const struct ability_def &abil )
/***********************************************************************/
{
    std::ostringstream ret;
    if (abil.mp_cost)
    {
        ret << abil.mp_cost;
        if (abil.flags & ABFLAG_PERMANENT_MP)
            ret << " Permanent";
        ret << " MP";
    }

    if (abil.hp_cost)
    {
        if (ret.str().length())
            ret << ", ";

        ret << abil.hp_cost;
        if (abil.flags & ABFLAG_PERMANENT_HP)
            ret << " Permanent";
        ret << " HP";
    }

    if (abil.food_cost)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Food";   // randomized and amount hidden from player
    }

    if (abil.piety_cost)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Piety";  // randomized and amount hidden from player
    }

    if (abil.flags & ABFLAG_BREATH)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Breath"; 
    }

    if (abil.flags & ABFLAG_DELAY)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Delay";
    }

    if (abil.flags & ABFLAG_PAIN)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Pain";
    }

    if (abil.flags & ABFLAG_EXHAUSTION)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Exhaustion";
    }

    if (abil.flags & ABFLAG_INSTANT)
    {
        if (ret.str().length())
            ret << ", ";

        ret << "Instant"; // not really a cost, more of a bonus -bwr
    }

    // If we haven't output anything so far, then the effect has no cost
    if (!ret.str().length())
        ret << "None";

    return (ret.str());
}

std::vector<const char *> get_ability_names()
{
    std::vector<const char *> abils;
    if (generate_abilities(false))
    {
        for (int i = 0; i < 52; ++i)
        {
            if (Curr_abil[i].which != ABIL_NON_ABILITY)
                abils.push_back( get_ability_name_by_index(i) );
        }
    }
    return (abils);
}

/*
   Activates a menu which gives player access to all of their non-spell
   special abilities - e.g. nagas' spit poison, or the Invocations you get
   from worshipping. Generated dynamically - the function checks to see which
   abilities you have every time.
 */
bool activate_ability(void)
/*************************/
{
    unsigned char keyin = 0;

    int power;
    struct dist abild;
    struct bolt beam;
    struct dist spd;

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return (false);
    }

    // populate the array of structs {dlb}:
    if (!generate_abilities(false))
    {
        mpr("Sorry, you're not good enough to have a special ability.");
        return (false);
    }

    if (!generate_abilities(you.conf))
    {
        mpr("You're too confused!");
        return (false);
    }

    bool  need_redraw = false;
    bool  need_prompt = true;
    bool  need_getch  = true;

    for (;;)
    {
        if (need_redraw)
        {
            mesclr( true );
            redraw_screen();
        }

        if (need_prompt)
            mpr( "Use which ability? (? or * to list)", MSGCH_PROMPT );

        if (need_getch)
            keyin = get_ch();

        need_redraw = false;
        need_prompt = true;
        need_getch  = true;

        if (isalpha( keyin ))
        {
            break;
        }
        else if (keyin == '?' || keyin == '*')
        {
            keyin = show_abilities();

            need_getch  = false;
            need_redraw = true;
            need_prompt = true;
        }
        else if (keyin == ESCAPE || keyin == ' ' 
                || keyin == '\r' || keyin == '\n')
        {
            canned_msg( MSG_OK );    
            return (false);
        }
    }

    if (!isalpha( keyin ))
    {
        mpr("You can't do that.");
        return (false);
    }

    const int abil_used = letter_to_index(keyin);

    if (Curr_abil[abil_used].which == -1)
    {
        mpr("You can't do that.");
        return (false);
    }

    // some abilities don't need a hunger check
    bool hungerCheck = true;
    switch (Curr_abil[abil_used].which)
    {
        case ABIL_RENOUNCE_RELIGION:
        case ABIL_EVOKE_STOP_LEVITATING:
        case ABIL_EVOKE_TURN_VISIBLE:
        case ABIL_END_TRANSFORMATION:
        case ABIL_DELAYED_FIREBALL:
        case ABIL_MUMMY_RESTORATION:
            hungerCheck = false;
            break;
        default:
            break;
    }

    if (hungerCheck && you.hunger_state < HS_HUNGRY)
    {
        mpr("You're too hungry.");
        return (false);
    }

    // no turning back now... {dlb}
    const struct ability_def abil = get_ability_def(Curr_abil[abil_used].which);

    if (random2avg(100, 3) < Curr_abil[abil_used].fail)
    {
        mpr("You fail to use your ability.");
        you.turn_is_over = true;
        return (false);
    }

    if (!enough_mp( abil.mp_cost, false ))
        return (false);

    if (!enough_hp( abil.hp_cost, false ))
        return (false);

    // Note: the costs will not be applied until after this switch 
    // statement... it's assumed that only failures have returned! -- bwr
    switch (abil.ability)
    {
    case ABIL_MUMMY_RESTORATION:
    {
        mpr( "You infuse your body with magical energy." );
        bool did_restore = restore_stat( STAT_ALL, false );

        const int oldhpmax = you.hp_max;
        unrot_hp( 100 );
        if (you.hp_max > oldhpmax)
            did_restore = true;

        // If nothing happened, don't take one max MP, don't use a turn.
        if (!did_restore)
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            return (false);
        }

        break;
    }

    case ABIL_DELAYED_FIREBALL:
    {
        if (spell_direction(spd, beam, DIR_NONE, TARG_ENEMY) == -1)
            return (false);
        
        // Note: power level of ball calculated at release -- bwr
        fireball( calc_spell_power( SPELL_DELAYED_FIREBALL, true ), beam );

        // only one allowed since this is instantaneous -- bwr
        you.attribute[ ATTR_DELAYED_FIREBALL ] = 0;
        break;
    }

    case ABIL_GLAMOUR:
        if (you.duration[DUR_GLAMOUR])
        {
            canned_msg(MSG_CANNOT_DO_YET);
            return (false);
        }

        mpr("You use your Elvish wiles.");

        cast_glamour( 10 + random2(you.experience_level) 
                         + random2(you.experience_level) );

        you.duration[DUR_GLAMOUR] = 20 + random2avg(13, 3);
        break;

    case ABIL_SPIT_POISON:      // Naga + spit poison mutation
        if (you.duration[DUR_BREATH_WEAPON])
        {
            canned_msg(MSG_CANNOT_DO_YET);
            return (false);
        }
        else if (spell_direction(abild, beam) == -1)
        {
            return (false);
        }
        else
        {
            mpr("You spit poison.");

            zapping( ZAP_SPIT_POISON,
                     you.experience_level 
                        + you.mutation[MUT_SPIT_POISON] * 5
                        + (you.species == SP_NAGA) * 10,
                     beam );

            you.duration[DUR_BREATH_WEAPON] = 3 + random2(5);
        }
        break;

    case ABIL_EVOKE_MAPPING:    // randarts
        mpr("You sense your surroundings.");

        magic_mapping(  3 + roll_dice( 2, you.skills[SK_EVOCATIONS] ),
                       40 + roll_dice( 2, you.skills[SK_EVOCATIONS] ) );

        exercise( SK_EVOCATIONS, 1 ); 
        break;

    case ABIL_MAPPING:          // Gnome + sense surrounds mut
        mpr("You sense your surroundings.");

        magic_mapping(  3 + roll_dice( 2, you.experience_level )
                            + you.mutation[MUT_MAPPING] * 10,
                       40 + roll_dice( 2, you.experience_level ) );
        break;

    case ABIL_EVOKE_TELEPORTATION:    // ring of teleportation
    case ABIL_TELEPORTATION:          // teleport mut
        if (you.mutation[MUT_TELEPORT_AT_WILL] == 3)
            you_teleport2( true, true ); // instant and to new area of Abyss
        else
            you_teleport();

        if (abil.ability == ABIL_EVOKE_TELEPORTATION)
            exercise( SK_EVOCATIONS, 1 );
        break;

    case ABIL_BREATHE_FIRE:
    case ABIL_BREATHE_FROST:
    case ABIL_BREATHE_POISON:
    case ABIL_BREATHE_LIGHTNING:
    case ABIL_SPIT_ACID:
    case ABIL_BREATHE_POWER:
    case ABIL_BREATHE_STICKY_FLAME:
    case ABIL_BREATHE_STEAM:
        if (you.duration[DUR_BREATH_WEAPON]
                && abil.ability != ABIL_SPIT_ACID)
        {
            canned_msg(MSG_CANNOT_DO_YET);
            return (false);
        }
        else if (spell_direction( abild, beam ) == -1)
        {
            return (false);
        }

        switch (Curr_abil[abil_used].which)
        {
        case ABIL_BREATHE_FIRE:
            power = you.experience_level;
            power += you.mutation[MUT_BREATHE_FLAMES] * 4;

            if (you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON)
                power += 12;

            // don't check for hell serpents - they get hell fire,
            // never regular fire (GDL)
            mprf("You breathe fire%c", (power < 15)?'.':'!');

            zapping( ZAP_BREATHE_FIRE, power, beam);
            break;

        case ABIL_BREATHE_FROST:
            mpr("You exhale a wave of freezing cold.");
            zapping(ZAP_BREATHE_FROST, you.experience_level, beam);
            break;

        case ABIL_BREATHE_POISON:
            mpr("You exhale a blast of poison gas.");
            zapping(ZAP_BREATHE_POISON, you.experience_level, beam);
            break;

        case ABIL_BREATHE_LIGHTNING:
            mpr("You spit a bolt of lightning.");
            zapping(ZAP_LIGHTNING, (you.experience_level * 2), beam);
            break;

        case ABIL_SPIT_ACID:
            mpr("You spit acid.");
            zapping(ZAP_BREATHE_ACID, you.experience_level, beam);
            break;

        case ABIL_BREATHE_POWER:
            mpr("You spit a bolt of incandescent energy.");
            zapping(ZAP_BREATHE_POWER, you.experience_level, beam);
            break;

        case ABIL_BREATHE_STICKY_FLAME:
            mpr("You spit a glob of burning liquid.");
            zapping(ZAP_STICKY_FLAME, you.experience_level, beam);
            break;

        case ABIL_BREATHE_STEAM:
            mpr("You exhale a blast of scalding steam.");
            zapping(ZAP_BREATHE_STEAM, you.experience_level, beam);
            break;

        }

        if (abil.ability != ABIL_SPIT_ACID)
        {
            you.duration[DUR_BREATH_WEAPON] =
                3 + random2(4) + random2(30 - you.experience_level) / 2;

            if (Curr_abil[abil_used].which == ABIL_BREATHE_STEAM)
                you.duration[DUR_BREATH_WEAPON] /= 2;
        }
        break;

    case ABIL_EVOKE_BLINK:      // randarts
    case ABIL_BLINK:            // mutation
        random_blink(true);

        if (abil.ability == ABIL_EVOKE_BLINK)
            exercise( SK_EVOCATIONS, 1 );
        break;

    case ABIL_EVOKE_BERSERK:    // amulet of rage, randarts
        if (you.hunger_state < HS_SATIATED)
        {
            mpr("You're too hungry to berserk.");
            return (false);
        }

        // only exercise if berserk succeeds
        if ( go_berserk(true) )
            exercise( SK_EVOCATIONS, 1 );
        break;

    // fly (kenku) -- eventually becomes permanent (see acr.cc)
    case ABIL_FLY:
        cast_fly( you.experience_level * 4 );

        if (you.experience_level > 14)
        {
            mpr("You feel very comfortable in the air.");
            you.levitation = 100;
            you.duration[DUR_CONTROLLED_FLIGHT] = 100;
        }
        break;

    case ABIL_FLY_II:           // Fly (Draconians, or anything else with wings)
        if (you.exhausted)
        {
            mpr("You're too exhausted to fly.");
            return (false);
        }
        else if (you.burden_state != BS_UNENCUMBERED)
        {
            mpr("You're carrying too much weight to fly.");
            return (false);
        }
        else
        {
            cast_fly( you.experience_level * 2 );
            // you.attribute[ATTR_EXPENSIVE_FLIGHT] = 1;  // unused
        }
        break;

    // DEMONIC POWERS:
    case ABIL_SUMMON_MINOR_DEMON:
        summon_ice_beast_etc( you.experience_level * 4,
                                     summon_any_demon(DEMON_LESSER) );
        break;

    case ABIL_SUMMON_DEMONS:
        summon_ice_beast_etc( you.experience_level * 4,
                                     summon_any_demon(DEMON_COMMON) );
        break;

    case ABIL_HELLFIRE:
        if (your_spells(SPELL_HELLFIRE, 
                        20 + you.experience_level, false) == SPRET_ABORT)
            return (false);
        break;

    case ABIL_TORMENT:
        if (you.is_undead)
        {
            mpr("The unliving cannot use this ability.");
            return (false);
        }

        torment(TORMENT_GENERIC, you.x_pos, you.y_pos);
        break;

    case ABIL_RAISE_DEAD:
        your_spells(SPELL_ANIMATE_DEAD, you.experience_level * 5, false);
        break;

    case ABIL_CONTROL_DEMON:
        if (spell_direction(abild, beam) == -1)
        {
            return (false);
        }

        zapping(ZAP_CONTROL_DEMON, you.experience_level * 5, beam);
        break;

    case ABIL_TO_PANDEMONIUM:
        if (you.level_type == LEVEL_PANDEMONIUM)
        {
            mpr("You're already here.");
            return (false);
        }

        banished(DNGN_ENTER_PANDEMONIUM);
        break;

    case ABIL_CHANNELING:
        mpr("You channel some magical energy.");
        inc_mp(1 + random2(5), false);
        break;

    case ABIL_THROW_FLAME:
    case ABIL_THROW_FROST:
        if (spell_direction(abild, beam) == -1)
        {
            return (false);
        }

        zapping( (Curr_abil[abil_used].which == ABIL_THROW_FLAME ? ZAP_FLAME 
                                                                 : ZAP_FROST),
                    you.experience_level * 3,
                    beam );
        break;

    case ABIL_BOLT_OF_DRAINING:
        if (spell_direction(abild, beam) == -1)
        {
            return (false);
        }

        zapping(ZAP_NEGATIVE_ENERGY, you.experience_level * 6, beam);
        break;

    case ABIL_EVOKE_TURN_INVISIBLE:     // ring, randarts, darkness items
        if (you.hunger_state < HS_SATIATED)
        {
            mpr("You're too hungry to turn invisible.");
            return (false);
        }

        potion_effect( POT_INVISIBILITY, 2 * you.skills[SK_EVOCATIONS] + 5 );
        contaminate_player( 1 + random2(3) );
        exercise( SK_EVOCATIONS, 1 );
        break;

    case ABIL_EVOKE_TURN_VISIBLE:
        mpr("You feel less transparent.");
        you.invis = 1;
        break;

    case ABIL_EVOKE_LEVITATE:           // ring, boots, randarts
        potion_effect( POT_LEVITATION, 2 * you.skills[SK_EVOCATIONS] + 30 );
        exercise( SK_EVOCATIONS, 1 );
        break;

    case ABIL_EVOKE_STOP_LEVITATING:
        mpr("You feel heavy.");
        you.levitation = 1;
        break;

    case ABIL_END_TRANSFORMATION:
        mpr("You feel almost normal.");
        you.duration[DUR_TRANSFORMATION] = 2;
        break;

    // INVOCATIONS:
    case ABIL_ZIN_REPEL_UNDEAD:
    case ABIL_TSO_REPEL_UNDEAD:
        turn_undead(you.piety);

        if (!you.duration[DUR_REPEL_UNDEAD])
            mpr( "You feel a holy aura protecting you." );

        you.duration[DUR_REPEL_UNDEAD] += 8 
                                + roll_dice(2, 2 * you.skills[SK_INVOCATIONS]);

        if (you.duration[ DUR_REPEL_UNDEAD ] > 50)
            you.duration[ DUR_REPEL_UNDEAD ] = 50;

        exercise(SK_INVOCATIONS, 1);
        break;

    case ABIL_ZIN_HEALING:
        if (!cast_healing( 3 + (you.skills[SK_INVOCATIONS] / 6) ))
            break;

        exercise(SK_INVOCATIONS, 1 + random2(3));
        break;

    case ABIL_ZIN_PESTILENCE:
        mpr( "You call forth a swarm of pestilential beasts!" );

        if (!summon_swarm( you.skills[SK_INVOCATIONS] * 8, false, true ))
            mpr( "Nothing seems to have answered your call." );

        exercise( SK_INVOCATIONS, 2 + random2(4) );
        break;

    case ABIL_ZIN_HOLY_WORD:
        holy_word( you.skills[SK_INVOCATIONS] * 8 );
        exercise(SK_INVOCATIONS, 3 + random2(5));
        break;

    case ABIL_ZIN_SUMMON_GUARDIAN:
        summon_ice_beast_etc(you.skills[SK_INVOCATIONS] * 4, MONS_ANGEL, true);
        exercise(SK_INVOCATIONS, 8 + random2(10));
        break;

    case ABIL_TSO_SMITING:
        if (your_spells( SPELL_SMITING, (2 + skill_bump(SK_INVOCATIONS)) * 6,
                         false ) == SPRET_ABORT)
            return (false);
        exercise( SK_INVOCATIONS, (coinflip()? 3 : 2) );
        break;

    case ABIL_TSO_ANNIHILATE_UNDEAD:
        if (spell_direction(spd, beam) == -1)
        {
            return (false);
        }

        zapping(ZAP_DISPEL_UNDEAD, you.skills[SK_INVOCATIONS] * 6, beam);
        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_TSO_CLEANSING_FLAME:
        if (spell_direction(spd, beam) == -1)
        {
            return (false);
        }

        zapping(ZAP_CLEANSING_FLAME, 20 + you.skills[SK_INVOCATIONS] * 6, beam);
        exercise(SK_INVOCATIONS, 3 + random2(6));
        break;

    case ABIL_TSO_SUMMON_DAEVA:
        summon_ice_beast_etc(you.skills[SK_INVOCATIONS] * 4, MONS_DAEVA, true);
        exercise(SK_INVOCATIONS, 8 + random2(10));
        break;

    case ABIL_KIKU_RECALL_UNDEAD_SLAVES:
        recall(1);
        exercise(SK_INVOCATIONS, 1);
        break;

    case ABIL_KIKU_ENSLAVE_UNDEAD:
        if (spell_direction(spd, beam) == -1)
        {
            return (false);
        }

        zapping( ZAP_ENSLAVE_UNDEAD, you.skills[SK_INVOCATIONS] * 8, beam );
        exercise(SK_INVOCATIONS, 5 + random2(5));
        break;

    case ABIL_KIKU_INVOKE_DEATH:
        summon_ice_beast_etc(
                20 + you.skills[SK_INVOCATIONS] * 3, MONS_REAPER, true);
        exercise(SK_INVOCATIONS, 10 + random2(14));
        break;

    case ABIL_YRED_ANIMATE_CORPSE:
        mpr("You call on the dead to walk for you...");

        animate_a_corpse( you.x_pos, you.y_pos, BEH_FRIENDLY, 
                          you.pet_target, CORPSE_BODY );

        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_YRED_RECALL_UNDEAD:
        recall(1);
        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_YRED_ANIMATE_DEAD:
        mpr("You call on the dead to walk for you...");

        animate_dead( 1 + you.skills[SK_INVOCATIONS], BEH_FRIENDLY,
                      you.pet_target, 1 );

        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_YRED_DRAIN_LIFE:
        drain_life( you.skills[SK_INVOCATIONS] );
        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_YRED_CONTROL_UNDEAD:
        mass_enchantment( ENCH_CHARM, you.skills[SK_INVOCATIONS] * 8, MHITYOU );
        exercise(SK_INVOCATIONS, 3 + random2(4));
        break;

    case ABIL_SIF_MUNA_CHANNEL_ENERGY:
        mpr("You channel some magical energy.");

        inc_mp(1 + random2(you.skills[SK_INVOCATIONS] / 4 + 2), false);
        exercise(SK_INVOCATIONS, 1 + random2(3));
        break;

    case ABIL_OKAWARU_MIGHT:
        potion_effect( POT_MIGHT, you.skills[SK_INVOCATIONS] * 8 );
        exercise(SK_INVOCATIONS, 1 + random2(3));
        break;

    case ABIL_OKAWARU_HEALING:
        if (!cast_healing( 3 + (you.skills[SK_INVOCATIONS] / 6) ))
            break;

        exercise(SK_INVOCATIONS, 2 + random2(5));
        break;

    case ABIL_OKAWARU_HASTE:
        potion_effect( POT_SPEED, you.skills[SK_INVOCATIONS] * 8 );
        exercise(SK_INVOCATIONS, 3 + random2(7));
        break;

    case ABIL_MAKHLEB_MINOR_DESTRUCTION:
        if (spell_direction(spd, beam) == -1)
        {
            return (false);
        }

        power = you.skills[SK_INVOCATIONS]
                    + random2( 1 + you.skills[SK_INVOCATIONS] )
                    + random2( 1 + you.skills[SK_INVOCATIONS] );

        switch (random2(5))
        {
        case 0: zapping( ZAP_FLAME,        power,     beam ); break;
        case 1: zapping( ZAP_PAIN,         power,     beam ); break;
        case 2: zapping( ZAP_STONE_ARROW,  power,     beam ); break;
        case 3: zapping( ZAP_ELECTRICITY,  power,     beam ); break;
        case 4: zapping( ZAP_BREATHE_ACID, power / 2, beam ); break;
        }

        exercise(SK_INVOCATIONS, 1);
        break;

    case ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB:
        summon_ice_beast_etc( 20 + you.skills[SK_INVOCATIONS] * 3,
                              MONS_NEQOXEC + random2(5), true );

        exercise(SK_INVOCATIONS, 2 + random2(3));
        break;

    case ABIL_MAKHLEB_MAJOR_DESTRUCTION:
        if (spell_direction(spd, beam) == -1)
        {
            return (false);
        }

        power = you.skills[SK_INVOCATIONS] * 3
                    + random2( 1 + you.skills[SK_INVOCATIONS] )
                    + random2( 1 + you.skills[SK_INVOCATIONS] );

        switch (random2(8))
        {
        case 0: zapping( ZAP_FIRE,               power, beam ); break;
        case 1: zapping( ZAP_FIREBALL,           power, beam ); break;
        case 2: zapping( ZAP_LIGHTNING,          power, beam ); break;
        case 3: zapping( ZAP_NEGATIVE_ENERGY,    power, beam ); break;
        case 4: zapping( ZAP_STICKY_FLAME,       power, beam ); break;
        case 5: zapping( ZAP_IRON_BOLT,          power, beam ); break;
        case 6: zapping( ZAP_ORB_OF_ELECTRICITY, power, beam ); break;

        case 7:
            you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] = 1;
            mpr("Makhleb hurls a blast of lightning!");

            // make a divine lightning bolt...
            beam.beam_source = NON_MONSTER;
            beam.type = SYM_BURST;
            beam.damage = dice_def( 3, 30 );
            beam.flavour = BEAM_ELECTRICITY;
            beam.target_x = you.x_pos;
            beam.target_y = you.y_pos;
            beam.name = "blast of lightning";
            beam.colour = LIGHTCYAN;
            beam.thrower = KILL_YOU;
            beam.aux_source = "Makhleb's lightning strike";
            beam.ex_size = 1 + you.skills[SK_INVOCATIONS] / 8;
            beam.is_tracer = false;

            // ... and fire!
            explosion(beam);

            // protection down
            mpr("Your divine protection wanes.");
            you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] = 0;
            break;
        }

        exercise(SK_INVOCATIONS, 3 + random2(5));
        break;

    case ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB:
        summon_ice_beast_etc( 20 + you.skills[SK_INVOCATIONS] * 3,
                              MONS_EXECUTIONER + random2(5),
                              true );

        exercise(SK_INVOCATIONS, 6 + random2(6));
        break;

    case ABIL_TROG_BERSERK:
        // Trog abilities don't use or train invocations. 
        if (you.hunger_state < HS_SATIATED)
        {
            mpr("You're too hungry to berserk.");
            return (false);
        }

        go_berserk(true);
        break;

    case ABIL_TROG_MIGHT:
        // Trog abilities don't use or train invocations. 
        potion_effect( POT_MIGHT, 150 );
        break;

    case ABIL_TROG_HASTE_SELF:
        // Trog abilities don't use or train invocations. 
        potion_effect( POT_SPEED, 150 );
        break;

    case ABIL_SIF_MUNA_FORGET_SPELL:
        cast_selective_amnesia(true);
        break;

    case ABIL_ELYVILON_LESSER_HEALING:
        if (!cast_healing( 3 + (you.skills[SK_INVOCATIONS] / 6) ))
            break;

        exercise( SK_INVOCATIONS, 1 );
        break;

    case ABIL_ELYVILON_PURIFICATION:
        purification();
        exercise( SK_INVOCATIONS, 2 + random2(3) );
        break;

    case ABIL_ELYVILON_HEALING:
        if (!cast_healing( 10 + (you.skills[SK_INVOCATIONS] / 3) ))
            break;

        exercise( SK_INVOCATIONS, 3 + random2(5) );
        break;

    case ABIL_ELYVILON_RESTORATION:
        restore_stat( STAT_ALL, false );
        unrot_hp( 100 );

        exercise( SK_INVOCATIONS, 4 + random2(6) );
        break;

    case ABIL_ELYVILON_GREATER_HEALING:
        if (!cast_healing( 20 + you.skills[SK_INVOCATIONS] * 2 ))
            break;

        exercise( SK_INVOCATIONS, 6 + random2(10) );
        break;

    case ABIL_LUGONU_ABYSS_EXIT:
        if ( you.level_type != LEVEL_ABYSS )
        {
            mpr("You aren't in the Abyss!");
            return false;       // don't incur costs
        }
        banished(DNGN_EXIT_ABYSS);
        exercise(SK_INVOCATIONS, 8 + random2(10));

        // Lose 1d2 permanent HP
        you.hp_max -= (coinflip() ? 2 : 1);
        // Deflate HP
        set_hp( 1 + random2(you.hp), false );

        // Lose 1d2 permanent MP
        rot_mp(coinflip() ? 2 : 1);
        // Deflate MP
        if (you.magic_points)
            set_mp(random2(you.magic_points), false);
        break;

    case ABIL_LUGONU_BEND_SPACE:
        lugonu_bends_space();
        exercise(SK_INVOCATIONS, 2 + random2(3));
        break;

    case ABIL_LUGONU_SUMMON_DEMONS:
    {
        int ndemons = 1 + you.skills[SK_INVOCATIONS] / 4;
        if (ndemons > 5)
            ndemons = 5;
        
        for ( int i = 0; i < ndemons; ++i )
            summon_ice_beast_etc( 20 + you.skills[SK_INVOCATIONS] * 3,
                                  summon_any_demon(DEMON_COMMON), true);
        
        exercise(SK_INVOCATIONS, 6 + random2(6));
        break;
    }

    case ABIL_LUGONU_ABYSS_ENTER:
        if (you.level_type == LEVEL_ABYSS)
        {
            mpr("You're already here.");
            return false;
        }
        else if (you.level_type == LEVEL_PANDEMONIUM)
        {
            mpr("That doesn't work from Pandemonium.");
            return false;
        }

        activate_notes(false);  // this banishment shouldn't be noted
        banished(DNGN_ENTER_ABYSS);
        activate_notes(true);
        break;

    //jmf: intended as invocations from evil god(s):
    case ABIL_CHARM_SNAKE:
        cast_snake_charm( you.experience_level * 2
                            + you.skills[SK_INVOCATIONS] * 3 );

        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_TRAN_SERPENT_OF_HELL:
        transform(10 + (you.experience_level * 2) +
                  (you.skills[SK_INVOCATIONS] * 3), TRAN_SERPENT_OF_HELL);

        exercise(SK_INVOCATIONS, 6 + random2(9));
        break;

    case ABIL_BREATHE_HELLFIRE:
        if (you.duration[DUR_BREATH_WEAPON])
        {
            canned_msg(MSG_CANNOT_DO_YET);
            return (false);
        }

        if (your_spells( SPELL_HELLFIRE, 
                        20 + you.experience_level, false ) == SPRET_ABORT)
            return (false);

        you.duration[DUR_BREATH_WEAPON] +=
                        3 + random2(5) + random2(30 - you.experience_level);
        break;

    case ABIL_ROTTING:
        cast_rotting(you.experience_level * 2 + you.skills[SK_INVOCATIONS] * 3);
        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_TORMENT_II:
        if (you.is_undead)
        {
            mpr("The unliving cannot use this ability.");
            return (false);
        }

        torment(TORMENT_GENERIC, you.x_pos, you.y_pos);
        exercise(SK_INVOCATIONS, 2 + random2(4));
        break;

    case ABIL_RENOUNCE_RELIGION:
        if (yesno("Really renounce your faith, foregoing its fabulous benefits?")
            && yesno( "Are you sure you won't change your mind later?" ))
        {
            excommunication();
        }
        else
        {
            canned_msg(MSG_OK);
        }
        break;

    default:
        mpr("Sorry, you can't do that.");
        break;
    }

    // currently only delayed fireball is instantaneous -- bwr
    you.turn_is_over = !(abil.flags & ABFLAG_INSTANT);

    // All failures should have returned by this point, so we'll 
    // apply the costs -- its not too neat, but it works for now. -- bwr
    const int food_cost = abil.food_cost + random2avg(abil.food_cost, 2);
    const int piety_cost = abil.piety_cost + random2((abil.piety_cost + 1) / 2 + 1);

#if DEBUG_DIAGNOSTICS
    mprf(MSGCH_DIAGNOSTICS, "Cost: mp=%d; hp=%d; food=%d; piety=%d",
         abil.mp_cost, abil.hp_cost, food_cost, piety_cost );
#endif

    if (abil.mp_cost)
    {
        dec_mp( abil.mp_cost );
        if (abil.flags & ABFLAG_PERMANENT_MP)
            rot_mp(1);
    }

    if (abil.hp_cost)
    {
        dec_hp( abil.hp_cost, false );
        if (abil.flags & ABFLAG_PERMANENT_HP)
            rot_hp(1);
    }

    if (food_cost)
        make_hungry( food_cost, false );

    if (piety_cost)
        lose_piety( piety_cost );

    return (true);
}  // end activate_ability()


// Lists any abilities the player may possess
char show_abilities( void )
/*************************/
{
    int loopy = 0;
    char lines = 0;
    unsigned char anything = 0;
    char ki;
    bool can_invoke = false;

    const int num_lines = get_number_of_lines();

    for (loopy = 0; loopy < 52; loopy++)
    {
        if (Curr_abil[loopy].is_invocation)
        {
            can_invoke = true;
            break;
        }
    }


    clrscr();
    cprintf("  Ability                           Cost                    Success");
    lines++;

    for (int do_invoke = 0; do_invoke < (can_invoke ? 2 : 1); do_invoke++)
    {
        if (do_invoke)
        {
            anything++;
            textcolor(BLUE);
            cprintf(EOL "    Invocations - ");
            textcolor(LIGHTGREY);
            lines++;
        }

        for (loopy = 0; loopy < 52; loopy++)
        {
            if (lines > num_lines - 2)
            {
                gotoxy(1, num_lines);
                cprintf("-more-");

                ki = getch();

                if (ki == ESCAPE)
                {
                    return (ESCAPE);
                }

                if (ki >= 'A' && ki <= 'z')
                {
                    return (ki);
                }

                if (ki == 0)
                    ki = getch();

                lines = 0;
                clrscr();
                gotoxy(1, 1);
                anything = 0;
            }

            if (Curr_abil[loopy].which != ABIL_NON_ABILITY
                && (do_invoke == Curr_abil[loopy].is_invocation))
            {
                anything++;

                if (lines > 0)
                    cprintf(EOL);

                lines++;

                const struct ability_def abil = get_ability_def( Curr_abil[loopy].which );

                cprintf( " %c - %s", index_to_letter(loopy), abil.name );

                // Output costs:
                gotoxy( 35, wherey() );

                std::string cost_str = make_cost_description( abil );

                if (cost_str.length() > 24)
                    cost_str = cost_str.substr( 0, 24 );

                cprintf( "%s", cost_str.c_str() );

                gotoxy(60, wherey());

                cprintf( "%s", failure_rate_to_string(Curr_abil[loopy].fail));

                gotoxy(70, wherey());
            }                              // end if conditional
        }                                  // end "for loopy"
    }

    if (anything > 0)
    {
        ki = getch();

        if (ki >= 'A' && ki <= 'z')
        {
            return (ki);
        }

        if (ki == 0)
            ki = getch();

        return (ki);
    }

    ki = getch();

    return (ki);
}                               // end show_abilities()


bool generate_abilities( bool check_confused )
/*****************************/
{
    int loopy;

    // fill array of structs with "empty" values {dlb}:
    for (loopy = 0; loopy < 52; loopy++)
    {
        Curr_abil[loopy].which = ABIL_NON_ABILITY;
        Curr_abil[loopy].fail = 100;
        Curr_abil[loopy].is_invocation = false;
    }

    // first we do the racial abilities:
    
    // Mummies get the ability to restore HPs and stats, but it
    // costs permanent MP (and those can never be recovered).  -- bwr
    if (you.species == SP_MUMMY && you.experience_level >= 13)
    {
        insert_ability( ABIL_MUMMY_RESTORATION, check_confused );
    }

    // checking for species-related abilities and mutagenic counterparts {dlb}:
    if (you.attribute[ATTR_TRANSFORMATION] == TRAN_NONE
        && ((you.species == SP_GREY_ELF && you.experience_level >= 5)
            || (you.species == SP_HIGH_ELF && you.experience_level >= 15)))
    {
        insert_ability( ABIL_GLAMOUR, check_confused );
    }

    if (you.species == SP_NAGA)
    {
        if (you.mutation[MUT_BREATHE_POISON])
            insert_ability( ABIL_BREATHE_POISON, check_confused );
        else
            insert_ability( ABIL_SPIT_POISON, check_confused );
    }
    else if (you.mutation[MUT_SPIT_POISON])
    {
        insert_ability( ABIL_SPIT_POISON, check_confused );
    }

    if (player_genus(GENPC_DRACONIAN))
    {
        if (you.experience_level >= 7)
        {
            const int ability = (
                (you.species == SP_GREEN_DRACONIAN)  ? ABIL_BREATHE_POISON :
                (you.species == SP_RED_DRACONIAN)    ? ABIL_BREATHE_FIRE :
                (you.species == SP_WHITE_DRACONIAN)  ? ABIL_BREATHE_FROST :
                (you.species == SP_GOLDEN_DRACONIAN) ? ABIL_SPIT_ACID :
                (you.species == SP_BLACK_DRACONIAN)  ? ABIL_BREATHE_LIGHTNING :
                (you.species == SP_PURPLE_DRACONIAN) ? ABIL_BREATHE_POWER :
                (you.species == SP_PALE_DRACONIAN)   ? ABIL_BREATHE_STEAM :
                (you.species == SP_MOTTLED_DRACONIAN)? ABIL_BREATHE_STICKY_FLAME:
                                                     -1);
            if (ability != -1)
                insert_ability( ability, check_confused );
        }
    }

    //jmf: alternately put check elsewhere
    if ((you.level_type == LEVEL_DUNGEON && you.mutation[MUT_MAPPING]) ||
        (you.level_type == LEVEL_PANDEMONIUM && you.mutation[MUT_MAPPING]==3))
    {
        insert_ability( ABIL_MAPPING, check_confused );
    }

    if (!you.duration[DUR_CONTROLLED_FLIGHT] && !player_is_levitating())
    {
        // kenku can fly, but only from the ground
        // (until level 15, when it becomes permanent until revoked)
        //jmf: "upgrade" for draconians -- expensive flight
        if (you.species == SP_KENKU && you.experience_level >= 5)
            insert_ability( ABIL_FLY, check_confused );
        else if (player_genus(GENPC_DRACONIAN) && you.mutation[MUT_BIG_WINGS])
            insert_ability( ABIL_FLY_II, check_confused );
    }

    // demonic powers {dlb}:
    if (you.mutation[MUT_SUMMON_MINOR_DEMONS])
        insert_ability( ABIL_SUMMON_MINOR_DEMON, check_confused );

    if (you.mutation[MUT_SUMMON_DEMONS])
        insert_ability( ABIL_SUMMON_DEMONS, check_confused );

    if (you.mutation[MUT_HURL_HELLFIRE])
        insert_ability( ABIL_HELLFIRE, check_confused );

    if (you.mutation[MUT_CALL_TORMENT])
        insert_ability( ABIL_TORMENT, check_confused );

    if (you.mutation[MUT_RAISE_DEAD])
        insert_ability( ABIL_RAISE_DEAD, check_confused );

    if (you.mutation[MUT_CONTROL_DEMONS])
        insert_ability( ABIL_CONTROL_DEMON, check_confused );

    if (you.mutation[MUT_PANDEMONIUM])
        insert_ability( ABIL_TO_PANDEMONIUM, check_confused );

    if (you.mutation[MUT_CHANNEL_HELL])
        insert_ability( ABIL_CHANNELING, check_confused );

    if (you.mutation[MUT_THROW_FLAMES])
        insert_ability( ABIL_THROW_FLAME, check_confused );

    if (you.mutation[MUT_THROW_FROST])
        insert_ability( ABIL_THROW_FROST, check_confused );

    if (you.mutation[MUT_SMITE])
        insert_ability( ABIL_BOLT_OF_DRAINING, check_confused );

    if (you.duration[DUR_TRANSFORMATION])
        insert_ability( ABIL_END_TRANSFORMATION, check_confused );

    if (you.mutation[MUT_BLINK]) 
        insert_ability( ABIL_BLINK, check_confused );

    if (you.mutation[MUT_TELEPORT_AT_WILL])
        insert_ability( ABIL_TELEPORTATION, check_confused );

    // gods take abilities away until penance completed -- bwr
    if (!player_under_penance() && !silenced( you.x_pos, you.y_pos ))
    {
        for ( int i = 0; i < MAX_GOD_ABILITIES; ++i )
        {
            if ( you.piety >= piety_breakpoint(i) )
            {
                ability_type abil = god_abilities[you.religion][i];
                if ( abil != ABIL_NON_ABILITY )
                    insert_ability(abil, check_confused);
            }
        }
    }

    // and finally, the ability to opt-out of your faith {dlb}:
    if (you.religion != GOD_NO_GOD && !silenced( you.x_pos, you.y_pos ))
        insert_ability( ABIL_RENOUNCE_RELIGION, check_confused );

    //jmf: check for breath weapons -- they're exclusive of each other I hope!
    //     better make better ones first.
    if (you.attribute[ATTR_TRANSFORMATION] == TRAN_SERPENT_OF_HELL)
    {
        insert_ability( ABIL_BREATHE_HELLFIRE, check_confused );
    }
    else if (you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON
                                        || you.mutation[MUT_BREATHE_FLAMES])
    {
        insert_ability( ABIL_BREATHE_FIRE, check_confused );
    }

    // checking for unreleased delayed fireball
    if (you.attribute[ ATTR_DELAYED_FIREBALL ])
    {
        insert_ability( ABIL_DELAYED_FIREBALL, check_confused );
    }

    // evocations from items:
    if (scan_randarts(RAP_BLINK))
        insert_ability( ABIL_EVOKE_BLINK, check_confused );

    if (wearing_amulet(AMU_RAGE) || scan_randarts(RAP_BERSERK))
        insert_ability( ABIL_EVOKE_BERSERK, check_confused );

    if (scan_randarts( RAP_MAPPING ))
        insert_ability( ABIL_EVOKE_MAPPING, check_confused );

    if (player_equip( EQ_RINGS, RING_INVISIBILITY )
        || player_equip_ego_type( EQ_ALL_ARMOUR, SPARM_DARKNESS )
        || scan_randarts( RAP_INVISIBLE ))
    {
        // Now you can only turn invisibility off if you have an
        // activatable item.  Wands and potions allow will have 
        // to time out. -- bwr
        if (you.invis)
            insert_ability( ABIL_EVOKE_TURN_VISIBLE, check_confused );
        else
            insert_ability( ABIL_EVOKE_TURN_INVISIBLE, check_confused );
    }

    //jmf: "upgrade" for draconians -- expensive flight
    // note: this ability only applies to this counter
    if (player_equip( EQ_RINGS, RING_LEVITATION )
        || player_equip_ego_type( EQ_BOOTS, SPARM_LEVITATION )
        || scan_randarts( RAP_LEVITATE ))
    {
        // Now you can only turn levitation off if you have an
        // activatable item.  Potions and miscast effects will 
        // have to time out (this makes the miscast effect actually
        // a bit annoying). -- bwr
        if (you.levitation) 
            insert_ability( ABIL_EVOKE_STOP_LEVITATING, check_confused );
        else
            insert_ability( ABIL_EVOKE_LEVITATE, check_confused );
    }

    if (player_equip( EQ_RINGS, RING_TELEPORTATION )
        || scan_randarts( RAP_CAN_TELEPORT ))
    {
        insert_ability( ABIL_EVOKE_TELEPORTATION, check_confused );
    }

    // this is a shameless kludge for the time being {dlb}:
    // still shameless. -- bwr
    for (loopy = 0; loopy < 52; loopy++)
    {
        if (Curr_abil[loopy].which != ABIL_NON_ABILITY)
            return (true);
    }

    return (false);
}                               // end generate_abilities()

// Note: we're trying for a behaviour where the player gets
// to keep their assigned invocation slots if they get excommunicated
// and then rejoin (but if they spend time with another god we consider 
// the old invocation slots void and erase them).  We also try to 
// protect any bindings the character might have made into the 
// traditional invocation slots (A-E and X). -- bwr
static void set_god_ability_helper( int abil, char letter )
{
    int i;
    const int index = letter_to_index( letter );

    for (i = 0; i < 52; i++)
    {
        if (you.ability_letter_table[i] == abil)
            break;
    }

    if (i == 52)        // ability is not already assigned
    {
        // if slot is unoccupied, move in
        if (you.ability_letter_table[index] == ABIL_NON_ABILITY)
            you.ability_letter_table[index] = abil;
    }
}

// return GOD_NO_GOD if it isn't a god ability, otherwise return
// the index of the god.
static int is_god_ability(int abil)
{
    if ( abil == ABIL_NON_ABILITY )
        return GOD_NO_GOD;
    for ( int i = 0; i < MAX_NUM_GODS; ++i )
        for ( int j = 0; j < MAX_GOD_ABILITIES; ++j )
            if ( god_abilities[i][j] == abil )
                return i;
    return GOD_NO_GOD;
}

void set_god_ability_slots( void )
{
    ASSERT( you.religion != GOD_NO_GOD );

    int i;

    set_god_ability_helper( ABIL_RENOUNCE_RELIGION, 'X' );

    // clear out other god invocations
    for (i = 0; i < 52; i++)
    {
        const int god = is_god_ability(you.ability_letter_table[i]);
        if ( god != GOD_NO_GOD && god != you.religion )
            you.ability_letter_table[i] = ABIL_NON_ABILITY;
    }

    // finally, add in current god's invocations in traditional slots:
    int num = 0;
    for ( i = 0; i < MAX_GOD_ABILITIES; ++i )
    {
        if ( god_abilities[you.religion][i] != ABIL_NON_ABILITY )
        {
            set_god_ability_helper(god_abilities[you.religion][i],
                                   (Options.lowercase_invocations ? 'a' : 'A') + num);
            ++num;
        }
    }
}


// returns index to Curr_abil, -1 on failure
static int find_ability_slot( int which_ability )
/***********************************************/
{
    int  slot;
    for (slot = 0; slot < 52; slot++)
    {
        if (you.ability_letter_table[slot] == which_ability)
            break;
    }

    // no requested slot, find new one and make it preferred.
    if (slot == 52)  
    {
        // skip over a-e if player prefers them for invocations
        for (slot = (Options.lowercase_invocations ? 5 : 0); slot < 52; slot++)
        {
            if (you.ability_letter_table[slot] == ABIL_NON_ABILITY)
                break;
        }

        // if we skipped over a-e to reserve them, try them now
        if (Options.lowercase_invocations && slot == 52)
        {
            for (slot = 5; slot >= 0; slot--)
            {
                if (you.ability_letter_table[slot] == ABIL_NON_ABILITY)
                    break;
            }
        }

        // All letters are assigned, check Curr_abil and try to steal a letter
        if (slot == 52)   
        {
            // backwards, to protect the low lettered slots from replacement
            for (slot = 51; slot >= 0; slot--)
            {
                if (Curr_abil[slot].which == ABIL_NON_ABILITY)
                    break;
            }

            // no slots at all == no hope of adding
            if (slot < 0) 
                return (-1);
        }

        // this ability now takes over this slot
        you.ability_letter_table[slot] = which_ability;
    }

    return (slot);
}

static bool insert_ability( int which_ability, bool check_conf ) 
/**********************************************/
{
    ASSERT( which_ability != ABIL_NON_ABILITY );

    int failure = 0;
    bool perfect = false;  // is perfect
    bool invoc = false;

    // Look through the table to see if there's a preference, else 
    // find a new empty slot for this ability. -- bwr
    const int slot = find_ability_slot( which_ability );
    if (slot == -1)
        return (false);

    if (check_conf)
    {
        const ability_def &abil = get_ability_def(which_ability);
        if (you.conf && !testbits(abil.flags, ABFLAG_CONF_OK))
            return (false);
    }

    Curr_abil[slot].which = which_ability;

    switch (which_ability)
    {
    // begin spell abilities
    case ABIL_DELAYED_FIREBALL:
    case ABIL_MUMMY_RESTORATION:
        perfect = true;
        failure = 0;
        break;

    // begin species abilities - some are mutagenic, too {dlb}
    case ABIL_GLAMOUR:
        failure = 50 - (you.experience_level * 2);
        break;

    case ABIL_SPIT_POISON:
        failure = ((you.species == SP_NAGA) ? 20 : 40)
                        - 10 * you.mutation[MUT_SPIT_POISON] 
                        - you.experience_level;
        break;

    case ABIL_EVOKE_MAPPING:
        failure = 30 - you.skills[SK_EVOCATIONS]; 
        break;

    case ABIL_MAPPING:
        failure = 40 - 10 * you.mutation[MUT_MAPPING] - you.experience_level;
        break;

    case ABIL_BREATHE_FIRE:
        failure = ((you.species == SP_RED_DRACONIAN) ? 30 : 50)
                        - 10 * you.mutation[MUT_BREATHE_FLAMES]
                        - you.experience_level;

        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON)
            failure -= 20;
        break;

    case ABIL_BREATHE_FROST:
    case ABIL_BREATHE_POISON:
    case ABIL_SPIT_ACID:
    case ABIL_BREATHE_LIGHTNING:
    case ABIL_BREATHE_POWER:
    case ABIL_BREATHE_STICKY_FLAME:
        failure = 30 - you.experience_level;

        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON)
            failure -= 20;
        break;

    case ABIL_BREATHE_STEAM:
        failure = 20 - you.experience_level;

        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON)
            failure -= 20;
        break;

    case ABIL_FLY:              // this is for kenku {dlb}
        failure = 45 - (3 * you.experience_level);
        break;

    case ABIL_FLY_II:           // this is for draconians {dlb}
        failure = 45 - (you.experience_level + you.strength);
        break;
        // end species abilties (some mutagenic)

        // begin demonic powers {dlb}
    case ABIL_THROW_FLAME:
    case ABIL_THROW_FROST:
        failure = 10 - you.experience_level;
        break;

    case ABIL_SUMMON_MINOR_DEMON:
        failure = 27 - you.experience_level;
        break;

    case ABIL_CHANNELING:
    case ABIL_BOLT_OF_DRAINING:
        failure = 30 - you.experience_level;
        break;

    case ABIL_CONTROL_DEMON:
        failure = 35 - you.experience_level;
        break;

    case ABIL_SUMMON_DEMONS:
        failure = 40 - you.experience_level;
        break;

    case ABIL_TO_PANDEMONIUM:
        failure = 57 - (you.experience_level * 2);
        break;

    case ABIL_HELLFIRE:
    case ABIL_RAISE_DEAD:
        failure = 50 - you.experience_level;
        break;

    case ABIL_TORMENT:
        failure = 60 - you.experience_level;
        break;

    case ABIL_BLINK:
        failure = 30 - (10 * you.mutation[MUT_BLINK]) - you.experience_level;
        break;

    case ABIL_TELEPORTATION:
        failure = ((you.mutation[MUT_TELEPORT_AT_WILL] > 1) ? 30 : 50) 
                    - you.experience_level;
        break;
        // end demonic powers {dlb}

        // begin transformation abilities {dlb}
    case ABIL_END_TRANSFORMATION:
        perfect = true;
        failure = 0;
        break;

    case ABIL_BREATHE_HELLFIRE:
        failure = 32 - you.experience_level;
        break;
        // end transformation abilities {dlb}
        //
        // begin item abilities - some possibly mutagenic {dlb}
    case ABIL_EVOKE_TURN_INVISIBLE:
    case ABIL_EVOKE_TELEPORTATION:
        failure = 60 - 2 * you.skills[SK_EVOCATIONS];
        break;

    case ABIL_EVOKE_TURN_VISIBLE:
    case ABIL_EVOKE_STOP_LEVITATING:
        perfect = true;
        failure = 0;
        break;

    case ABIL_EVOKE_LEVITATE:
    case ABIL_EVOKE_BLINK:
        failure = 40 - 2 * you.skills[SK_EVOCATIONS];
        break;

    case ABIL_EVOKE_BERSERK:
        failure = 50 - 2 * you.skills[SK_EVOCATIONS];

        if (you.species == SP_TROLL)
            failure -= 30;
        else if (player_genus(GENPC_DWARVEN) || you.species == SP_HILL_ORC 
                || you.species == SP_OGRE)
        {
            failure -= 10;
        }
        break;
        // end item abilities - some possibly mutagenic {dlb}

        // begin invocations {dlb}
    case ABIL_ELYVILON_PURIFICATION:
        invoc = true;
        failure = 20 - (you.piety / 20) - (5 * you.skills[SK_INVOCATIONS]);
        break;

    case ABIL_ZIN_REPEL_UNDEAD:
    case ABIL_TSO_REPEL_UNDEAD:
    case ABIL_KIKU_RECALL_UNDEAD_SLAVES:
    case ABIL_OKAWARU_MIGHT:
    case ABIL_ELYVILON_LESSER_HEALING:
        invoc = true;
        failure = 30 - (you.piety / 20) - (6 * you.skills[SK_INVOCATIONS]);
        break;

    // These three are Trog abilities... Invocations means nothing -- bwr
    case ABIL_TROG_BERSERK:    // piety >= 30
        invoc = true;
        failure = 30 - you.piety;       // starts at 0%
        break;

    case ABIL_TROG_MIGHT:         // piety >= 50
        invoc = true;
        failure = 80 - you.piety;       // starts at 30%
        break;

    case ABIL_TROG_HASTE_SELF:       // piety >= 100
        invoc = true;
        failure = 160 - you.piety;      // starts at 60%
        break;

    case ABIL_YRED_ANIMATE_CORPSE:
        invoc = true;
        failure = 40 - (you.piety / 20) - (3 * you.skills[SK_INVOCATIONS]);
        break;

    case ABIL_ZIN_HEALING:
    case ABIL_TSO_SMITING:
    case ABIL_OKAWARU_HEALING:
    case ABIL_MAKHLEB_MINOR_DESTRUCTION:
    case ABIL_SIF_MUNA_FORGET_SPELL:
    case ABIL_KIKU_ENSLAVE_UNDEAD:
    case ABIL_YRED_ANIMATE_DEAD:
    case ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB:
    case ABIL_ELYVILON_HEALING:
        invoc = true;
        failure = 40 - (you.piety / 20) - (5 * you.skills[SK_INVOCATIONS]);
        break;

    case ABIL_SIF_MUNA_CHANNEL_ENERGY:
        invoc = true;
        failure = 40 - you.intel - you.skills[SK_INVOCATIONS];
        break;

    case ABIL_YRED_RECALL_UNDEAD:
        invoc = true;
        failure = 50 - (you.piety / 20) - (you.skills[SK_INVOCATIONS] * 4);
        break;

    case ABIL_ZIN_PESTILENCE:
    case ABIL_TSO_ANNIHILATE_UNDEAD:
        invoc = true;
        failure = 60 - (you.piety / 20) - (5 * you.skills[SK_INVOCATIONS]);
        break;

    case ABIL_MAKHLEB_MAJOR_DESTRUCTION:
    case ABIL_YRED_DRAIN_LIFE:
        invoc = true;
        failure = 60 - (you.piety / 25) - (you.skills[SK_INVOCATIONS] * 4);
        break;

    case ABIL_ZIN_HOLY_WORD:
    case ABIL_TSO_CLEANSING_FLAME:
    case ABIL_ELYVILON_RESTORATION:
    case ABIL_YRED_CONTROL_UNDEAD:
    case ABIL_OKAWARU_HASTE:
    case ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB:
        invoc = true;
        failure = 70 - (you.piety / 25) - (you.skills[SK_INVOCATIONS] * 4);
        break;

    case ABIL_ZIN_SUMMON_GUARDIAN:
    case ABIL_TSO_SUMMON_DAEVA:
    case ABIL_KIKU_INVOKE_DEATH:
    case ABIL_ELYVILON_GREATER_HEALING:
        invoc = true;
        failure = 80 - (you.piety / 25) - (you.skills[SK_INVOCATIONS] * 4);
        break;

        //jmf: following for to-be-created gods
    case ABIL_CHARM_SNAKE:
        invoc = true;
        failure = 40 - (you.piety / 20) - (3 * you.skills[SK_INVOCATIONS]);
        break;

    case ABIL_TRAN_SERPENT_OF_HELL:
        invoc = true;
        failure = 80 - (you.piety / 25) - (you.skills[SK_INVOCATIONS] * 4);
        break;

    case ABIL_ROTTING:
        invoc = true;
        failure = 60 - (you.piety / 20) - (5 * you.skills[SK_INVOCATIONS]);
        break;

    case ABIL_TORMENT_II:
        invoc = true;
        failure = 70 - (you.piety / 25) - (you.skills[SK_INVOCATIONS] * 4);
        break;

    case ABIL_RENOUNCE_RELIGION:
        invoc = true;
        perfect = true;
        failure = 0;
        break;

        // end invocations {dlb}
    default:
        failure = -1;
        break;
    }

    // Perfect abilities are things like "renounce religion", which
    // shouldn't have a failure rate ever. -- bwr
    if (failure <= 0 && !perfect)
        failure = 1;

    if (failure > 100)
        failure = 100;

    Curr_abil[slot].fail = failure;
    Curr_abil[slot].is_invocation = invoc;

    return (true);
}                               // end insert_ability()

////////////////////////////////////////////////////////////////////////////

static int lugonu_warp_monster(int x, int y, int pow, int)
{
    if (!in_bounds(x, y) || mgrd[x][y] == NON_MONSTER)
        return (0);

    monsters &mon = menv[ mgrd[x][y] ];

    if (!mons_friendly(&mon))
        behaviour_event( &mon, ME_ANNOY, MHITYOU );
    
    if (check_mons_resist_magic(&mon, pow * 2))
    {
        mprf("%s %s.",
             mon.name(DESC_CAP_THE).c_str(), mons_resist_string(&mon));
        return (1);
    }

    const int damage = 1 + random2(pow / 6);
    if (mon.type == MONS_BLINK_FROG)
        mon.heal(damage, false);
    else if (!check_mons_resist_magic(&mon, pow))
    {
        mon.hurt(&you, damage);
        if (!mon.alive())
            return (1);
    }

    mon.blink();

    return (1);
}

static void lugonu_warp_area(int pow)
{
    apply_area_around_square( lugonu_warp_monster, you.x_pos, you.y_pos, pow );
}

static void lugonu_bends_space()
{
    const int pow = 4 + skill_bump(SK_INVOCATIONS);
    const bool area_warp = random2(pow) > 9;

    mprf("Space bends %saround you!", area_warp? "sharply " : "");

    if (area_warp)
        lugonu_warp_area(pow);

    random_blink(false, true);
    
    const int damage = roll_dice(1, 4);
    ouch(damage, 0, KILLED_BY_WILD_MAGIC, "a spatial distortion");
}
