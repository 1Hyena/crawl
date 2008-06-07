/*
 *  File:       spells2.h
 *  Summary:    Implementations of some additional spells.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *               <1>     -/--/--        LRH             Created
 */

#ifndef SPELLS2_H
#define SPELLS2_H

#include "enum.h"
#include "itemprop.h"  // from brand_type

struct dist;

// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
bool brand_weapon(brand_type which_brand, int power);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability - spell
 * *********************************************************************** */
int animate_a_corpse(int axps, int ayps, beh_type corps_beh,
                     int corps_hit, int class_allowed);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability - it_use3 - monstuff - mstuff2 - spell
 * *********************************************************************** */
int animate_dead(actor *caster, int power, beh_type corps_beh,
                 int corps_hit, int actual);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
char burn_freeze(int pow, beam_type b_f);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
int corpse_rot(int power);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: it_use3 - spell
 * *********************************************************************** */
bool summon_elemental(int pow, int restricted_type,
                      unsigned char unfriendly);

struct dist;
// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
int vampiric_drain(int pow, const dist &);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
int detect_creatures( int pow, bool telepathic = false );


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
unsigned char detect_items( int pow );


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
unsigned char detect_traps( int pow ); 


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: item_use - spell
 * *********************************************************************** */
void cast_refrigeration(int pow);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: item_use - spell
 * *********************************************************************** */
void cast_toxic_radiance(void);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
void cast_twisted(int power, beh_type corps_beh, int corps_hit);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability
 * *********************************************************************** */
void drain_life(int pow);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability - food - it_use2 - spell
 * returns TRUE if a stat was restored.
 * *********************************************************************** */
bool restore_stat(unsigned char which_stat, unsigned char stat_gain,
                  bool suppress_msg, bool recovery = false);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability - spell
 * *********************************************************************** */
bool summon_general_creature_spell(spell_type spell, int pow,
                                   bool god_gift = false);

bool summon_general_creature(int pow, bool quiet, monster_type mon,
                             beh_type beha, int hostile = -1, int dur = -1,
                             bool god_gift = false);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
void summon_animals(int pow);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
void summon_small_mammals(int pow);

bool summon_berserker(int pow, beh_type beha, bool god_gift);

// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability - religion - spell
 * *********************************************************************** */
bool summon_swarm(int pow, beh_type beha, bool god_gift);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
void summon_things(int pow);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: ability - spell
 * *********************************************************************** */
void turn_undead(int pow);      // what should I use for pow?


#endif
