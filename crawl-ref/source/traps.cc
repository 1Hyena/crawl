/*
 *  File:       traps.cc
 *  Summary:    Traps related functions.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *               <1>     9/11/07        MPC             Split from misc.cc
 */

#include "AppHdr.h"

#include "externs.h"
#include "traps.h"

#include <algorithm>

#include "beam.h"
#include "branch.h"
#include "delay.h"
#include "directn.h"
#include "it_use2.h"
#include "items.h"
#include "itemprop.h"
#include "makeitem.h"
#include "misc.h"
#include "mon-util.h"
#include "monstuff.h"
#include "mtransit.h"
#include "ouch.h"
#include "place.h"
#include "player.h"
#include "randart.h"
#include "skills.h"
#include "spells3.h"
#include "spl-mis.h"
#include "spl-util.h"
#include "terrain.h"
#include "transfor.h"
#include "tutorial.h"
#include "view.h"

static void dart_trap(bool trap_known, int trapped, bolt &pbolt, bool poison);

// Returns the number of a net on a given square.
// If trapped only stationary ones are counted
// otherwise the first net found is returned.
int get_trapping_net(const coord_def& where, bool trapped)
{
    for (stack_iterator si(where); si; ++si)
    {
         if (si->base_type == OBJ_MISSILES
             && si->sub_type == MI_THROWING_NET
             && (!trapped || item_is_stationary(*si)))
         {
             return (si->index());
         }
    }
    return (NON_ITEM);
}

// If there are more than one net on this square
// split off one of them for checking/setting values.
static void maybe_split_nets(item_def &item, const coord_def& where)
{
    if (item.quantity == 1)
    {
        set_item_stationary(item);
        return;
    }

    item_def it;

    it.base_type = item.base_type;
    it.sub_type  = item.sub_type;
    it.plus      = item.plus;
    it.plus2     = item.plus2;
    it.flags     = item.flags;
    it.special   = item.special;
    it.quantity  = --item.quantity;
    item_colour(it);

    item.quantity = 1;
    set_item_stationary(item);

    copy_item_to_grid( it, where );
}

void mark_net_trapping(const coord_def& where)
{
    int net = get_trapping_net(where);
    if (net == NON_ITEM)
    {
        net = get_trapping_net(where, false);
        if (net != NON_ITEM)
            maybe_split_nets(mitm[net], where);
    }
}

void monster_caught_in_net(monsters *mon, bolt &pbolt)
{
    if (mon->body_size(PSIZE_BODY) >= SIZE_GIANT)
        return;

    if (mons_is_insubstantial(mon->type))
    {
        if (mons_near(mon) && player_monster_visible(mon))
            mprf("The net passes right through %s!", mon->name(DESC_NOCAP_THE).c_str());
        return;
    }

    const monsters* mons = static_cast<const monsters*>(mon);
    bool mon_flies = mons->flight_mode() == FL_FLY;
    if (mon_flies && (!mons_is_confused(mons) || one_chance_in(3)))
    {
        simple_monster_message(mon, " darts out from under the net!");
        return;
    }

    if (mons->type == MONS_OOZE || mons->type == MONS_PULSATING_LUMP)
    {
        simple_monster_message(mon, " oozes right through the net!");
        return;
    }

    if (!mons_is_caught(mon) && mon->add_ench(ENCH_HELD))
    {
        if (mons_near(mon) && !player_monster_visible(mon))
            mpr("Something gets caught in the net!");
        else
            simple_monster_message(mon, " is caught in the net!");

        if (mon_flies)
        {
            simple_monster_message(mon, " falls like a stone!");
            mons_check_pool(mon, pbolt.killer(), pbolt.beam_source);
        }
    }
}

void player_caught_in_net()
{
    if (you.body_size(PSIZE_BODY) >= SIZE_GIANT)
        return;

    if (you.attribute[ATTR_TRANSFORMATION] == TRAN_AIR)
    {
        mpr("The net passes right through you!");
        return;
    }

    if (you.flight_mode() == FL_FLY && (!you.confused() || one_chance_in(3)))
    {
        mpr("You dart out from under the net!");
        return;
    }

    if (!you.attribute[ATTR_HELD])
    {
        you.attribute[ATTR_HELD] = 10;
        mpr("You become entangled in the net!");
        stop_running();

        // I guess levitation works differently, keeping both you
        // and the net hovering above the floor
        if (you.flight_mode() == FL_FLY)
        {
            mpr("You fall like a stone!");
            fall_into_a_pool(you.pos(), false, grd(you.pos()));
        }

        stop_delay(true); // even stair delays
    }
}

void check_net_will_hold_monster(monsters *mons)
{
    if (mons->body_size(PSIZE_BODY) >= SIZE_GIANT)
    {
        int net = get_trapping_net(mons->pos());
        if (net != NON_ITEM)
            destroy_item(net);

        if (see_grid(mons->pos()))
        {
            if (player_monster_visible(mons))
            {
                mprf("The net rips apart, and %s comes free!",
                     mons->name(DESC_NOCAP_THE).c_str());
            }
            else
                mpr("All of a sudden the net rips apart!");
        }
    }
    else if (mons_is_insubstantial(mons->type)
             || mons->type == MONS_OOZE
             || mons->type == MONS_PULSATING_LUMP)
    {
        const int net = get_trapping_net(mons->pos());
        if (net != NON_ITEM)
            remove_item_stationary(mitm[net]);

        if (mons_is_insubstantial(mons->type))
        {
            simple_monster_message(mons,
                                   " drifts right through the net!");
        }
        else
        {
            simple_monster_message(mons,
                                   " oozes right through the net!");
        }
    }
    else
        mons->add_ench(ENCH_HELD);
}

static void dart_trap(bool trap_known, int trapped, bolt &pbolt, bool poison)
{
    int damage_taken = 0;
    int trap_hit, your_dodge;

    if (one_chance_in(5) || (trap_known && !one_chance_in(4)))
    {
        mprf( "You avoid triggering a%s trap.", pbolt.name.c_str() );
        return;
    }

    if (you.equip[EQ_SHIELD] != -1 && one_chance_in(3))
        exercise( SK_SHIELDS, 1 );

    std::string msg = "A" + pbolt.name + " shoots out and ";

    if (random2( 20 + 5 * you.shield_blocks * you.shield_blocks )
                                                < player_shield_class())
    {
        you.shield_blocks++;
        msg += "hits your shield.";
        mpr(msg.c_str());
    }
    else
    {
        // note that this uses full ( not random2limit(foo,40) )
        // player_evasion.
        trap_hit = (20 + (you.your_level * 2)) * random2(200) / 100;

        your_dodge = player_evasion() + random2(you.dex) / 3
            - 2 + (you.duration[DUR_REPEL_MISSILES] * 10);

        if (trap_hit >= your_dodge && you.duration[DUR_DEFLECT_MISSILES] == 0)
        {
            msg += "hits you!";
            mpr(msg.c_str());

            if (poison && x_chance_in_y(50 - (3 * player_AC()) / 2, 100)
                && !player_res_poison())
            {
                poison_player( 1 + random2(3) );
            }

            damage_taken = roll_dice( pbolt.damage );
            damage_taken -= random2( player_AC() + 1 );

            if (damage_taken > 0)
                ouch( damage_taken, 0, KILLED_BY_TRAP, pbolt.name.c_str() );
        }
        else
        {
            msg += "misses you.";
            mpr(msg.c_str());
        }

        if (player_light_armour(true) && coinflip())
            exercise( SK_DODGING, 1 );
    }

    pbolt.target = you.pos();

    if (coinflip())
        itrap( pbolt, trapped );
}                               // end dart_trap()

//
// itrap takes location from target_x, target_y of bolt strcture.
//

void itrap( bolt &pbolt, int trapped )
{
    object_class_type base_type = OBJ_MISSILES;
    int sub_type = MI_DART;

    switch (env.trap[trapped].type)
    {
    case TRAP_DART:
        base_type = OBJ_MISSILES;
        sub_type = MI_DART;
        break;
    case TRAP_ARROW:
        base_type = OBJ_MISSILES;
        sub_type = MI_ARROW;
        break;
    case TRAP_BOLT:
        base_type = OBJ_MISSILES;
        sub_type = MI_BOLT;
        break;
    case TRAP_SPEAR:
        base_type = OBJ_WEAPONS;
        sub_type = WPN_SPEAR;
        break;
    case TRAP_AXE:
        base_type = OBJ_WEAPONS;
        sub_type = WPN_HAND_AXE;
        break;
    case TRAP_NEEDLE:
        base_type = OBJ_MISSILES;
        sub_type = MI_NEEDLE;
        break;
    case TRAP_NET:
        base_type = OBJ_MISSILES;
        sub_type = MI_THROWING_NET;
        break;
    default:
        return;
    }

    trap_item( base_type, sub_type, pbolt.target );

    return;
}

void handle_traps(trap_type trt, int i, bool trap_known)
{
    struct bolt beam;

    bool branchtype = false;
    if (trap_category(trt) == DNGN_TRAP_MECHANICAL && trt != TRAP_NET
        && trt != TRAP_BLADE)
    {
        if (you.where_are_you == BRANCH_ORCISH_MINES)
        {
            beam.name = "n orcish";
            branchtype = true;
        }
        else if (you.where_are_you == BRANCH_ELVEN_HALLS)
        {
            beam.name = "n elven";
            branchtype = true;
        }
        else
            beam.name = "";
    }

    switch (trt)
    {
    case TRAP_DART:
        beam.name += " dart";
        beam.damage = dice_def( 1, 4 + (you.your_level / 2) );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_NEEDLE:
        beam.name += " needle";
        beam.damage = dice_def( 1, 0 );
        dart_trap(trap_known, i, beam, true);
        break;

    case TRAP_ARROW:
        beam.name += (branchtype? "" : "n");
        beam.name += " arrow";
        beam.damage = dice_def( 1, 7 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_BOLT:
        beam.name += " bolt";
        beam.damage = dice_def( 1, 13 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_SPEAR:
        beam.name += " spear";
        beam.damage = dice_def( 1, 10 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_AXE:
        beam.name += (branchtype? "" : "n");
        beam.name += " axe";
        beam.damage = dice_def( 1, 15 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_TELEPORT:
        mpr("You enter a teleport trap!");

        if (scan_randarts(RAP_PREVENT_TELEPORTATION))
            mpr("You feel a weird sense of stasis.");
        else
            you_teleport_now( true );
        break;

    case TRAP_ALARM:
        if (silenced(you.pos()))
        {
            if (trap_known)
                mpr("The alarm is silenced.");
            else
                grd(you.pos()) = DNGN_UNDISCOVERED_TRAP;
            return;
        }

        noisy(12, you.pos(), "An alarm trap emits a blaring wail!");

        break;

    case TRAP_BLADE:
        if (trap_known && one_chance_in(3))
            mpr("You avoid triggering a blade trap.");
        else if (random2limit(player_evasion(), 40)
                 + (random2(you.dex) / 3) + (trap_known ? 3 : 0) > 8)
        {
            mpr("A huge blade swings just past you!");
        }
        else
        {
            mpr("A huge blade swings out and slices into you!");
            int damage = (you.your_level * 2) + random2avg(29, 2)
                          - random2(1 + player_AC());
            ouch( damage, 0, KILLED_BY_TRAP, " blade" );
            bleed_onto_floor(you.pos(), -1, damage, true);
        }
        break;

    case TRAP_NET:
        if (trap_known && one_chance_in(3))
            mpr("A net swings high above you.");
        else
        {
            if (random2limit(player_evasion(), 40)
                + (random2(you.dex) / 3) + (trap_known ? 3 : 0) > 12)
            {
                mpr("A net drops to the ground!");
            }
            else
            {
                mpr("A large net falls onto you!");
                player_caught_in_net();
            }

            trap_item( OBJ_MISSILES, MI_THROWING_NET, env.trap[i].pos);
            if (you.attribute[ATTR_HELD])
                mark_net_trapping(you.pos());

            grd(env.trap[i].pos) = DNGN_FLOOR;
            env.trap[i].type = TRAP_UNASSIGNED;
        }
        break;

    // If we don't trigger the shaft, and the player doesn't
    // already know about it, don't let him/her notice it.
    case TRAP_SHAFT:
    {
        if (!you.will_trigger_shaft())
        {
            if (trap_known && !you.airborne())
                mpr("You don't fall through the shaft.");

            if (!trap_known)
                grd(you.pos()) = DNGN_UNDISCOVERED_TRAP;

            return;
        }

        // Paranoia
        if (!is_valid_shaft_level())
        {
            if (trap_known)
                mpr("The shaft disappears in a puff of logic!");

            grd(env.trap[i].pos) = DNGN_FLOOR;
            env.trap[i].type = TRAP_UNASSIGNED;
            return;
        }

        if (!you.do_shaft() && !trap_known)
        {
            grd(you.pos()) = DNGN_UNDISCOVERED_TRAP;
            return;
        }

        break;
    }

    case TRAP_ZOT:
    default:
        mpr((trap_known) ? "You enter the Zot trap."
                         : "Oh no! You have blundered into a Zot trap!");
        MiscastEffect( &you, ZOT_TRAP_MISCAST, SPTYP_RANDOM,
                       3, "a Zot trap" );
        break;
    }
    learned_something_new(TUT_SEEN_TRAP, you.pos());

    if (!trap_known) // Now you know...
        exercise(SK_TRAPS_DOORS, ((coinflip()) ? 2 : 1));
}

void destroy_trap( const coord_def& pos )
{
    for (int i = 0; i < MAX_TRAPS; ++i)
    {
        if (env.trap[i].pos == pos && env.trap[i].type != TRAP_UNASSIGNED)
        {
            grd(pos) = DNGN_FLOOR;
            env.trap[i].type = TRAP_UNASSIGNED;
            return;
        }
    }
}

void disarm_trap( dist &disa )
{
    if (you.duration[DUR_BERSERKER])
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    int i, j;

    for (i = 0; i < MAX_TRAPS; i++)
    {
        if (env.trap[i].pos == you.pos() + disa.delta)
            break;

        if (i == MAX_TRAPS - 1)
        {
            mpr("Error - couldn't find that trap.", MSGCH_ERROR);
            return;
        }
    }

    switch (trap_category(env.trap[i].type))
    {
    case DNGN_TRAP_MAGICAL:
        mpr("You can't disarm that trap.");
        return;
    case DNGN_TRAP_NATURAL:
        // Only shafts for now.
        mpr("You can't disarm a shaft.");
        return;
    default:
        break;
    }

    if (random2(you.skills[SK_TRAPS_DOORS] + 2) <= random2(you.your_level + 5))
    {
        mpr("You failed to disarm the trap.");

        you.turn_is_over = true;

        if (random2(you.dex) > 5 + random2(5 + you.your_level))
            exercise(SK_TRAPS_DOORS, 1 + random2(you.your_level / 5));
        else
        {
            if (env.trap[i].type == TRAP_NET && env.trap[i].pos != you.pos())
            {
                if (coinflip())
                    return;

                mpr("You stumble into the trap!");
                move_player_to_grid( env.trap[i].pos, true, false, true);
            }
            else
                handle_traps(env.trap[i].type, i, false);

            if (coinflip())
                exercise(SK_TRAPS_DOORS, 1);
        }

        return;
    }

    mpr("You have disarmed the trap.");

    bolt beam;

    beam.target = you.pos() + disa.delta;

    if (env.trap[i].type == TRAP_NET)
        trap_item( OBJ_MISSILES, MI_THROWING_NET, beam.target );
    else if (env.trap[i].type != TRAP_BLADE
             && trap_category(env.trap[i].type) == DNGN_TRAP_MECHANICAL)
    {
        const int num_to_make = 10 + random2(you.skills[SK_TRAPS_DOORS]);
        for (j = 0; j < num_to_make; j++)
        {
            // Places items (eg darts), which will automatically stack.
            itrap(beam, i);
        }
    }

    grd(you.pos() + disa.delta) = DNGN_FLOOR;
    env.trap[i].type = TRAP_UNASSIGNED;
    you.turn_is_over = true;

    // Reduced from 5 + random2(5).
    exercise(SK_TRAPS_DOORS, 1 + random2(5) + (you.your_level / 5));
}

// Attempts to take a net off a given monster.
// This doesn't actually have any effect (yet).
// Do not expect gratitude for this!
// ----------------------------------
void remove_net_from(monsters *mon)
{
    you.turn_is_over = true;

    int net = get_trapping_net(mon->pos());

    if (net == NON_ITEM)
    {
        mon->del_ench(ENCH_HELD, true);
        return;
    }

    // factor in whether monster is paralysed or invisible
    int paralys = 0;
    if (mons_is_paralysed(mon)) // makes this easier
        paralys = random2(5);

    int invis = 0;
    if (!player_monster_visible(mon)) // makes this harder
        invis = 3 + random2(5);

    bool net_destroyed = false;
    if ( random2(you.skills[SK_TRAPS_DOORS] + 2) + paralys
           <= random2( 2*mon->body_size(PSIZE_BODY) + 3 ) + invis)
    {
        if (one_chance_in(you.skills[SK_TRAPS_DOORS] + you.dex/2))
        {
            mitm[net].plus--;
            mpr("You tear at the net.");
            if (mitm[net].plus < -7)
            {
                mpr("Whoops! The net comes apart in your hands!");
                mon->del_ench(ENCH_HELD, true);
                destroy_item(net);
                net_destroyed = true;
            }
        }

        if (!net_destroyed)
        {
            if (player_monster_visible(mon))
            {
                mprf("You fail to remove the net from %s.",
                     mon->name(DESC_NOCAP_THE).c_str());
            }
            else
                mpr("You fail to remove the net.");
        }

        if (random2(you.dex) > 5 + random2( 2*mon->body_size(PSIZE_BODY) ))
            exercise(SK_TRAPS_DOORS, 1 + random2(mon->body_size(PSIZE_BODY)/2));
        return;
    }

    mon->del_ench(ENCH_HELD, true);
    remove_item_stationary(mitm[net]);

    if (player_monster_visible(mon))
        mprf("You free %s.", mon->name(DESC_NOCAP_THE).c_str());
    else
        mpr("You loosen the net.");

}

// Decides whether you will try to tear the net (result <= 0)
// or try to slip out of it (result > 0).
// Both damage and escape could be 9 (more likely for damage)
// but are capped at 5 (damage) and 4 (escape).
static int damage_or_escape_net(int hold)
{
    // Spriggan: little (+2)
    // Halfling, Kobold, Gnome: small (+1)
    // Human, Elf, ...: medium (0)
    // Ogre, Troll, Centaur, Naga: large (-1)
    // transformations: spider, bat: tiny (+3); ice beast: large (-1)
    int escape = SIZE_MEDIUM - you.body_size(PSIZE_BODY);

    int damage = -escape;

    // your weapon may damage the net, max. bonus of 2
    if (you.equip[EQ_WEAPON] != -1)
    {
        if (can_cut_meat(you.inv[you.equip[EQ_WEAPON]]))
            damage++;

        int brand = get_weapon_brand( you.inv[you.equip[EQ_WEAPON]] );
        if (brand == SPWPN_FLAMING || brand == SPWPN_VORPAL)
            damage++;
    }
    else if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
        damage += 2;
    else if (you.has_usable_claws())
    {
        int level = you.has_claws();
        if (level == 1)
            damage += coinflip();
        else
            damage += level - 1;
    }

    // Berserkers get a fighting bonus.
    if (you.duration[DUR_BERSERKER])
        damage += 2;

    // Check stats.
    if (x_chance_in_y(you.strength, 18))
        damage++;
    if (x_chance_in_y(you.dex, 12))
        escape++;
    if (x_chance_in_y(player_evasion(), 20))
        escape++;

    // Monsters around you add urgency.
    if (!i_feel_safe())
    {
        damage++;
        escape++;
    }

    // Confusion makes the whole thing somewhat harder
    // (less so for trying to escape).
    if (you.duration[DUR_CONF])
    {
        if (escape > 1)
            escape--;
        else if (damage >= 2)
            damage -= 2;
    }

    // Damaged nets are easier to destroy.
    if (hold < 0)
    {
        damage += random2(-hold/3 + 1);

        // ... and easier to slip out of (but only if escape looks feasible).
        if (you.attribute[ATTR_HELD] < 5 || escape >= damage)
            escape += random2(-hold/2) + 1;
    }

    // If undecided, choose damaging approach (it's quicker).
    if (damage >= escape)
        return (-damage); // negate value

    return (escape);
}

// Calls the above function to decide on how to get free.
// Note that usually the net will be damaged until trying to slip out
// becomes feasible (for size etc.), so it may take even longer.
void free_self_from_net()
{
    int net = get_trapping_net(you.pos());

    if (net == NON_ITEM) // really shouldn't happen!
    {
        you.attribute[ATTR_HELD] = 0;
        return;
    }

    int hold = mitm[net].plus;
    int do_what = damage_or_escape_net(hold);
#ifdef DEBUG_DIAGNOSTICS
    mprf(MSGCH_DIAGNOSTICS, "net.plus: %d, ATTR_HELD: %d, do_what: %d",
         hold, you.attribute[ATTR_HELD], do_what);
#endif

    if (do_what <= 0) // You try to destroy the net
    {
        // For previously undamaged nets this takes at least 2 and at most
        // 8 turns.
        bool can_slice = you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS
                         || you.equip[EQ_WEAPON] != -1
                            && can_cut_meat(you.inv[you.equip[EQ_WEAPON]]);

        int damage = -do_what;

        if (damage < 1)
            damage = 1;

        if (you.duration[DUR_BERSERKER])
            damage *= 2;

        // Medium sized characters are at disadvantage and sometimes
        // get a bonus.
        if (you.body_size(PSIZE_BODY) == SIZE_MEDIUM)
            damage += coinflip();

        if (damage > 5)
            damage = 5;

        hold -= damage;
        mitm[net].plus = hold;

        if (hold < -7)
        {
            mprf("You %s the net and break free!",
                 can_slice ? (damage >= 4? "slice" : "cut") :
                             (damage >= 4? "shred" : "rip"));

            destroy_item(net);

            you.attribute[ATTR_HELD] = 0;
            return;
        }

        if (damage >= 4)
        {
            mprf("You %s into the net.",
                 can_slice? "slice" : "tear a large gash");
        }
        else
            mpr("You struggle against the net.");

        // Occasionally decrease duration a bit
        // (this is so switching from damage to escape does not hurt as much).
        if (you.attribute[ATTR_HELD] > 1 && coinflip())
        {
            you.attribute[ATTR_HELD]--;

            if (you.attribute[ATTR_HELD] > 1 && hold < -random2(5))
                you.attribute[ATTR_HELD]--;
        }
   }
   else
   {
        // You try to escape (takes at least 3 turns, and at most 10).
        int escape = do_what;

        if (you.duration[DUR_HASTE]) // extra bonus, also Berserk
            escape++;

        // Medium sized characters are at disadvantage and sometimes
        // get a bonus.
        if (you.body_size(PSIZE_BODY) == SIZE_MEDIUM)
            escape += coinflip();

        if (escape > 4)
            escape = 4;

        if (escape >= you.attribute[ATTR_HELD])
        {
            if (escape >= 3)
                mpr("You slip out of the net!");
            else
                mpr("You break free from the net!");

            you.attribute[ATTR_HELD] = 0;
            remove_item_stationary(mitm[net]);
            return;
        }

        if (escape >= 3)
            mpr("You try to slip out of the net.");
        else
            mpr("You struggle to escape the net.");

        you.attribute[ATTR_HELD] -= escape;
   }
}

void clear_trapping_net()
{
    if (!you.attribute[ATTR_HELD])
        return;

    const int net = get_trapping_net(you.pos());
    if (net != NON_ITEM)
        remove_item_stationary(mitm[net]);

    you.attribute[ATTR_HELD] = 0;
}

bool trap_item(object_class_type base_type, char sub_type,
               const coord_def& where)
{
    item_def item;
    item.base_type = base_type;
    item.sub_type  = sub_type;
    item.plus      = 0;
    item.plus2     = 0;
    item.flags     = 0;
    item.special   = 0;
    item.quantity  = 1;

    if (base_type == OBJ_MISSILES)
    {
        if (sub_type == MI_NEEDLE)
            set_item_ego_type( item, OBJ_MISSILES, SPMSL_POISONED );
        else
            set_item_ego_type( item, OBJ_MISSILES, SPMSL_NORMAL );
    }
    else
    {
        set_item_ego_type( item, OBJ_WEAPONS, SPWPN_NORMAL );
    }

    item_colour(item);

    if (igrd(where) != NON_ITEM)
    {
        if (items_stack( item, mitm[ igrd(where) ] ))
        {
            inc_mitm_item_quantity( igrd(where), 1 );
            return (false);
        }

        // don't want to go overboard here. Will only generate up to three
        // separate trap items, or less if there are other items present.
        if (mitm[ igrd(where) ].link != NON_ITEM
            && (item.base_type != OBJ_MISSILES
                || item.sub_type != MI_THROWING_NET))
        {
            if (mitm[ mitm[ igrd(where) ].link ].link != NON_ITEM)
                return (false);
        }
    }

    // give appropriate racial flag for Orcish Mines and Elven Halls
    // should we ever allow properties of dungeon features, we could use that
    if (you.where_are_you == BRANCH_ORCISH_MINES)
        set_equip_race( item, ISFLAG_ORCISH );
    else if (you.where_are_you == BRANCH_ELVEN_HALLS)
        set_equip_race( item, ISFLAG_ELVEN );

    return (!copy_item_to_grid( item, where, 1 ));
}                               // end trap_item()

// returns appropriate trap symbol for a given trap type {dlb}
dungeon_feature_type trap_category(trap_type type)
{
    switch (type)
    {
    case TRAP_SHAFT:
        return (DNGN_TRAP_NATURAL);

    case TRAP_TELEPORT:
    case TRAP_ALARM:
    case TRAP_ZOT:
        return (DNGN_TRAP_MAGICAL);

    case TRAP_DART:
    case TRAP_ARROW:
    case TRAP_SPEAR:
    case TRAP_AXE:
    case TRAP_BLADE:
    case TRAP_BOLT:
    case TRAP_NEEDLE:
    case TRAP_NET:
    default:                    // what *would* be the default? {dlb}
        return (DNGN_TRAP_MECHANICAL);
    }
}                               // end trap_category()

// Returns index of the trap for a given (x,y) coordinate pair {dlb}
int trap_at_xy(const coord_def& xy)
{
    for (int which_trap = 0; which_trap < MAX_TRAPS; which_trap++)
    {
        if (env.trap[which_trap].pos == xy
            && env.trap[which_trap].type != TRAP_UNASSIGNED)
        {
            return (which_trap);
        }
    }

    // No idea how well this will be handled elsewhere. {dlb}
    return (-1);
}

trap_type trap_type_at_xy(const coord_def& xy)
{
    const int idx = trap_at_xy(xy);
    return (idx == -1 ? NUM_TRAPS : env.trap[idx].type);
}

bool is_valid_shaft_level(const level_id &place)
{
    if (place.level_type != LEVEL_DUNGEON)
        return (false);

    // Disallow shafts on the first two levels.
    if (place == BRANCH_MAIN_DUNGEON
        && you.your_level < 2)
    {
        return (false);
    }

    // Don't generate shafts in branches where teleport control
    // is prevented.  Prevents player from going down levels without
    // reaching stairs, and also keeps player from getting stuck
    // on lower levels with the innability to use teleport control to
    // get back up.
    if (testbits(get_branch_flags(place.branch), LFLAG_NO_TELE_CONTROL))
        return (false);

    const Branch &branch = branches[place.branch];

    // When generating levels, don't place a shaft on the level
    // immediately above the bottom of a branch if that branch is
    // significantly more dangerous than normal.
    int min_delta = 1;
    if (env.turns_on_level == -1 && branch.dangerous_bottom_level)
        min_delta = 2;

    return ((branch.depth - place.depth) >= min_delta);
}

level_id generic_shaft_dest(level_pos lpos)
{
    level_id  lid = lpos.id;
    coord_def pos = lpos.pos;

    if (lid.level_type != LEVEL_DUNGEON)
        return lid;

    int      curr_depth = lid.depth;
    Branch   &branch    = branches[lid.branch];

    // 25% drop one level
    // 50% drop two levels
    // 25% drop three levels
    lid.depth += 2;
    if (pos.x % 2)
        lid.depth--;
    if (pos.y % 2)
        lid.depth++;

    if (lid.depth > branch.depth)
        lid.depth = branch.depth;

    if (lid.depth == curr_depth)
        return lid;

    // Only shafts on the level immediately above a dangerous branch
    // bottom will take you to that dangerous bottom, and shafts can't
    // be created during level generation time.
    // Include level 27 of the main dungeon here, but don't restrict
    // shaft creation (so don't set branch.dangerous_bottom_level).
    if ((branch.dangerous_bottom_level)
        && lid.depth == branch.depth
        && (branch.depth - curr_depth) > 1)
    {
        lid.depth--;
    }

    return lid;
}

level_id generic_shaft_dest(coord_def pos)
{
    return generic_shaft_dest(level_pos(level_id::current(), pos));
}

void handle_items_on_shaft(const coord_def& pos, bool open_shaft)
{
    if (!is_valid_shaft_level())
        return;

    level_id  dest = generic_shaft_dest(pos);

    if (dest == level_id::current())
        return;

    int o = igrd(pos);

    if (o == NON_ITEM)
        return;

    igrd(pos) = NON_ITEM;

    if (is_terrain_seen(pos) && open_shaft)
    {
        mpr("A shaft opens up in the floor!");
        grd(pos) = DNGN_TRAP_NATURAL;
    }

    while (o != NON_ITEM)
    {
        int next = mitm[o].link;

        if (is_valid_item( mitm[o] ))
        {
            if (is_terrain_seen(pos))
            {
                mprf("%s falls through the shaft.",
                     mitm[o].name(DESC_INVENTORY).c_str());
            }
            add_item_to_transit(dest, mitm[o]);

            mitm[o].base_type = OBJ_UNASSIGNED;
            mitm[o].quantity = 0;
            mitm[o].props.clear();
        }

        o = next;
    }
}

static int num_traps_default(int level_number, const level_id &place)
{
    return random2avg(9, 2);
}

int num_traps_for_place(int level_number, const level_id &place)
{
    if (level_number == -1)
    {
        switch(place.level_type)
        {
        case LEVEL_DUNGEON:
            level_number = absdungeon_depth(place.branch, place.depth);
            break;
        case LEVEL_ABYSS:
            level_number = 51;
            break;
        case LEVEL_PANDEMONIUM:
            level_number = 52;
            break;
        default:
            level_number = you.your_level;
        }
    }

    switch (place.level_type)
    {
    case LEVEL_DUNGEON:
        if (branches[place.branch].num_traps_function != NULL)
            return branches[place.branch].num_traps_function(level_number);
        else
            return num_traps_default(level_number, place);
    case LEVEL_ABYSS:
        return traps_abyss_number(level_number);
    case LEVEL_PANDEMONIUM:
        return traps_pan_number(level_number);
    case LEVEL_LABYRINTH:
    case LEVEL_PORTAL_VAULT:
        ASSERT(false);
        break;
    default:
        return 0;
    }

    return 0;
}

static trap_type random_trap_default(int level_number, const level_id &place)
{
    trap_type type = TRAP_DART;

    if ((random2(1 + level_number) > 1) && one_chance_in(4))
        type = TRAP_NEEDLE;
    if (random2(1 + level_number) > 3)
        type = TRAP_SPEAR;
    if (random2(1 + level_number) > 5)
        type = TRAP_AXE;

    // Note we're boosting arrow trap numbers by moving it
    // down the list, and making spear and axe traps rarer.
    if (type == TRAP_DART ? random2(1 + level_number) > 2
                          : one_chance_in(7))
    {
        type = TRAP_ARROW;
    }

    if ((type == TRAP_DART || type == TRAP_ARROW) && one_chance_in(15))
        type = TRAP_NET;

    if (random2(1 + level_number) > 7)
        type = TRAP_BOLT;
    if (random2(1 + level_number) > 11)
        type = TRAP_BLADE;

    if (random2(1 + level_number) > 14 && one_chance_in(3)
        || (place == BRANCH_HALL_OF_ZOT && coinflip()))
    {
        type = TRAP_ZOT;
    }

    if (one_chance_in(50) && is_valid_shaft_level(place))
        type = TRAP_SHAFT;
    if (one_chance_in(20))
        type = TRAP_TELEPORT;
    if (one_chance_in(40))
        type = TRAP_ALARM;

    return (type);
}

trap_type random_trap_for_place(int level_number, const level_id &place)
{
    if (level_number == -1)
    {
        switch (place.level_type)
        {
        case LEVEL_DUNGEON:
            level_number = absdungeon_depth(place.branch, place.depth);
            break;
        case LEVEL_ABYSS:
            level_number = 51;
            break;
        case LEVEL_PANDEMONIUM:
            level_number = 52;
            break;
        default:
            level_number = you.your_level;
        }
    }

    switch (place.level_type)
    {
    case LEVEL_DUNGEON:
        if (branches[place.branch].rand_trap_function != NULL)
            return branches[place.branch].rand_trap_function(level_number);
        else
            return random_trap_default(level_number, place);
    case LEVEL_ABYSS:
        return traps_abyss_type(level_number);
    case LEVEL_PANDEMONIUM:
        return traps_pan_type(level_number);
    default:
        return random_trap_default(level_number, place);
    }
    return NUM_TRAPS;
}

int traps_zero_number(int level_number)
{
    return 0;
}

int traps_pan_number(int level_number)
{
    return num_traps_default(level_number, level_id(LEVEL_PANDEMONIUM));
}

trap_type traps_pan_type(int level_number)
{
    return random_trap_default(level_number, level_id(LEVEL_PANDEMONIUM));
}

int traps_abyss_number(int level_number)
{
    return num_traps_default(level_number, level_id(LEVEL_ABYSS));
}

trap_type traps_abyss_type(int level_number)
{
    return random_trap_default(level_number, level_id(LEVEL_ABYSS));
}

int traps_lab_number(int level_number)
{
    return num_traps_default(level_number, level_id(LEVEL_LABYRINTH));
}

trap_type traps_lab_type(int level_number)
{
    return random_trap_default(level_number, level_id(LEVEL_LABYRINTH));
}
