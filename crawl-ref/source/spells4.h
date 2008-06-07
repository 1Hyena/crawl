/*
 *  File:       spells4.h
 *  Summary:    Yet More Spell Function Declarations
 *  Written by: Josh Fishman
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 * <2> 12jul2000  jmf  Fixed random (undocumented) damage
 * <1> 07jan2000  jmf  Created
 */


#ifndef SPELLS4_H
#define SPELLS4_H

#include "externs.h"

struct bolt;

bool backlight_monsters(int x, int y, int pow, int garbage);
int make_a_normal_cloud(int x, int y, int pow, int spread_rate,
                        cloud_type ctype, kill_category);
int disperse_monsters(int x, int y, int pow, int message);

void cast_bend(int pow);
void cast_condensation_shield(int pow);
void remove_divine_shield(void);
void cast_divine_shield(void);
void cast_detect_secret_doors(int pow);
void cast_discharge(int pow);
bool cast_evaporate(int pow, bolt& beem, int potion);
void cast_fulsome_distillation(int powc);
void cast_forescry(int pow);
void cast_fragmentation(int powc);
void cast_twist(int powc);
void cast_far_strike(int powc);
void cast_swap(int powc);
int cast_apportation(int powc);
void cast_ignite_poison(int pow);
void cast_intoxicate(int pow);
void cast_mass_sleep(int pow);
void cast_passwall(int pow);
void cast_rotting(int pow);
bool cast_sandblast(int powc, bolt &beam);
void cast_see_invisible(int pow);

void cast_shatter(int pow);
void cast_silence(int pow);
void cast_sticks_to_snakes(int pow);
void cast_chain_lightning( int pow );
void cast_conjure_ball_lightning(int pow);
void cast_summon_large_mammal(int pow);
void cast_tame_beasts(int pow);
void cast_dispersal(int pow);
void cast_snake_charm(int pow);
void cast_stoneskin(int pow);

int cast_semi_controlled_blink(int pow);
bool cast_portal_projectile(int pow, bolt& beam);

#endif
