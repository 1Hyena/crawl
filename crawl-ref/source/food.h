/*
 *  File:       food.cc
 *  Summary:    Functions for eating and butchering.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author: zelgadis $ on $Date: 2007-09-16 00:33:50 +0100 (Sun, 16 Sep 2007) $
 *
 *  Modified for Hexcrawl by Martin Bays, 2007
 *
 *  Change History (most recent first):
 *
 *               <1>     -/--/--        LRH             Created
 */


#ifndef FOOD_H
#define FOOD_H

enum food_type
{
    FOOD_MEAT_RATION,                  //    0
    FOOD_BREAD_RATION,
    FOOD_PEAR,
    FOOD_APPLE,
    FOOD_CHOKO,
    FOOD_HONEYCOMB,                    //    5
    FOOD_ROYAL_JELLY,
    FOOD_SNOZZCUMBER,
    FOOD_PIZZA,
    FOOD_APRICOT,
    FOOD_ORANGE,                       //   10
    FOOD_BANANA,
    FOOD_STRAWBERRY,
    FOOD_RAMBUTAN,
    FOOD_LEMON,
    FOOD_GRAPE,                        //   15
    FOOD_SULTANA,
    FOOD_LYCHEE,
    FOOD_BEEF_JERKY,
    FOOD_CHEESE,
    FOOD_SAUSAGE,                      //   20
    FOOD_CHUNK,
    NUM_FOODS
};


// last updated 19jun2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
bool butchery(void);


// last updated 19jun2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
bool eat_food(bool run_hook = true);


// last updated 19jun2000 {dlb}
/* ***********************************************************************
 * called from: abl-show - acr - fight - food - spell
 * *********************************************************************** */
void make_hungry(int hunger_amount, bool suppress_msg);


// last updated 19jun2000 {dlb}
/* ***********************************************************************
 * called from: acr - fight - food - it_use2 - item_use
 * *********************************************************************** */
void lessen_hunger(int statiated_amount, bool suppress_msg);


// last updated 19jun2000 {dlb}
/* ***********************************************************************
 * called from: acr - decks - food - religion
 * *********************************************************************** */
void set_hunger(int new_hunger_level, bool suppress_msg);


// last updated 10sept2000 {bwr}
/* ***********************************************************************
 * called from: delay.cc
 * *********************************************************************** */
void weapon_switch( int targ );

bool can_ingest(int what_isit, int kindof_thing, bool suppress_msg, 
                bool reqid = false, bool check_hunger = true);

void eat_floor_item(int item_link);

bool eat_from_floor(void);

void eat_from_inventory(int which_inventory_slot);

bool prompt_eat_from_inventory(void);

void chunk_nutrition_message(int nutrition);

#endif
