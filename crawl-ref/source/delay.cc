/*
 *  File:     delay.cc
 *  Summary:  Functions for handling multi-turn actions.
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 * <1> Sept 09, 2001     BWR             Created
 */

#include "AppHdr.h"
#include "externs.h"

#include <stdio.h>
#include <string.h>

#include "clua.h"
#include "command.h"
#include "database.h"
#include "delay.h"
#include "enum.h"
#include "food.h"
#include "invent.h"
#include "items.h"
#include "itemname.h"
#include "itemprop.h"
#include "item_use.h"
#include "it_use2.h"
#include "message.h"
#include "misc.h"
#include "monstuff.h"
#include "mon-util.h"
#include "mstuff2.h"
#include "ouch.h"
#include "output.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "spl-util.h"
#include "stash.h"
#include "state.h"
#include "stuff.h"
#include "travel.h"
#include "tutorial.h"
#include "view.h"
#include "xom.h"

extern std::vector<SelItem> items_for_multidrop;

static void xom_check_corpse_waste();
static void armour_wear_effects(const int item_inv_slot);
static void handle_run_delays(const delay_queue_item &delay);
static void handle_macro_delay();
static void finish_delay(const delay_queue_item &delay);

// monster cannot be affected in these states
// (all results of Recite, plus friendly + stupid;
// note that berserk monsters are also hasted)
static bool recite_mons_useless(const monsters *mon)
{
    return (mons_intel(mon->type) < I_NORMAL
            || mons_friendly(mon)
            || mons_is_fleeing(mon)
            || mons_is_sleeping(mon)
            || mons_neutral(mon)
            || mons_is_confused(mon)
            || mons_is_paralysed(mon)
            || mon->has_ench(ENCH_BATTLE_FRENZY)
            || mon->has_ench(ENCH_HASTE));
}

// power is maximum 50
static int recite_to_monsters(int x, int y, int pow, int unused)
{
    UNUSED(unused);

    const int mon = mgrd[x][y];
    if (mon == NON_MONSTER)
        return (0);

    monsters *mons = &menv[mon];

    if (recite_mons_useless(mons))
        return (0);

    if (coinflip()) // nothing happens
        return (0);

    int resist;
    const mon_holy_type holiness = mons_holiness(mons);
    if (holiness == MH_HOLY)
    {
        resist = 7 - random2(you.skills[SK_INVOCATIONS]);
        if (resist < 0)
            resist = 0;
    }
    else
    {
        resist = mons_resist_magic(mons);

        // much lower chances at influencing undead/demons
        if (holiness == MH_UNDEAD)
            pow -= 2 + random2(3);
        else if (holiness == MH_DEMONIC)
            pow -= 3 + random2(5);
    }

    pow -= resist;

    if (pow > 0)
        pow = random2avg(pow,2);

    if (pow <= 0) // Uh oh...
    {
        if (one_chance_in(resist+1))
            return (0);  // nothing happens, whew!

        if (!one_chance_in(4) &&
             mons->add_ench(mon_enchant(ENCH_HASTE, 0, KC_YOU,
                            (16 + random2avg(13, 2)) * 10)))
        {
            simple_monster_message(mons, " speeds up in annoyance!");
        }
        else if (!one_chance_in(3) &&
                 mons->add_ench(mon_enchant(ENCH_BATTLE_FRENZY, 1, KC_YOU,
                                            (16 + random2avg(13, 2)) * 10)))
        {
            simple_monster_message(mons, " goes into a battle-frenzy!");
        }
        else if (mons->can_go_berserk())
            mons->go_berserk(true);
        else
            return (0); // nothing happens

        // bad effects stop the recital
        stop_delay();
        return (1);
    }

    switch (pow)
    {
        case 0:
            return (0); // handled above
        case 1:
        case 2:
        case 3:
        case 4:
            if (!mons_class_is_confusable(mons->type)
                || !mons->add_ench(mon_enchant(ENCH_CONFUSION, 0, KC_YOU,
                                               (16 + random2avg(13, 2)) * 10)))
            {
              return (0);
            }
            simple_monster_message(mons, " looks confused.");
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            mons->put_to_sleep();
            simple_monster_message(mons, " falls asleep!");
            break;
        case 9:
        case 10:
        case 11:
        case 12:
            if (!mons->add_ench(ENCH_NEUTRAL))
                return (0);
            simple_monster_message(mons, " seems impressed!");
            break;
        case 13:
        case 14:
        case 15:
            if (!mons->add_ench(ENCH_FEAR))
                return (0);
            simple_monster_message(mons, " turns to flee.");
            break;
        case 16:
        case 17:
            if (!mons->add_ench(mon_enchant(ENCH_PARALYSIS, 0, KC_YOU,
                                (16 + random2avg(13, 2)) * 10)))
                return (0);
            simple_monster_message(mons, " freezes in fright!");
            break;
        default:
            if (holiness == MH_HOLY)
                good_god_holy_attitude_change(mons);
            else
            {
                if (holiness == MH_UNDEAD || holiness == MH_DEMONIC)
                {
                    if (!mons->add_ench(ENCH_NEUTRAL))
                        return (0);
                    simple_monster_message(mons, " seems impressed!");
                }
                else
                {
                    // permanently neutral
                    mons->attitude = ATT_NEUTRAL;
                    mons->flags |= MF_WAS_NEUTRAL;

                    // give half of the monster's xp
                    unsigned int exp_gain = 0, avail_gain = 0;
                    gain_exp( exper_value(mons) / 2 + 1, &exp_gain, &avail_gain );
                    mons->flags |= MF_GOT_HALF_XP;

                    simple_monster_message(mons, " seems fully impressed!");
                }
            }
            break;
    }

    return (1);
}       // end recite_to_monsters()

static const char* _get_recite_speech(const std::string key, int weight)
{
    seed_rng( weight + you.x_pos + you.y_pos);
    const std::string str = getSpeakString("zin_recite_speech_" + key);

    if (!str.empty())
        return (str.c_str());

    // in case nothing is found
    if (key == "start")
        return ("begin reciting the Axioms of Law.");

    return ("reciting");
}

// Returns true if this delay can act as a parent to other delays, i.e. if
// other delays can be spawned while this delay is running. If is_parent_delay
// returns true, new delays will be pushed immediately to the front of the
// delay in question, rather than at the end of the queue.
static bool is_parent_delay(delay_type delay)
{
    // Interlevel travel can do upstairs/downstairs delays.
    // Lua macros can in theory perform any of the other delays,
    // including travel; in practise travel still assumes there can be
    // no parent delay.
    return (delay == DELAY_TRAVEL
            || delay == DELAY_MACRO
            || delay == DELAY_MULTIDROP);
}

static int push_delay(const delay_queue_item &delay)
{
    for (delay_queue_type::iterator i = you.delay_queue.begin();
            i != you.delay_queue.end();
            ++i)
    {
        if (is_parent_delay( i->type ))
        {
            you.delay_queue.insert(i, delay);
            return (i - you.delay_queue.begin());
        }
    }
    you.delay_queue.push_back( delay );
    return (you.delay_queue.size() - 1);
}

static void pop_delay()
{
    if (!you.delay_queue.empty())
        you.delay_queue.erase( you.delay_queue.begin() );
}

static void clear_pending_delays()
{
    while (you.delay_queue.size() > 1)
    {
        const delay_queue_item delay =
            you.delay_queue[you.delay_queue.size() - 1];
        you.delay_queue.pop_back();

        if (is_run_delay(delay.type) && you.running)
            stop_running();
    }
}

void start_delay( delay_type type, int turns, int parm1, int parm2 )
/***********************************************************/
{
    ASSERT(!crawl_state.is_repeating_cmd() || type == DELAY_MACRO);

    delay_queue_item delay;

    delay.type     = type;
    delay.duration = turns;
    delay.parm1    = parm1;
    delay.parm2    = parm2;
    delay.started  = false;

    // Handle zero-turn delays (possible with butchering).
    if (turns == 0)
    {
        delay.started = true;
        // Don't issue startup message.
        if (push_delay(delay) == 0)
            finish_delay(delay);
        return;
    }
    push_delay( delay );
}

void stop_delay( bool stop_stair_travel )
/*********************/
{
    if (you.delay_queue.empty())
        return;

    delay_queue_item delay = you.delay_queue.front();

    ASSERT(!crawl_state.is_repeating_cmd() || delay.type == DELAY_MACRO);

    // At the very least we can remove any queued delays, right
    // now there is no problem with doing this... note that
    // any queuing here can only happen from a single command,
    // as the effect of a delay doesn't normally allow interaction
    // until it is done... it merely chains up individual actions
    // into a single action.  -- bwr
    clear_pending_delays();

    switch (delay.type)
    {
    case DELAY_BUTCHER:
    case DELAY_BOTTLE_BLOOD:
    case DELAY_OFFER_CORPSE:
    {
        bool multiple_corpses  = false;
        bool butcher_swap_warn = false;
        int wpn_delay = -1;
        for (unsigned int i = 1; i < you.delay_queue.size(); i++)
        {
            if (you.delay_queue[i].type == DELAY_BUTCHER
                || you.delay_queue[i].type == DELAY_BOTTLE_BLOOD
                || you.delay_queue[i].type == DELAY_OFFER_CORPSE)
            {
                multiple_corpses = true;
            }
            else if (you.delay_queue[i].type == DELAY_WEAPON_SWAP)
            {
                wpn_delay = i;
                butcher_swap_warn = true;
                break;
            }
            else
                break;
        }

        const std::string butcher_verb =
                (delay.type == DELAY_BUTCHER      ? "butchering" :
                 delay.type == DELAY_BOTTLE_BLOOD ? "bottling blood from"
                                                  : "sacrificing");

        // Corpse keeps track of work in plus2 field, see handle_delay() -- bwr
        if (butcher_swap_warn)
        {
            const int butcher_swap_weapon = you.delay_queue[wpn_delay].parm1;

            std::string weapon;
            if (butcher_swap_weapon == -1)
                weapon = "unarmed combat";
            else
            {
                weapon = "your " +
                    you.inv[butcher_swap_weapon].name(DESC_BASENAME);
            }
            mprf(MSGCH_WARN, "You stop %s the corpse%s; not switching "
                             "back to %s.", butcher_verb.c_str(),
                             (multiple_corpses ? "s" : ""), weapon.c_str());
        }
        else
            mprf("You stop %s the corpse%s.", butcher_verb.c_str(),
                 multiple_corpses ? "s" : "");

        pop_delay();
        break;
    }
    case DELAY_MEMORISE:
        // Losing work here is okay... having to start from
        // scratch is a reasonable behaviour. -- bwr
        mpr( "Your memorisation is interrupted." );
        pop_delay();
        break;

    case DELAY_PASSWALL:
        // The lost work here is okay since this spell requires
        // the player to "attune to the rock".  If changed, then
        // the delay should be increased to reduce the power of
        // this spell. -- bwr
        mpr( "Your meditation is interrupted." );
        pop_delay();
        break;

    case DELAY_MULTIDROP:
        // No work lost
        if (!items_for_multidrop.empty())
            mpr( "You stop dropping stuff." );
        pop_delay();
        break;

    case DELAY_RECITE:
        mprf(MSGCH_PLAIN, "You stop %s.",
             _get_recite_speech("other", you.num_turns + delay.duration));
        pop_delay();
        break;

    case DELAY_RUN:
    case DELAY_REST:
    case DELAY_TRAVEL:
    case DELAY_MACRO:
        // Always interruptible.
        pop_delay();

        // Keep things consistent, otherwise disturbing phenomena can occur.
        // Note that runrest::stop() will turn around and call stop_delay()
        // again, but that's okay because the delay is already popped off
        // the queue.
        if (is_run_delay(delay.type) && you.running)
            stop_running();

        // There's no special action needed for macros - if we don't call out
        // to the Lua function, it can't do damage.
        break;

    case DELAY_INTERRUPTIBLE:
        // always stoppable by definition...
        // try using a more specific type anyways. -- bwr
        pop_delay();
        break;

    case DELAY_EAT:
        // XXX: Large problems with object destruction here... food can
        // be from in the inventory or on the ground and these are
        // still handled quite differently.  Eventually we would like
        // this to be stoppable, with partial food items implemented. -- bwr
        break;

    case DELAY_FEED_VAMPIRE:
    {
        mpr("You stop draining the corpse.");
        xom_check_corpse_waste();
        item_def &corpse = (delay.parm1 ? you.inv[delay.parm2]
                                        : mitm[delay.parm2]);

        if (!mons_skeleton( corpse.plus ))
        {
            if (delay.parm1)
                dec_inv_item_quantity( delay.parm2, 1 );
            else
                dec_mitm_item_quantity( delay.parm2, 1 );
        }
        else
        {
            mpr("All blood oozes out of the corpse!");
            bleed_onto_floor(you.x_pos, you.y_pos, corpse.plus, delay.duration,
                             false);
            corpse.sub_type = CORPSE_SKELETON;
            corpse.special  = 90;
            corpse.colour   = LIGHTGREY;
        }
        did_god_conduct(DID_DRINK_BLOOD, 8);
        pop_delay();
        break;
    }
    case DELAY_ARMOUR_ON:
    case DELAY_ARMOUR_OFF:
        // These two have the default action of not being interruptible,
        // although they will often be chained (remove cloak, remove
        // armour, wear new armour, replace cloak), all of which can
        // be stopped when complete.  This is a fairly reasonable
        // behaviour, although perhaps the character should have
        // option of reversing the current action if it would take
        // less time to get out of the plate mail that's half on
        // than it would take to continue.  Probably too much trouble,
        // and would have to have a prompt... this works just fine. -- bwr
        break;

    case DELAY_ASCENDING_STAIRS:  // short... and probably what people want
    case DELAY_DESCENDING_STAIRS: // short... and probably what people want
         if (stop_stair_travel)
         {
#ifdef DEBUG_DIAGNOSTICS
             mpr("Stop ascending/descending stairs.");
#endif
             pop_delay();
         }
         break;

    case DELAY_WEAPON_SWAP:       // one turn... too much trouble
    case DELAY_DROP_ITEM:         // one turn... only used for easy armour drops
    case DELAY_JEWELLERY_ON:      // one turn
    case DELAY_UNINTERRUPTIBLE:   // never stoppable
    default:
        break;
    }

    if (is_run_delay(delay.type))
        update_turn_count();
}

void stop_butcher_delay()
{
    if (current_delay_action() == DELAY_BUTCHER
        || current_delay_action() == DELAY_BOTTLE_BLOOD
        || current_delay_action() == DELAY_OFFER_CORPSE)
    {
        stop_delay();
    }
}

bool you_are_delayed( void )
/**************************/
{
    return (!you.delay_queue.empty());
}

delay_type current_delay_action( void )
/******************************/
{
    return (you_are_delayed() ? you.delay_queue.front().type
                              : DELAY_NOT_DELAYED);
}

bool is_run_delay(int delay)
{
    return (delay == DELAY_RUN || delay == DELAY_REST || delay == DELAY_TRAVEL);
}

bool is_being_butchered(const item_def &item)
{
    if (!you_are_delayed())
        return (false);

    const delay_queue_item &delay = you.delay_queue.front();
    if (delay.type == DELAY_BUTCHER || delay.type == DELAY_BOTTLE_BLOOD)
    {
        const item_def &corpse = mitm[ delay.parm1 ];
        return (&corpse == &item);
    }

    return (false);
}

bool is_vampire_feeding()
{
    if (!you_are_delayed())
        return (false);

    const delay_queue_item &delay = you.delay_queue.front();
    return (delay.type == DELAY_FEED_VAMPIRE);
}

// check whether there are monsters who might be influenced by Recite
// return 0, if no monsters found
// return 1, if eligible audience found
// return -1, if entire audience already affected or too dumb to understand
int check_recital_audience()
{
    int mid;
    monsters *mons;
    bool found_monsters = false;

    for (int x = you.x_pos - 8; x <= you.x_pos + 8; x++)
       for (int y = you.y_pos - 8; y <= you.y_pos + 8; y++)
       {
            if (!in_bounds(x,y) || !see_grid(x, y))
                continue;

            mid = mgrd[x][y];
            if (mid == NON_MONSTER)
                continue;

            mons = &menv[mid];
            if (!found_monsters)
                found_monsters = true;

            // can not be affected in these states
            if (recite_mons_useless(mons))
                continue;

            return (1);
      }

#ifdef DEBUG_DIAGNOSTICS
      if (!found_monsters)
          mprf(MSGCH_DIAGNOSTICS, "No audience found!");
      else
          mprf(MSGCH_DIAGNOSTICS, "No sensible audience found!");
#endif

   // no use preaching to the choir, nor to common animals
   if (found_monsters)
       return (-1);

   // Sorry, no audience found!
   return (0);
}

// Xom is amused by a potential food source going to waste, and is
// more amused the hungrier you are.
static void xom_check_corpse_waste()
{
    int food_need = 7000 - you.hunger;
    if (food_need < 0)
        food_need = 0;

    xom_is_stimulated(64 + (191 * food_need / 6000));
}

void handle_delay( void )
/***********************/
{
    if (!you_are_delayed())
        return;

    delay_queue_item &delay = you.delay_queue.front();

    if (!delay.started)
    {
        switch (delay.type)
        {
        case DELAY_ARMOUR_ON:
            mpr("You start putting on your armour.", MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_ARMOUR_OFF:
            mpr("You start removing your armour.", MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_BUTCHER:
        case DELAY_BOTTLE_BLOOD:
            mprf(MSGCH_MULTITURN_ACTION, "You start %s the %s.",
                 (delay.type == DELAY_BOTTLE_BLOOD ? "bottling blood from"
                                                   : "butchering"),
                 mitm[delay.parm1].name(DESC_PLAIN).c_str());

            // also for bottling blood
            if (you.duration[DUR_PRAYER]
                && god_hates_butchery(you.religion))
            {
                did_god_conduct(DID_DEDICATED_BUTCHERY, 10);
            }
            break;
        case DELAY_MEMORISE:
            mpr("You start memorising the spell.", MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_PASSWALL:
            mpr("You begin to meditate on the wall.", MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_RECITE:
            mprf(MSGCH_PLAIN, "You %s",
                 _get_recite_speech("start", you.num_turns + delay.duration));
            if (apply_area_visible(recite_to_monsters, delay.parm1))
                viewwindow(true, false);
            break;
        case DELAY_FEED_VAMPIRE:
        {
            item_def &corpse = (delay.parm1 ? you.inv[delay.parm2]
                                            : mitm[delay.parm2]);
            vampire_nutrition_per_turn(corpse, -1);
            break;
        }
        default:
            break;
        }
        delay.started = true;
    }

    ASSERT(!crawl_state.is_repeating_cmd() || delay.type == DELAY_MACRO);

    // Run delays and Lua delays don't have a specific end time.
    if (is_run_delay(delay.type))
    {
        // Hack - allow autoprayer to trigger during run delays
        if ( do_autopray() )
            return;

        handle_run_delays(delay);
        return;
    }

    if (delay.type == DELAY_MACRO)
    {
        handle_macro_delay();
        return;
    }

    // First check cases where delay may no longer be valid:
    // XXX: need to handle passwall when monster digs -- bwr
    if (delay.type == DELAY_BUTCHER || delay.type == DELAY_BOTTLE_BLOOD
        || delay.type == DELAY_OFFER_CORPSE)
    {
        if (delay.type == DELAY_BOTTLE_BLOOD && you.experience_level < 6)
        {
            mpr("You cannot bottle blood anymore!");
            stop_delay();
            return;
        }

        // A monster may have raised the corpse you're chopping up! -- bwr
        // Note that a monster could have raised the corpse and another
        // monster could die and create a corpse with the same ID number...
        // However, it would not be at the player's square like the
        // original and that's why we do it this way.
        if (is_valid_item(mitm[ delay.parm1 ])
            && mitm[ delay.parm1 ].base_type == OBJ_CORPSES
            && mitm[ delay.parm1 ].x == you.x_pos
            && mitm[ delay.parm1 ].y == you.y_pos )
        {
            if (mitm[ delay.parm1 ].sub_type == CORPSE_SKELETON)
            {
                mpr("The corpse rots away into a skeleton!");
                if (delay.type == DELAY_BUTCHER
                    || delay.type == DELAY_BOTTLE_BLOOD)
                {
                    if (player_mutation_level(MUT_SAPROVOROUS) == 3)
                        xom_check_corpse_waste();
                    else
                        xom_is_stimulated(32);
                    delay.duration = 0;
                }
                else
                {
                    // Don't attempt to offer a skeleton.
                    pop_delay();

                    // Chain onto the next delay.
                    handle_delay();
                    return;
                }
            }
            else
            {
                // Only give the rotting message if the corpse wasn't
                // previously rotten. (special < 100 is the rottenness check)
                if (food_is_rotten(mitm[delay.parm1]) && delay.parm2 >= 100)
                {
                    mpr("The corpse rots.", MSGCH_ROTTEN_MEAT);
                    if (delay.type == DELAY_OFFER_CORPSE)
                    {
                        // don't attempt to offer a rotten corpse
                        pop_delay();

                        // Chain onto the next delay.
                        handle_delay();
                        return;
                    }

                    delay.parm2 = 99; // don't give the message twice

                    if (you.is_undead != US_UNDEAD
                        && player_mutation_level(MUT_SAPROVOROUS) < 3)
                    {
                        xom_check_corpse_waste();
                    }
                    // Vampires won't continue bottling rotting corpses.
                    if (delay.type == DELAY_BOTTLE_BLOOD)
                    {
                        mpr("You stop bottling this corpse's foul-smelling "
                            "blood!");
                        delay.duration = 0;
                    }
                }

                // mark work done on the corpse in case we stop -- bwr
                mitm[ delay.parm1 ].plus2++;
            }
        }
        else if (delay.type == DELAY_OFFER_CORPSE)
        {
            mprf("Corpse %d no longer valid!", delay.parm1);
            // don't attempt to offer an invalid item
            pop_delay();

            // Chain onto the next delay.
            handle_delay();
            return;
        }
        else
        {
            // corpse is no longer valid! End the butchering normally
            // instead of using stop_delay() so that the player switches
            // back to their main weapon if necessary.
            delay.duration = 0;
        }

        if (delay.type == DELAY_OFFER_CORPSE && !you.duration[DUR_PRAYER]
            && do_autopray())
        {
            return;
        }
    }
    else if (delay.type == DELAY_MULTIDROP)
    {
        // Throw away invalid items; items usually go invalid because
        // of chunks rotting away.
        while (!items_for_multidrop.empty()
               // Don't look for gold in inventory
               && items_for_multidrop[0].slot != PROMPT_GOT_SPECIAL
               && !is_valid_item(you.inv[ items_for_multidrop[0].slot ]))
        {
            items_for_multidrop.erase( items_for_multidrop.begin() );
        }

        if ( items_for_multidrop.empty() )
        {
            // ran out of things to drop
            pop_delay();
            return;
        }
    }
    else if (delay.type == DELAY_RECITE)
    {
        if (check_recital_audience() < 1 // maybe you've lost your audience
            || Options.hp_warning && you.hp*Options.hp_warning <= you.hp_max
               && delay.parm2*Options.hp_warning > you.hp_max
            || you.hp*2 < delay.parm2) // or significant health drop
        {
            stop_delay();
            return;
        }
    }

    // Handle delay:
    if (delay.duration > 0)
    {
#if DEBUG_DIAGNOSTICS
        mprf(MSGCH_DIAGNOSTICS, "Delay type: %d (%s), duration: %d",
             delay.type, delay_name(delay.type), delay.duration );
#endif
        // delay.duration-- *must* be done before multidrop, because
        // multidrop is now a parent delay, which means other delays
        // can be pushed to the front of the queue, invalidating the
        // "delay" reference here, and resulting in tons of debugging
        // fun with valgrind.
        delay.duration--;

        switch ( delay.type )
        {
        case DELAY_ARMOUR_ON:
            mprf(MSGCH_MULTITURN_ACTION, "You continue putting on %s.",
                 you.inv[delay.parm1].name(DESC_NOCAP_YOUR).c_str());
            break;
        case DELAY_ARMOUR_OFF:
            mprf(MSGCH_MULTITURN_ACTION, "You continue taking off %s.",
                 you.inv[delay.parm1].name(DESC_NOCAP_YOUR).c_str());
            break;
        case DELAY_BUTCHER:
            mprf(MSGCH_MULTITURN_ACTION, "You continue butchering the corpse.");
            break;
        case DELAY_BOTTLE_BLOOD:
            mprf(MSGCH_MULTITURN_ACTION, "You continue bottling blood from "
                                         "the corpse.");
            break;
        case DELAY_MEMORISE:
            mpr("You continue memorising.", MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_PASSWALL:
            mpr("You continue meditating on the rock.",
                MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_RECITE:
            mprf(MSGCH_MULTITURN_ACTION, "You continue %s.",
                 _get_recite_speech("other", you.num_turns + delay.duration+1));
            if (apply_area_visible(recite_to_monsters, delay.parm1))
                viewwindow(true, false);
            break;
        case DELAY_MULTIDROP:
            drop_item( items_for_multidrop[0].slot,
                       items_for_multidrop[0].quantity,
                       items_for_multidrop.size() == 1 );
            items_for_multidrop.erase( items_for_multidrop.begin() );
            break;
        case DELAY_EAT:
            mpr("You continue eating.", MSGCH_MULTITURN_ACTION);
            break;
        case DELAY_FEED_VAMPIRE:
        {
            item_def &corpse = (delay.parm1 ? you.inv[delay.parm2]
                                            : mitm[delay.parm2]);
            if (food_is_rotten(corpse))
            {
                mpr("This corpse has started to rot.", MSGCH_ROTTEN_MEAT);
                xom_check_corpse_waste();
                stop_delay();
                return;
            }
            mprf(MSGCH_MULTITURN_ACTION, "You continue drinking.");
            vampire_nutrition_per_turn(corpse, 0);
            break;
        }
        default:
            break;
        }
    }
    else
    {
        finish_delay(delay);
    }
}

static void finish_delay(const delay_queue_item &delay)
{
    switch (delay.type)
    {
    case DELAY_WEAPON_SWAP:
        weapon_switch( delay.parm1 );
        break;

    case DELAY_JEWELLERY_ON:
        puton_ring( delay.parm1, false );
        break;

    case DELAY_ARMOUR_ON:
        armour_wear_effects( delay.parm1 );
        break;

    case DELAY_ARMOUR_OFF:
    {
        mprf("You finish taking off %s.",
             you.inv[delay.parm1].name(DESC_NOCAP_YOUR).c_str());

        const equipment_type slot =
            get_armour_slot( you.inv[delay.parm1] );

        if (slot == EQ_BODY_ARMOUR)
        {
            you.equip[EQ_BODY_ARMOUR] = -1;
        }
        else
        {
            switch (slot)
            {
            case EQ_SHIELD:
                if (delay.parm1 == you.equip[EQ_SHIELD])
                    you.equip[EQ_SHIELD] = -1;
                break;

            case EQ_CLOAK:
                if (delay.parm1 == you.equip[EQ_CLOAK])
                    you.equip[EQ_CLOAK] = -1;
                break;

            case EQ_HELMET:
                if (delay.parm1 == you.equip[EQ_HELMET])
                    you.equip[EQ_HELMET] = -1;
                break;

            case EQ_GLOVES:
                if (delay.parm1 == you.equip[EQ_GLOVES])
                    you.equip[EQ_GLOVES] = -1;
                break;

            case EQ_BOOTS:
                if (delay.parm1 == you.equip[EQ_BOOTS])
                    you.equip[EQ_BOOTS] = -1;
                break;

            default:
                break;
            }
        }

        unwear_armour( delay.parm1 );

        you.redraw_armour_class = true;
        you.redraw_evasion = true;
        break;
    }
    case DELAY_EAT:
        mprf("You finish eating.");
        // For chunks, warn the player if they're not getting much
        // nutrition.
        if (delay.parm1)
            chunk_nutrition_message(delay.parm1);
        break;

    case DELAY_FEED_VAMPIRE:
    {
        mprf("You finish drinking.");
        did_god_conduct(DID_DRINK_BLOOD, 8);

        item_def &corpse = (delay.parm1 ? you.inv[delay.parm2]
                                        : mitm[delay.parm2]);
        vampire_nutrition_per_turn(corpse, 1);

        if (!mons_skeleton( corpse.plus ))
        {
            if (delay.parm1)
                dec_inv_item_quantity( delay.parm2, 1 );
            else
                dec_mitm_item_quantity( delay.parm2, 1 );
        }
        else if (!one_chance_in(4))
        {
            corpse.sub_type = CORPSE_SKELETON;
            corpse.special = 90;
            corpse.colour = LIGHTGREY;
        }
        break;
    }
    case DELAY_MEMORISE:
        mpr( "You finish memorising." );
        add_spell_to_memory( static_cast<spell_type>( delay.parm1 ) );
        break;

    case DELAY_RECITE:
        mprf(MSGCH_PLAIN, "You finish %s.",
             _get_recite_speech("other", you.num_turns + delay.duration));
        break;

    case DELAY_PASSWALL:
    {
        mpr( "You finish merging with the rock." );
        more();  // or the above message won't be seen

        const int pass_x = delay.parm1;
        const int pass_y = delay.parm2;

        if (pass_x != 0 && pass_y != 0)
        {

            switch (grd[ pass_x ][ pass_y ])
            {
            default:
                if (!you.can_pass_through_feat(grd[pass_x][pass_y]))
                    ouch(1 + you.hp, 0, KILLED_BY_PETRIFICATION);
                break;

            case DNGN_SECRET_DOOR:      // oughtn't happen
            case DNGN_CLOSED_DOOR:      // open the door
                grd[ pass_x ][ pass_y ] = DNGN_OPEN_DOOR;
                break;
            }

            // move any monsters out of the way:
            int mon = mgrd[ pass_x ][ pass_y ];
            if (mon != NON_MONSTER)
            {
                // one square, a few squares, anywhere...
                if (!shift_monster(&menv[mon])
                    && !monster_blink(&menv[mon]))
                {
                    monster_teleport( &menv[mon], true, true );
                }
            }

            move_player_to_grid(pass_x, pass_y, false, true, true);
            redraw_screen();
        }
        break;
    }

    case DELAY_BUTCHER:
    case DELAY_BOTTLE_BLOOD:
    {
        const item_def &item = mitm[delay.parm1];
        if (is_valid_item(item) && item.base_type == OBJ_CORPSES)
        {
            if (item.sub_type == CORPSE_SKELETON)
            {
                mprf("The corpse rots away into a skeleton just before you "
                     "finish %s!",
                     (delay.type == DELAY_BOTTLE_BLOOD ? "bottling its blood"
                                                       : "butchering"));

                if (player_mutation_level(MUT_SAPROVOROUS) == 3)
                    xom_check_corpse_waste();
                else
                    xom_is_stimulated(64);

                break;
            }

            if (delay.type == DELAY_BOTTLE_BLOOD)
            {
                turn_corpse_into_blood_potions( mitm[ delay.parm1 ] );
            }
            else
            {
                mprf("You finish %s the %s into pieces.",
                     (you.has_usable_claws()
                      || player_mutation_level(MUT_FANGS) == 3
                         && you.species != SP_VAMPIRE) ? "ripping"
                                                       : "chopping",
                     mitm[delay.parm1].name(DESC_PLAIN).c_str());

                if (is_good_god(you.religion) && is_player_same_species(item.plus))
                {
                    simple_god_message(" expects more respect for your departed "
                                       "relatives.");
                }
                else if (you.religion == GOD_ZIN && mons_intel(item.plus) >= I_NORMAL)
                {
                    simple_god_message(" expects more respect for this departed "
                                       "soul.");
                }

                if (you.species == SP_VAMPIRE && delay.type == DELAY_BUTCHER
                    && mons_has_blood(item.plus) && !food_is_rotten(item))
                {
                    mpr("What a waste.");
                }
                turn_corpse_into_chunks( mitm[ delay.parm1 ] );

                if (you.duration[DUR_BERSERKER] &&
                    you.berserk_penalty != NO_BERSERK_PENALTY)
                {
                    mpr("You enjoyed that.");
                    you.berserk_penalty = 0;
                }
            }
        }
        else
        {
            mprf("You stop %s.", can_bottle_blood_from_corpse(item.plus) ?
                                 "bottling this corpse's blood"
                                 : "butchering the corpse");
        }
        StashTrack.update_stash(); // Stash-track the generated item(s)
        break;
    }

    case DELAY_OFFER_CORPSE:
    {
        if (!you.duration[DUR_PRAYER] && !do_autopray())
        {
            stop_delay();
            return;
        }

        offer_corpse(delay.parm1);
        break;
    }
    case DELAY_DROP_ITEM:
        // Note:  checking if item is droppable is assumed to
        // be done before setting up this delay... this includes
        // quantity (delay.parm2). -- bwr

        // Make sure item still exists.
        if (!is_valid_item( you.inv[ delay.parm1 ] ))
            break;

        // Must handle unwield_item before we attempt to copy
        // so that temporary brands and such are cleared. -- bwr
        if (delay.parm1 == you.equip[EQ_WEAPON])
        {
            unwield_item();
            canned_msg( MSG_EMPTY_HANDED );
        }

        if (!copy_item_to_grid( you.inv[ delay.parm1 ],
                                you.x_pos, you.y_pos, delay.parm2,
                                true ))
        {
            mpr("Too many items on this level, not dropping the item.");
        }
        else
        {
            mprf("You drop %s.", quant_name(you.inv[delay.parm1], delay.parm2,
                                            DESC_NOCAP_A).c_str());
            dec_inv_item_quantity( delay.parm1, delay.parm2 );
        }
        break;

    case DELAY_ASCENDING_STAIRS:
        up_stairs();
        break;

    case DELAY_DESCENDING_STAIRS:
        down_stairs( delay.parm1 );
        break;

    case DELAY_INTERRUPTIBLE:
    case DELAY_UNINTERRUPTIBLE:
        // these are simple delays that have no effect when complete
        break;

    default:
        mpr( "You finish doing something." );
        break;
    }

    you.wield_change = true;
    print_stats();  // force redraw of the stats
    pop_delay();

    // Chain onto the next delay.
    handle_delay();
}

static void armour_wear_effects(const int item_slot)
{
    item_def &arm = you.inv[item_slot];

    set_ident_flags(arm, ISFLAG_EQ_ARMOUR_MASK );
    mprf("You finish putting on %s.", arm.name(DESC_NOCAP_YOUR).c_str());

    const equipment_type eq_slot      = get_armour_slot(arm);
    const bool           known_cursed = item_known_cursed(arm);

    if (eq_slot == EQ_BODY_ARMOUR)
    {
        you.equip[EQ_BODY_ARMOUR] = item_slot;

        if (you.duration[DUR_ICY_ARMOUR] != 0)
        {
            mpr( "Your icy armour melts away.", MSGCH_DURATION );
            you.redraw_armour_class = 1;
            you.duration[DUR_ICY_ARMOUR] = 0;
        }
    }
    else
    {
        switch (eq_slot)
        {
        case EQ_SHIELD:
            if (you.duration[DUR_CONDENSATION_SHIELD])
            {
                mpr( "Your icy shield evaporates.", MSGCH_DURATION );
                you.duration[DUR_CONDENSATION_SHIELD] = 0;
            }
            you.equip[EQ_SHIELD] = item_slot;
            break;
        case EQ_CLOAK:
            you.equip[EQ_CLOAK] = item_slot;
            break;
        case EQ_HELMET:
            you.equip[EQ_HELMET] = item_slot;
            break;
        case EQ_GLOVES:
            you.equip[EQ_GLOVES] = item_slot;
            break;
        case EQ_BOOTS:
            you.equip[EQ_BOOTS] = item_slot;
            break;
        default:
            break;
        }
    }

    int ego = get_armour_ego_type( arm );
    if (ego != SPARM_NORMAL)
    {
        switch (ego)
        {
        case SPARM_RUNNING:
            mprf("You feel quick%s.",
                   (you.species == SP_NAGA || player_mutation_level(MUT_HOOVES))
                    ? "" : " on your feet");
            break;

        case SPARM_FIRE_RESISTANCE:
            mpr("You feel resistant to fire.");
            break;

        case SPARM_COLD_RESISTANCE:
            mpr("You feel resistant to cold.");
            break;

        case SPARM_POISON_RESISTANCE:
            mpr("You feel healthy.");
            break;

        case SPARM_SEE_INVISIBLE:
            mpr("You feel perceptive.");
            break;

        case SPARM_DARKNESS:
            if (!you.duration[DUR_INVIS])
                mpr("You become transparent for a moment.");
            break;

        case SPARM_STRENGTH:
            modify_stat(STAT_STRENGTH, 3, false, arm);
            break;

        case SPARM_DEXTERITY:
            modify_stat(STAT_DEXTERITY, 3, false, arm);
            break;

        case SPARM_INTELLIGENCE:
            modify_stat(STAT_INTELLIGENCE, 3, false, arm);
            break;

        case SPARM_PONDEROUSNESS:
            mpr("You feel rather ponderous.");
            // you.speed += 2;
            you.redraw_evasion = 1;
            break;

        case SPARM_LEVITATION:
            mpr("You feel rather light.");
            break;

        case SPARM_MAGIC_RESISTANCE:
            mpr("You feel resistant to magic.");
            break;

        case SPARM_PROTECTION:
            mpr("You feel protected.");
            break;

        case SPARM_STEALTH:
            mpr("You feel stealthy.");
            break;

        case SPARM_RESISTANCE:
            mpr("You feel resistant to extremes of temperature.");
            break;

        case SPARM_POSITIVE_ENERGY:
            mpr("Your life-force is being protected.");
            break;

        case SPARM_ARCHMAGI:
            if (!you.skills[SK_SPELLCASTING])
                mpr("You feel strangely numb.");
            else
                mpr("You feel extremely powerful.");
            break;
        }
    }

    if (is_random_artefact( arm ))
        use_randart( item_slot );

    if (item_cursed( arm ))
    {
        mpr( "Oops, that feels deathly cold." );
        learned_something_new(TUT_YOU_CURSED);

        // Cursed cloaks prevent you from removing body armour
        int cloak_mult = 1;
        if (get_armour_slot(arm) == EQ_CLOAK)
            cloak_mult = 2;

        if (known_cursed)
            xom_is_stimulated(32 * cloak_mult);
        else
            xom_is_stimulated(64 * cloak_mult);
    }

    if (eq_slot == EQ_SHIELD)
        warn_shield_penalties();

    you.redraw_armour_class = true;
    you.redraw_evasion = true;
}

static command_type get_running_command()
{
    if ( kbhit() )
    {
        stop_running();
        return CMD_NO_CMD;
    }
    if ( is_resting() )
    {
        you.running.rest();
        if ( !is_resting() && you.running.hp == you.hp
             && you.running.mp == you.magic_points )
        {
            mpr("Done searching.");
        }
        return CMD_MOVE_NOWHERE;
    }
    else if (Options.travel_delay > 0)
    {
        delay(Options.travel_delay);
    }

    return direction_to_command( you.running.x, you.running.y );
}

static void handle_run_delays(const delay_queue_item &delay)
{
    // Handle inconsistencies between the delay queue and you.running.
    // We don't want to send the game into a deadlock.
    if (!you.running)
    {
        update_turn_count();
        pop_delay();
        return;
    }

    if (you.turn_is_over)
        return;

    command_type cmd = CMD_NO_CMD;
    switch (delay.type)
    {
    case DELAY_REST:
    case DELAY_RUN:
        cmd = get_running_command();
        break;
    case DELAY_TRAVEL:
        cmd = travel();
        break;
    default:
        break;
    }

    if (cmd != CMD_NO_CMD)
    {
        if ( delay.type != DELAY_REST )
            mesclr();
        process_command(cmd);
    }

    // If you.running has gone to zero, and the run delay was not
    // removed, remove it now. This is needed to clean up after
    // find_travel_pos() function in travel.cc.
    if (!you.running && is_run_delay(current_delay_action()))
    {
        pop_delay();
        update_turn_count();
    }

    if (you.running && !you.turn_is_over
        && you_are_delayed()
        && !is_run_delay(current_delay_action()))
    {
        handle_delay();
    }
}

static void handle_macro_delay()
{
    run_macro();
}

void run_macro(const char *macroname)
{
    const int currdelay = current_delay_action();
    if (currdelay != DELAY_NOT_DELAYED && currdelay != DELAY_MACRO)
        return;

#ifdef CLUA_BINDINGS
    if (!clua)
    {
        mpr("Lua not initialized", MSGCH_DIAGNOSTICS);
        stop_delay();
        return;
    }

    if (!clua.callbooleanfn(false, "c_macro", "s", macroname))
    {
        if (clua.error.length())
            mprf(MSGCH_ERROR, "Lua error: %s", clua.error.c_str());

        stop_delay();
    }
    else
    {
        start_delay(DELAY_MACRO, 1);
    }
#else
    stop_delay();
#endif
}

// Returns 1 if the delay should be interrupted, 0 if the user function
// had no opinion on the matter, -1 if the delay should not be interrupted.
static int userdef_interrupt_activity( const delay_queue_item &idelay,
                                        activity_interrupt_type ai,
                                        const activity_interrupt_data &at )
{
#ifdef CLUA_BINDINGS
    const int delay = idelay.type;
    lua_State *ls = clua.state();
    if (!ls || ai == AI_FORCE_INTERRUPT)
        return (true);

    // Kludge: We have to continue to support ch_stop_run. :-(
    if (is_run_delay(delay) && you.running && ai == AI_SEE_MONSTER)
    {
        bool stop_run = false;
        if (clua.callfn("ch_stop_run", "M>b",
                        (const monsters *) at.data, &stop_run))
        {
            if (stop_run)
                return (true);

            // No further processing.
            return (-1);
        }

        // If we get here, ch_stop_run wasn't defined, fall through to the
        // other handlers.
    }

    const char *interrupt_name = activity_interrupt_name(ai);
    const char *act_name = delay_name(delay);

    bool ran = clua.callfn("c_interrupt_activity", "1:ssA",
                    act_name, interrupt_name, &at);
    if (ran)
    {
        // If the function returned nil, we want to cease processing.
        if (lua_isnil(ls, -1))
        {
            lua_pop(ls, 1);
            return (-1);
        }

        bool stopact = lua_toboolean(ls, -1);
        lua_pop(ls, 1);
        if (stopact)
            return (true);
    }

    if (delay == DELAY_MACRO && clua.callbooleanfn(true, "c_interrupt_macro",
                                                   "sA", interrupt_name, &at))
    {
        return (true);
    }

#endif
    return (false);
}

// Returns true if the activity should be interrupted, false otherwise.
static bool should_stop_activity(const delay_queue_item &item,
                                 activity_interrupt_type ai,
                                 const activity_interrupt_data &at)
{
    int userd = userdef_interrupt_activity(item, ai, at);

    // If the user script wanted to stop the activity or cease processing,
    // do so.
    if (userd)
        return (userd == 1);

    delay_type curr = current_delay_action();

    if (curr != DELAY_REST && (ai == AI_FULL_HP || ai == AI_FULL_MP))
        return false;
    else if (ai == AI_SEE_MONSTER && (curr == DELAY_ASCENDING_STAIRS ||
                                      curr == DELAY_DESCENDING_STAIRS))
        return false;

    return (ai == AI_FORCE_INTERRUPT
            || Options.activity_interrupts[item.type][ai]);
}

inline static void monster_warning(activity_interrupt_type ai,
                                   const activity_interrupt_data &at,
                                   int atype)
{
    if (ai == AI_SEE_MONSTER && is_run_delay(atype))
    {
        const monsters* mon = static_cast<const monsters*>(at.data);
        if (!mon->visible())
            return;
        if (at.context == "already seen")
        {
            // Only say "comes into view" if the monster wasn't in view
            // during the previous turn.
            if (testbits(mon->flags, MF_WAS_IN_VIEW))
            {
                mprf(MSGCH_WARN, "%s is too close now for your liking.",
                     mon->name(DESC_CAP_THE).c_str());
            }
        }
        else
        {
            std::string text = mon->name(DESC_CAP_A);
            // For named monsters also mention the base type.
            if (!(mon->mname).empty())
                text += " " + mons_type_name(mon->type, DESC_NOCAP_THE);

            if (at.context == "thin air")
            {
                if (mon->type == MONS_AIR_ELEMENTAL)
                    text += " forms itself from the air.";
                else
                    text += " appears from thin air.";
            }
            else if (at.context == "surfaces")
                text += " surfaces.";
            else if (at.context.find("bursts forth") != std::string::npos)
            {
                text += " bursts forth from the ";
                if (mons_habitat(mon) == HT_LAVA)
                    text += "lava";
                else if (mons_habitat(mon) == HT_WATER)
                    text += "water";
                else
                    text += "realm of bugdom";
                text += ".";
            }
            else
                text += " comes into view.";

            const std::string mweap =
                get_monster_desc(mon, false, DESC_NONE);

            if (!mweap.empty())
            {
                text += " " + mon->pronoun(PRONOUN_CAP)
                        + " is" + mweap + ".";
            }
            print_formatted_paragraph(text,
                                      get_number_of_cols(),
                                      MSGCH_WARN);
        }

        if (Options.tutorial_left)
        {
            // enforce that this message comes first
            tutorial_first_monster(*mon);
            if (get_mons_colour(mon) != mon->colour)
                learned_something_new(TUT_MONSTER_BRAND);
        }
    }
}

static void paranoid_option_disable( activity_interrupt_type ai,
                                     const activity_interrupt_data &at )
{
    if (ai == AI_HIT_MONSTER || ai == AI_MONSTER_ATTACKS)
    {
        const monsters* mon = static_cast<const monsters*>(at.data);
        if (mon && !player_monster_visible(mon) && !mons_is_submerged(mon))
        {
            std::vector<std::string> deactivatees;
            std::vector<std::string> restart;
            if (Options.autoprayer_on)
            {
                deactivatees.push_back("autoprayer");
                Options.autoprayer_on = false;
                restart.push_back("Ctrl+V");
            }

            if (Options.autopickup_on)
            {
                deactivatees.push_back("autopickup");
                Options.autopickup_on = false;
                restart.push_back("Ctrl+A");
            }

            if (!deactivatees.empty())
            {
                mprf(MSGCH_WARN, "Deactivating %s; reactivate with %s.",
                      comma_separated_line(deactivatees.begin(),
                                           deactivatees.end()).c_str(),
                      comma_separated_line(restart.begin(),
                                           restart.end()).c_str());
            }

            if (Options.tutorial_left)
            {
                learned_something_new(TUT_INVISIBLE_DANGER);
                Options.tut_seen_invisible = you.num_turns;
            }
        }
    }
}

// Returns true if any activity was stopped.
bool interrupt_activity( activity_interrupt_type ai,
                         const activity_interrupt_data &at )
{
    paranoid_option_disable(ai, at);

    if (crawl_state.is_repeating_cmd())
        return interrupt_cmd_repeat(ai, at);

    const int delay = current_delay_action();

    if (delay == DELAY_NOT_DELAYED)
        return (false);

#ifdef DEBUG_DIAGNOSTICS
    mprf(MSGCH_DIAGNOSTICS, "Activity interrupt: %s",
         activity_interrupt_name(ai));
#endif

    // First try to stop the current delay.
    const delay_queue_item &item = you.delay_queue.front();

    if (should_stop_activity(item, ai, at))
    {
        // no monster will attack you inside a sanctuary,
        // so presence of monsters won't matter
        if (is_sanctuary(you.x_pos, you.y_pos))
            return (false);

        monster_warning(ai, at, item.type);
        stop_delay();
        return (true);
    }

    // Check the other queued delays; the first delay that is interruptible
    // will kill itself and all subsequent delays. This is so that a travel
    // delay stacked behind a delay such as stair/autopickup will be killed
    // correctly by interrupts that the simple stair/autopickup delay ignores.
    for (int i = 1, size = you.delay_queue.size(); i < size; ++i)
    {
        const delay_queue_item &it = you.delay_queue[i];
        if (should_stop_activity(it, ai, at))
        {
            // Do we have a queued run delay? If we do, flush the delay queue
            // so that stop running Lua notifications happen.
            for (int j = i; j < size; ++j)
            {
                if (is_run_delay( you.delay_queue[j].type ))
                {
                    monster_warning(ai, at, you.delay_queue[j].type);
                    stop_delay();
                    return (true);
                }
            }

            // Non-run queued delays can be discarded without any processing.
            you.delay_queue.erase( you.delay_queue.begin() + i,
                                   you.delay_queue.end() );
            return (true);
        }
    }

    return (false);
}

static const char *activity_interrupt_names[] =
{
    "force", "keypress", "full_hp", "full_mp", "statue",
    "hungry", "message", "hp_loss", "burden", "stat",
    "monster", "monster_attack", "teleport", "hit_monster"
};

const char *activity_interrupt_name(activity_interrupt_type ai)
{
    ASSERT( sizeof(activity_interrupt_names)
            / sizeof(*activity_interrupt_names) == NUM_AINTERRUPTS );

    if (ai == NUM_AINTERRUPTS)
        return ("");

    return activity_interrupt_names[ai];
}

activity_interrupt_type get_activity_interrupt(const std::string &name)
{
    ASSERT( sizeof(activity_interrupt_names)
            / sizeof(*activity_interrupt_names) == NUM_AINTERRUPTS );

    for (int i = 0; i < NUM_AINTERRUPTS; ++i)
        if (name == activity_interrupt_names[i])
            return activity_interrupt_type(i);

    return (NUM_AINTERRUPTS);
}

static const char *delay_names[] =
{
    "not_delayed", "eat", "vampire_feed", "armour_on", "armour_off",
    "jewellery_on", "memorise", "butcher", "bottle_blood", "offer_corpse",
    "weapon_swap", "passwall", "drop_item", "multidrop", "ascending_stairs",
    "descending_stairs", "recite", "run", "rest", "travel", "macro",
    "interruptible", "uninterruptible"
};

// Gets a delay given its name.
// name must be lowercased already!
delay_type get_delay(const std::string &name)
{
    ASSERT( sizeof(delay_names) / sizeof(*delay_names) == NUM_DELAYS );

    for (int i = 0; i < NUM_DELAYS; ++i)
    {
        if (name == delay_names[i])
            return delay_type(i);
    }

    // Also check American spellings:
    if (name == "armor_on")
        return (DELAY_ARMOUR_ON);

    if (name == "armor_off")
        return (DELAY_ARMOUR_OFF);

    if (name == "memorize")
        return (DELAY_MEMORISE);

    if (name == "jewelry_on")
        return (DELAY_JEWELLERY_ON);

    return (NUM_DELAYS);
}

const char *delay_name(int delay)
{
    ASSERT( sizeof(delay_names) / sizeof(*delay_names) == NUM_DELAYS );

    if (delay < 0 || delay >= NUM_DELAYS)
        return ("");

    return delay_names[delay];
}
