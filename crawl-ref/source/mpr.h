/*
 *  File:       mpr.h
 *  Summary:    Functions used to print simple messages.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *               <2>     9/11/07        MPC             Split off from message.h
 *               <2>     5/08/99        JDJ             mpr takes a const char* instead of a char array.
 *               <1>     -/--/--        LRH             Created
 */

#ifndef MPR_H
#define MPR_H

// if you mess with this list, you'll need to make changes in initfile.cc
// to message_channel_names, and probably also to message.cc to colour
// everything properly
enum msg_channel_type
{
    MSGCH_PLAIN,          // regular text
    MSGCH_PROMPT,         // various prompts
    MSGCH_GOD,            // god/religion (param is god)
    MSGCH_PRAY,           // praying messages (param is god)
    MSGCH_DURATION,       // effect down/warnings
    MSGCH_DANGER,         // serious life threats (ie very large HP attacks)
    MSGCH_WARN,           // much less serious threats
    MSGCH_FOOD,           // hunger notices
    MSGCH_RECOVERY,       // recovery from disease/stat/poison condition
    MSGCH_SOUND,          // messages about things the player hears
    MSGCH_TALK,           // monster talk (param is monster type)
    MSGCH_TALK_VISUAL,    // silent monster "talk" (not restricted by silence)
    MSGCH_INTRINSIC_GAIN, // player level/stat/species-power gains
    MSGCH_MUTATION,       // player gain/lose mutations
    MSGCH_MONSTER_SPELL,  // monsters casting spells
    MSGCH_MONSTER_ENCHANT,// monsters enchantments up and down
    MSGCH_MONSTER_DAMAGE, // monster damage reports (param is level)
    MSGCH_MONSTER_TARGET, // message marking the monster as a target
    MSGCH_ROTTEN_MEAT,    // messages about chunks/corpses becoming rotten  
    MSGCH_EQUIPMENT,      // equipment listing messages
    MSGCH_FLOOR_ITEMS,    // like equipment, but lists of floor items
    MSGCH_MULTITURN_ACTION,  // delayed action messages
    MSGCH_EXAMINE,        // messages describing monsters, features, items
    MSGCH_EXAMINE_FILTER, // "less important" instances of the above
    MSGCH_DIAGNOSTICS,    // various diagnostic messages
    MSGCH_TUTORIAL,       // messages for tutorial
    NUM_MESSAGE_CHANNELS  // always last
};

enum msg_colour_type
{
    MSGCOL_BLACK        = 0,    // the order of these colours is important
    MSGCOL_BLUE,
    MSGCOL_GREEN,
    MSGCOL_CYAN,
    MSGCOL_RED,
    MSGCOL_MAGENTA,
    MSGCOL_BROWN,
    MSGCOL_LIGHTGRAY,
    MSGCOL_DARKGRAY,
    MSGCOL_LIGHTBLUE,
    MSGCOL_LIGHTGREEN,
    MSGCOL_LIGHTCYAN,
    MSGCOL_LIGHTMAGENTA,
    MSGCOL_YELLOW,
    MSGCOL_WHITE,
    MSGCOL_DEFAULT,             // use default colour
    MSGCOL_ALTERNATE,           // use secondary default colour scheme
    MSGCOL_MUTED,               // don't print messages
    MSGCOL_PLAIN                // same as plain channel
};

// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: ability - acr - bang - beam - chardump - command - debug -
 *              decks - direct - effects - fight - files - food - it_use2 -
 *              it_use3 - item_use - items - macro - misc - monplace -
 *              monstuff - mstuff2 - mutation - ouch - overmap - player -
 *              religion - shopping - skills - spell - spl-book - spells -
 *              spells1 - spells2 - spells3 - spells4 - stuff - transfor -
 *              view
 * *********************************************************************** */
void mpr(const char *inf, msg_channel_type channel = MSGCH_PLAIN, int param=0);

// 4.1-style mpr, currently named mprf for minimal disruption.
void mprf( msg_channel_type channel, int param, const char *format, ... );
void mprf( msg_channel_type channel, const char *format, ... );
void mprf( const char *format, ... );

#endif
