/*
 *  File:       enum.h
 *  Summary:    Global (ick) enums.
 *  Written by: Daniel Ligon
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *      <11>   7 Aug 01  MV     Changed MSLOT_UNASSIGNED_I to MSLOT_MISCELLANY
 *                              added NUM_MISCELLANY, changed MONS_ANOTHER_
 *                              LAVA_THING to MONS_SALAMANDER
 *      <10>   7/29/00   JDJ    Changed NUM_SPELL_TYPES to 14 (from 32767!).
 *             24jun2000 jmf    Changed comment spacing so stuff fit in 80
 *                              columns; deleted some leading numbers in
 *                              comments (reasoning as above).
 *                              Also removed many "must be last" comments,
 *                              esp. where less-than-accurate.
 *      <9>    10jan2000 dlb    extensive - see changes.340             S
 *      <8>    04nov1999 cdl    added killed_by
 *      <7>    29sep1999 BCR    Added comments showing where uniques are
 *      <6>    25sep1999 CDL    Added commands
 *      <5>    09sep1999 BWR    Removed Great Swords skill
 *      <4>    06aug1999 BWR    added branch and level types
 *      <3>    02jun1999 DML    beams, clouds, ench, ms, kill,
 *                              other minor changes
 *      <2>    26may1999 JDJ    Added a header guard.
 *      <1>    --/--/--  CDL    Created
 */


#ifndef ENUM_H
#define ENUM_H

enum ability_type
{
    ABIL_NON_ABILITY = -1,
    ABIL_SPIT_POISON = 1,              //    1
    ABIL_MAPPING,
    ABIL_TELEPORTATION,
    ABIL_BREATHE_FIRE,                 //    5
    ABIL_BLINK,
    ABIL_BREATHE_FROST,
    ABIL_BREATHE_POISON,
    ABIL_BREATHE_LIGHTNING,
    ABIL_SPIT_ACID,                    //   10
    ABIL_BREATHE_POWER,
    ABIL_EVOKE_BERSERK,
    ABIL_BREATHE_STICKY_FLAME,
    ABIL_BREATHE_STEAM,
    ABIL_FLY,                          //   15
    ABIL_SUMMON_MINOR_DEMON,
    ABIL_SUMMON_DEMONS,
    ABIL_HELLFIRE,
    ABIL_TORMENT,
    ABIL_RAISE_DEAD,                   //   20
    ABIL_CONTROL_DEMON,
    ABIL_TO_PANDEMONIUM,
    ABIL_CHANNELING,
    ABIL_THROW_FLAME,
    ABIL_THROW_FROST,                  //   25
    ABIL_BOLT_OF_DRAINING,
    ABIL_BREATHE_HELLFIRE,
    ABIL_FLY_II,
    ABIL_DELAYED_FIREBALL,
    ABIL_MUMMY_RESTORATION,            //   30
    ABIL_EVOKE_MAPPING,
    ABIL_EVOKE_TELEPORTATION,
    ABIL_EVOKE_BLINK,                  //   33
    ABIL_EVOKE_TURN_INVISIBLE = 51,    //   51
    ABIL_EVOKE_TURN_VISIBLE,
    ABIL_EVOKE_LEVITATE,
    ABIL_EVOKE_STOP_LEVITATING,
    ABIL_END_TRANSFORMATION,           //   55
    
    // Divine abilities
    ABIL_ZIN_RECITE = 110,             //  110
    ABIL_ZIN_SMITING,   
    ABIL_ZIN_REVITALISATION,
    ABIL_ZIN_SANCTUARY,
    ABIL_TSO_REPEL_UNDEAD = 120,       //  120
    ABIL_TSO_DIVINE_SHIELD,
    ABIL_TSO_ANNIHILATE_UNDEAD,
    ABIL_TSO_CLEANSING_FLAME,
    ABIL_TSO_SUMMON_DAEVA,             //  124
    ABIL_KIKU_RECALL_UNDEAD_SLAVES = 130,   //  130
    ABIL_KIKU_ENSLAVE_UNDEAD = 132,         //  132
    ABIL_KIKU_INVOKE_DEATH,                 //  133
    ABIL_YRED_ANIMATE_CORPSE = 140,         //  140
    ABIL_YRED_RECALL_UNDEAD,
    ABIL_YRED_ANIMATE_DEAD,
    ABIL_YRED_DRAIN_LIFE,
    ABIL_YRED_CONTROL_UNDEAD,               //  144
    // 160 - reserved for Vehumet
    ABIL_OKAWARU_MIGHT = 170,               //  170
    // Okawaru no longer heals (JPEG)
    ABIL_OKAWARU_HASTE = 172,               //  172
    ABIL_MAKHLEB_MINOR_DESTRUCTION = 180,      //  180
    ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB,
    ABIL_MAKHLEB_MAJOR_DESTRUCTION,
    ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB,   //  183
    ABIL_SIF_MUNA_CHANNEL_ENERGY = 190,        //  190
    ABIL_SIF_MUNA_FORGET_SPELL,
    ABIL_TROG_BURN_BOOKS = 199,
    ABIL_TROG_BERSERK = 200,          //  200
    ABIL_TROG_REGENERATION,
    ABIL_TROG_BROTHERS_IN_ARMS,                   //  202
    ABIL_ELYVILON_DESTROY_WEAPONS = 219,
    ABIL_ELYVILON_LESSER_HEALING = 220,         //  220
    ABIL_ELYVILON_PURIFICATION,
    ABIL_ELYVILON_HEALING,
    ABIL_ELYVILON_RESTORATION,
    ABIL_ELYVILON_GREATER_HEALING,              //  224
    ABIL_LUGONU_ABYSS_EXIT,
    ABIL_LUGONU_BEND_SPACE,
    ABIL_LUGONU_BANISH,
    ABIL_LUGONU_CORRUPT,
    ABIL_LUGONU_ABYSS_ENTER,
    ABIL_NEMELEX_PEEK_DECK,
    ABIL_NEMELEX_DRAW_CARD,
    ABIL_NEMELEX_TRIPLE_DRAW,
    ABIL_NEMELEX_MARK_DECK,
    ABIL_NEMELEX_STACK_DECK,
    ABIL_BEOGH_SMITING,
    ABIL_BEOGH_RECALL_ORCISH_FOLLOWERS,

    ABIL_CHARM_SNAKE,
    ABIL_TRAN_SERPENT_OF_HELL,
    ABIL_ROTTING,
    ABIL_TORMENT_II,
    ABIL_TRAN_BAT,
    ABIL_RENOUNCE_RELIGION = 250       //  250
};

enum activity_interrupt_type
{
    AI_FORCE_INTERRUPT  = 0,        // Forcibly kills any activity that can be
                                    // interrupted.
    AI_KEYPRESS,
    AI_FULL_HP,                     // Player is fully healed
    AI_FULL_MP,                     // Player has recovered all mp
    AI_STATUE,                      // Bad statue has come into view
    AI_HUNGRY,                      // Hunger increased
    AI_MESSAGE,                     // Message was displayed
    AI_HP_LOSS,
    AI_BURDEN_CHANGE,
    AI_STAT_CHANGE,
    AI_SEE_MONSTER,
    AI_MONSTER_ATTACKS,
    AI_TELEPORT,
    AI_HIT_MONSTER,                 // Player hit monster (invis or
                                    // mimic) during travel/explore.

    // Always the last.
    NUM_AINTERRUPTS
};

enum actor_type
{
    ACT_NONE = -1,
    ACT_PLAYER,
    ACT_MONSTER
};
 
enum attribute_type
{
    ATTR_DIVINE_LIGHTNING_PROTECTION,  //    0
    ATTR_TRANSFORMATION,
    ATTR_CARD_COUNTDOWN,
    ATTR_NUM_DEMONIC_POWERS,
    ATTR_WAS_SILENCED,          //jmf: added for silenced messages
    ATTR_GOD_GIFT_COUNT,        //jmf: added to help manage god gift giving
    ATTR_DELAYED_FIREBALL,      // bwr: reserve fireballs
    ATTR_HELD,                  // caught in a net
    ATTR_ABYSS_ENTOURAGE,       // maximum number of hostile monsters in
                                // sight of the player while in the Abyss.
    ATTR_DIVINE_SHIELD,         // strength of TSO's Divine Shield
    ATTR_UNIQUE_RUNES,
    ATTR_DEMONIC_RUNES,
    ATTR_ABYSSAL_RUNES,
    ATTR_RUNES_IN_ZOT,
    NUM_ATTRIBUTES
};

enum quiver_type
{
    QUIVER_THROW,           // no launcher wielded -> darts, stones, ...
    QUIVER_BOW,             // wielded bow -> arrows
    QUIVER_SLING,           // wielded sling -> stones, sling bullets
    QUIVER_CROSSBOW,        // wielded crossbow -> bolts
    QUIVER_HAND_CROSSBOW,   // wielded hand crossbow -> darts
    QUIVER_BLOWGUN,         // wielded blowgun -> needles
    NUM_QUIVER
};

enum beam_type                  // beam[].flavour
{
    BEAM_NONE,                    //    0
    
    BEAM_MISSILE,
    BEAM_MMISSILE,                //    and similarly irresistible things
    BEAM_FIRE,
    BEAM_COLD,
    BEAM_MAGIC,
    BEAM_ELECTRICITY,
    BEAM_POISON,
    BEAM_NEG,
    BEAM_ACID,
    BEAM_MIASMA,

    BEAM_SPORE,
    BEAM_POISON_ARROW,
    BEAM_HELLFIRE,
    BEAM_NAPALM,
    BEAM_STEAM,
    BEAM_HELLFROST,
    BEAM_ENERGY,
    BEAM_HOLY,                    //   aka beam of cleansing, golden flame
    BEAM_FRAG,
    BEAM_LAVA,
    BEAM_BACKLIGHT,
    BEAM_SLEEP,
    BEAM_ICE,
    BEAM_NUKE,
    BEAM_RANDOM,                  //   currently translates into FIRE..ACID

    // These used to be handled in the colour field:
    BEAM_SLOW,                  // BLACK
    BEAM_HASTE,                 // BLUE
    BEAM_HEALING,               // GREEN
    BEAM_PARALYSIS,             // CYAN
    BEAM_CONFUSION,             // RED
    BEAM_INVISIBILITY,          // MAGENTA
    BEAM_DIGGING,               // BROWN
    BEAM_TELEPORT,              // LIGHTGREY
    BEAM_POLYMORPH,             // DARKGREY
    BEAM_CHARM,                 // LIGHTBLUE
    BEAM_BANISH,                // LIGHTGREEN
    BEAM_DEGENERATE,            // LIGHTCYAN
    BEAM_ENSLAVE_UNDEAD,        // LIGHTRED
    BEAM_PAIN,                  // LIGHTMAGENTA
    BEAM_DISPEL_UNDEAD,         // YELLOW
    BEAM_DISINTEGRATION,        // WHITE
    BEAM_ENSLAVE_DEMON,         // colour "16"
    BEAM_BLINK,
    BEAM_PETRIFY,

    // new beams for evaporate
    BEAM_POTION_STINKING_CLOUD,
    BEAM_POTION_POISON,
    BEAM_POTION_MIASMA,
    BEAM_POTION_STEAM,
    BEAM_POTION_FIRE,
    BEAM_POTION_COLD,
    BEAM_POTION_BLACK_SMOKE,
    BEAM_POTION_BLUE_SMOKE,
    BEAM_POTION_PURP_SMOKE,
    BEAM_POTION_RANDOM,

    BEAM_LINE_OF_SIGHT          // only used for checking monster LOS
};

enum book_type
{
    BOOK_MINOR_MAGIC_I,                //    0
    BOOK_MINOR_MAGIC_II,
    BOOK_MINOR_MAGIC_III,
    BOOK_CONJURATIONS_I,
    BOOK_CONJURATIONS_II,
    BOOK_FLAMES,                       //    5
    BOOK_FROST,
    BOOK_SUMMONINGS,
    BOOK_FIRE,
    BOOK_ICE,
    BOOK_SURVEYANCES,                  //   10
    BOOK_SPATIAL_TRANSLOCATIONS,
    BOOK_ENCHANTMENTS,
    BOOK_YOUNG_POISONERS,
    BOOK_TEMPESTS,
    BOOK_DEATH,                        //   15
    BOOK_HINDERANCE,
    BOOK_CHANGES,
    BOOK_TRANSFIGURATIONS,
    BOOK_PRACTICAL_MAGIC,
    BOOK_WAR_CHANTS,                   //   20
    BOOK_CLOUDS,
    BOOK_HEALING,
    BOOK_NECROMANCY,
    BOOK_NECRONOMICON,
    BOOK_CALLINGS,                     //   25
    BOOK_CHARMS,
    BOOK_DEMONOLOGY,
    BOOK_AIR,
    BOOK_SKY,
    BOOK_DIVINATIONS,                  //   30
    BOOK_WARP,
    BOOK_ENVENOMATIONS,
    BOOK_ANNIHILATIONS,
    BOOK_UNLIFE,
    BOOK_DESTRUCTION,                  //   35
    BOOK_CONTROL,
    BOOK_MUTATIONS,
    BOOK_TUKIMA,
    BOOK_GEOMANCY,
    BOOK_EARTH,                        //   40
    BOOK_MANUAL,
    BOOK_WIZARDRY,
    BOOK_POWER,
    BOOK_CANTRIPS,                     //jmf: 04jan2000
    BOOK_PARTY_TRICKS,           // 45 //jmf: 04jan2000
    BOOK_BEASTS,
    BOOK_STALKING,         // renamed -- assassination was confusing  -- bwr
    NUM_BOOKS
};

enum branch_type                // you.where_are_you
{
    BRANCH_MAIN_DUNGEON,        //    0
    BRANCH_ECUMENICAL_TEMPLE,
    BRANCH_ORCISH_MINES,
    BRANCH_ELVEN_HALLS,
    BRANCH_LAIR,
    BRANCH_SWAMP,
    BRANCH_SHOALS,
    BRANCH_SLIME_PITS,
    BRANCH_SNAKE_PIT,
    BRANCH_HIVE,
    BRANCH_VAULTS,
    BRANCH_HALL_OF_BLADES,
    BRANCH_CRYPT,
    BRANCH_TOMB,
    BRANCH_VESTIBULE_OF_HELL,
    BRANCH_DIS,
    BRANCH_GEHENNA,
    BRANCH_COCYTUS,
    BRANCH_TARTARUS,
    BRANCH_INFERNO,             // unimplemented
    BRANCH_THE_PIT,             // unimplemented
    BRANCH_HALL_OF_ZOT,
    BRANCH_CAVERNS,             // unimplemented
    NUM_BRANCHES
};

enum builder_rc_type
{
    BUILD_QUIT = -1,            // all done, don't continue
    BUILD_SKIP = 1,             // skip further generation
    BUILD_CONTINUE = 0          // continue generation
};

enum burden_state_type          // you.burden_state
{
    BS_UNENCUMBERED,            //    0
    BS_ENCUMBERED = 2,          //    2
    BS_OVERLOADED = 5           //    5
};

enum canned_message_type
{
    MSG_SOMETHING_APPEARS,
    MSG_NOTHING_HAPPENS,
    MSG_YOU_RESIST,
    MSG_TOO_BERSERK,
    MSG_PRESENT_FORM,
    MSG_NOTHING_CARRIED,
    MSG_CANNOT_DO_YET,
    MSG_OK,
    MSG_UNTHINKING_ACT,
    MSG_SPELL_FIZZLES,
    MSG_HUH,
    MSG_EMPTY_HANDED
};

enum char_set_type
{
    CSET_ASCII,         // flat 7-bit ASCII
    CSET_IBM,           // 8-bit ANSI/Code Page 437
    CSET_DEC,           // 8-bit DEC, 0xE0-0xFF shifted for line drawing chars
    CSET_UNICODE,       // Unicode.
    NUM_CSET
};

enum cloud_type 
{
    CLOUD_NONE,
    CLOUD_FIRE,
    CLOUD_STINK,
    CLOUD_COLD,
    CLOUD_POISON,
    CLOUD_GREY_SMOKE,
    CLOUD_BLUE_SMOKE,
    CLOUD_PURP_SMOKE,
    CLOUD_STEAM,
    CLOUD_MIASMA,
    CLOUD_BLACK_SMOKE,
    CLOUD_MIST,
    CLOUD_RANDOM = 98,
    CLOUD_DEBUGGING = 99    //   99: used once as 'nonexistent cloud' {dlb}
};

enum command_type
{
    CMD_NO_CMD = 1000,
    CMD_MOVE_NOWHERE,
    CMD_MOVE_LEFT,
    CMD_MOVE_DOWN,
    CMD_MOVE_UP,
    CMD_MOVE_RIGHT,
    CMD_MOVE_UP_LEFT,
    CMD_MOVE_DOWN_LEFT,
    CMD_MOVE_UP_RIGHT,
    CMD_MOVE_DOWN_RIGHT,
    CMD_RUN_LEFT,
    CMD_RUN_DOWN,
    CMD_RUN_UP,
    CMD_RUN_RIGHT,
    CMD_RUN_UP_LEFT,
    CMD_RUN_DOWN_LEFT,
    CMD_RUN_UP_RIGHT,
    CMD_RUN_DOWN_RIGHT,
    CMD_OPEN_DOOR_LEFT,
    CMD_OPEN_DOOR_DOWN,
    CMD_OPEN_DOOR_UP,
    CMD_OPEN_DOOR_RIGHT,
    CMD_OPEN_DOOR_UP_LEFT,
    CMD_OPEN_DOOR_DOWN_LEFT,
    CMD_OPEN_DOOR_UP_RIGHT,
    CMD_OPEN_DOOR_DOWN_RIGHT,
    CMD_OPEN_DOOR,
    CMD_CLOSE_DOOR,
    CMD_REST,
    CMD_GO_UPSTAIRS,
    CMD_GO_DOWNSTAIRS,
    CMD_TOGGLE_AUTOPICKUP,
    CMD_PICKUP,
    CMD_DROP,
    CMD_BUTCHER,
    CMD_INSPECT_FLOOR,
    CMD_EXAMINE_OBJECT,
    CMD_EVOKE,
    CMD_WIELD_WEAPON,
    CMD_WEAPON_SWAP,
    CMD_THROW,
    CMD_FIRE,
    CMD_WEAR_ARMOUR,
    CMD_REMOVE_ARMOUR,
    CMD_WEAR_JEWELLERY,
    CMD_REMOVE_JEWELLERY,
    CMD_LIST_WEAPONS,
    CMD_LIST_ARMOUR,
    CMD_LIST_JEWELLERY,
    CMD_LIST_EQUIPMENT,
    CMD_ZAP_WAND,
    CMD_CAST_SPELL,
    CMD_MEMORISE_SPELL,
    CMD_USE_ABILITY,
    CMD_PRAY,
    CMD_EAT,
    CMD_QUAFF,
    CMD_READ,
    CMD_LOOK_AROUND,
    CMD_SEARCH,
    CMD_SHOUT,
    CMD_DISARM_TRAP,
    CMD_CHARACTER_DUMP,
    CMD_DISPLAY_COMMANDS,
    CMD_DISPLAY_INVENTORY,
    CMD_DISPLAY_KNOWN_OBJECTS,
    CMD_DISPLAY_MUTATIONS,
    CMD_DISPLAY_SKILLS,
    CMD_DISPLAY_MAP,
    CMD_DISPLAY_OVERMAP,
    CMD_DISPLAY_RELIGION,
    CMD_DISPLAY_CHARACTER_STATUS,
    CMD_DISPLAY_SPELLS,
    CMD_EXPERIENCE_CHECK,
    CMD_GET_VERSION,
    CMD_ADJUST_INVENTORY,
    CMD_REPLAY_MESSAGES,
    CMD_REDRAW_SCREEN,
    CMD_MACRO_ADD,
    CMD_SAVE_GAME,
    CMD_SAVE_GAME_NOW,
    CMD_SUSPEND_GAME,
    CMD_QUIT,
    CMD_WIZARD,
    CMD_DESTROY_ITEM,

    CMD_MARK_STASH,
    CMD_FORGET_STASH,
    CMD_SEARCH_STASHES,
    CMD_EXPLORE,
    CMD_INTERLEVEL_TRAVEL,
    CMD_FIX_WAYPOINT,

    CMD_CLEAR_MAP,
    CMD_INSCRIBE_ITEM,
    CMD_TOGGLE_AUTOPRAYER,
    CMD_MAKE_NOTE,
    CMD_RESISTS_SCREEN,

    CMD_READ_MESSAGES,
    
    CMD_MOUSE_MOVE,
    CMD_MOUSE_CLICK,

    CMD_ANNOTATE_LEVEL,

    /* overmap commands */
    CMD_MAP_CLEAR_MAP,
    CMD_MAP_ADD_WAYPOINT,
    CMD_MAP_EXCLUDE_AREA,
    CMD_MAP_CLEAR_EXCLUDES,

    CMD_MAP_MOVE_LEFT,
    CMD_MAP_MOVE_DOWN,
    CMD_MAP_MOVE_UP,
    CMD_MAP_MOVE_RIGHT,
    CMD_MAP_MOVE_UP_LEFT,
    CMD_MAP_MOVE_DOWN_LEFT,
    CMD_MAP_MOVE_UP_RIGHT,
    CMD_MAP_MOVE_DOWN_RIGHT,

    CMD_MAP_JUMP_LEFT,
    CMD_MAP_JUMP_DOWN,
    CMD_MAP_JUMP_UP,
    CMD_MAP_JUMP_RIGHT,
    CMD_MAP_JUMP_UP_LEFT,
    CMD_MAP_JUMP_DOWN_LEFT,
    CMD_MAP_JUMP_UP_RIGHT,
    CMD_MAP_JUMP_DOWN_RIGHT,

    CMD_MAP_SCROLL_DOWN,
    CMD_MAP_SCROLL_UP,

    CMD_MAP_FIND_UPSTAIR,
    CMD_MAP_FIND_DOWNSTAIR,
    CMD_MAP_FIND_YOU,
    CMD_MAP_FIND_PORTAL,
    CMD_MAP_FIND_TRAP,
    CMD_MAP_FIND_ALTAR,
    CMD_MAP_FIND_EXCLUDED,
    CMD_MAP_FIND_F,
    CMD_MAP_FIND_WAYPOINT,
    CMD_MAP_FIND_STASH,

    CMD_MAP_GOTO_TARGET,

    CMD_MAP_WIZARD_TELEPORT,

    CMD_MAP_EXIT_MAP,

    /* targeting commands */
    CMD_TARGET_DOWN_LEFT,
    CMD_TARGET_DOWN,
    CMD_TARGET_DOWN_RIGHT,
    CMD_TARGET_LEFT,
    CMD_TARGET_RIGHT,
    CMD_TARGET_UP_LEFT,
    CMD_TARGET_UP,
    CMD_TARGET_UP_RIGHT,

    CMD_TARGET_DIR_DOWN_LEFT,
    CMD_TARGET_DIR_DOWN,
    CMD_TARGET_DIR_DOWN_RIGHT,
    CMD_TARGET_DIR_LEFT,
    CMD_TARGET_DIR_RIGHT,
    CMD_TARGET_DIR_UP_LEFT,
    CMD_TARGET_DIR_UP,
    CMD_TARGET_DIR_UP_RIGHT,

    CMD_TARGET_CYCLE_TARGET_MODE,
    CMD_TARGET_PREV_TARGET,
    CMD_TARGET_MAYBE_PREV_TARGET,
    CMD_TARGET_SELECT,
    CMD_TARGET_SELECT_ENDPOINT,
    CMD_TARGET_OBJ_CYCLE_BACK,
    CMD_TARGET_OBJ_CYCLE_FORWARD,
    CMD_TARGET_CYCLE_FORWARD,
    CMD_TARGET_CYCLE_BACK,
    CMD_TARGET_CYCLE_BEAM,
    CMD_TARGET_HIDE_BEAM,
    CMD_TARGET_CENTER,
    CMD_TARGET_CANCEL,
    CMD_TARGET_OLD_SPACE,
    CMD_TARGET_FIND_TRAP,
    CMD_TARGET_FIND_PORTAL,
    CMD_TARGET_FIND_ALTAR,
    CMD_TARGET_FIND_UPSTAIR,
    CMD_TARGET_FIND_DOWNSTAIR,
    CMD_TARGET_FIND_YOU,
    CMD_TARGET_DESCRIBE,
    CMD_TARGET_WIZARD_MAKE_FRIENDLY,
    CMD_TARGET_WIZARD_MAKE_SHOUT,
    CMD_TARGET_WIZARD_GIVE_ITEM,
    CMD_TARGET_HELP,

#ifdef USE_TILE
    CMD_TARGET_MOUSE_MOVE,
    CMD_TARGET_MOUSE_SELECT,
    CMD_EDIT_PREFS,
    CMD_EDIT_PLAYER_TILE,
    CMD_USE_ITEM,
    CMD_VIEW_ITEM,
#endif

    // Disable/enable -more- prompts.
    CMD_DISABLE_MORE,
    CMD_ENABLE_MORE,
    
    // [ds] Silently ignored, requests another round of input.
    CMD_NEXT_CMD,

    // Repeat previous command
    CMD_PREV_CMD_AGAIN,

    // Repeat next command a given number of times
    CMD_REPEAT_CMD,

    // Stick the keyspresses of the command to be repeated into the
    // input buffer.
    CMD_REPEAT_KEYS
};

enum conduct_type
{
    DID_NECROMANCY = 1,                 // vamp/drain/pain wpns, Zong/Curses
    DID_UNHOLY,                         // demon wpns, demon spells
    DID_ATTACK_HOLY,
    DID_ATTACK_FRIEND,
    DID_FRIEND_DIES,
    DID_STABBING,
    DID_POISON,
    DID_DEDICATED_BUTCHERY,
    // killings need no longer be dedicated (JPEG)
    DID_KILL_LIVING,
    DID_KILL_UNDEAD,
    DID_KILL_DEMON,
    DID_KILL_NATURAL_EVIL,    // TSO
    DID_KILL_MUTATOR_OR_ROTTER, // Zin
    DID_KILL_WIZARD,
    DID_KILL_PRIEST,
    DID_KILL_ANGEL,
    DID_LIVING_KILLED_BY_UNDEAD_SLAVE,
    DID_LIVING_KILLED_BY_SERVANT,
    DID_UNDEAD_KILLED_BY_SERVANT,
    DID_DEMON_KILLED_BY_SERVANT,
    DID_NATURAL_EVIL_KILLED_BY_SERVANT, // TSO
    DID_ANGEL_KILLED_BY_SERVANT,
    DID_SPELL_MEMORISE,
    DID_SPELL_CASTING,
    DID_SPELL_PRACTISE,
    DID_SPELL_NONUTILITY,               // unused
    DID_CARDS,
    DID_STIMULANTS,                     // unused
    DID_DRINK_BLOOD,
    DID_CANNIBALISM,
    DID_EAT_MEAT,                       // unused
    DID_EAT_SOULED_BEING,  // Zin
    DID_CREATED_LIFE,                   // unused
    DID_DELIBERATE_MUTATING,

    NUM_CONDUCTS
};

enum confirm_level_type
{
    CONFIRM_NONE_EASY,
    CONFIRM_SAFE_EASY,
    CONFIRM_ALL_EASY
};

enum death_knight_type
{
    DK_NO_SELECTION,
    DK_NECROMANCY,
    DK_YREDELEMNUL,
    DK_RANDOM
};

enum startup_book_type
{
    SBT_NO_SELECTION = 0,
    SBT_FIRE,
    SBT_COLD,
    SBT_SUMM,
    SBT_RANDOM
};

// When adding new delays, update their names in delay.cc, or bad things will
// happen.
enum delay_type
{
    DELAY_NOT_DELAYED,                  
    DELAY_EAT,                         
    DELAY_ARMOUR_ON,
    DELAY_ARMOUR_OFF,
    DELAY_JEWELLERY_ON,
    DELAY_MEMORISE,
    DELAY_BUTCHER,
    DELAY_WEAPON_SWAP,                 // for easy_butcher
    DELAY_PASSWALL,
    DELAY_DROP_ITEM,
    DELAY_MULTIDROP,
    DELAY_ASCENDING_STAIRS,
    DELAY_DESCENDING_STAIRS,
    DELAY_RECITE,  // Zin's Recite invocation

    // [dshaligram] Shift-running, resting, travel and macros are now
    // also handled as delays.
    DELAY_RUN,
    DELAY_REST,
    DELAY_TRAVEL,

    DELAY_MACRO,

    DELAY_INTERRUPTIBLE,                // simple interruptible delay
    DELAY_UNINTERRUPTIBLE,              // simple uninterruptible delay

    NUM_DELAYS
};

enum description_level_type
{
    DESC_CAP_THE,                      // 0
    DESC_NOCAP_THE,                    // 1
    DESC_CAP_A,                        // 2
    DESC_NOCAP_A,                      // 3
    DESC_CAP_YOUR,                     // 4
    DESC_NOCAP_YOUR,                   // 5
    DESC_PLAIN,                        // 6
    DESC_NOCAP_ITS,                    // 7
    DESC_INVENTORY_EQUIP,              // 8
    DESC_INVENTORY,                    // 9

    // Partial item names.
    DESC_BASENAME,                     // Base name of item subtype
    DESC_QUALNAME,                     // Name without articles, quantities,
                                       // enchantments.
    DESC_DBNAME,                       // Name with which to look up item
                                       // description in the db.

    DESC_NONE
};

enum game_direction_type
{
    GDT_NONE,
    GDT_DESCENDING,
    GDT_ASCENDING
};

enum level_flag_type
{
    LFLAG_NONE = 0,

    LFLAG_NO_TELE_CONTROL = (1 << 0), // Teleport control not allowed.
    LFLAG_NOT_MAPPABLE    = (1 << 1), // Level not mappable (like Abyss).
    LFLAG_NO_MAGIC_MAP    = (1 << 2)  // Level can't be magic mapped.
};

// NOTE: The order of these is very important to their usage!
// [dshaligram] If adding/removing from this list, also update view.cc!
enum dungeon_char_type
{
    DCHAR_WALL,                 //  0
    DCHAR_WALL_MAGIC,
    DCHAR_FLOOR,
    DCHAR_FLOOR_MAGIC,
    DCHAR_DOOR_OPEN,
    DCHAR_DOOR_CLOSED,          //  5
    DCHAR_TRAP,
    DCHAR_STAIRS_DOWN,
    DCHAR_STAIRS_UP,
    DCHAR_ALTAR,
    DCHAR_ARCH,                 // 10
    DCHAR_FOUNTAIN,
    DCHAR_WAVY,
    DCHAR_STATUE,
    DCHAR_INVIS_EXPOSED,
    DCHAR_ITEM_DETECTED,        // 15
    DCHAR_ITEM_ORB,
    DCHAR_ITEM_WEAPON,
    DCHAR_ITEM_ARMOUR,
    DCHAR_ITEM_WAND,
    DCHAR_ITEM_FOOD,            // 20
    DCHAR_ITEM_SCROLL,
    DCHAR_ITEM_RING,
    DCHAR_ITEM_POTION,
    DCHAR_ITEM_MISSILE,
    DCHAR_ITEM_BOOK,            // 25
    DCHAR_ITEM_STAVE,
    DCHAR_ITEM_MISCELLANY,
    DCHAR_ITEM_CORPSE,
    DCHAR_ITEM_GOLD,
    DCHAR_ITEM_AMULET,          // 30
    DCHAR_CLOUD,                // 31

    DCHAR_SPACE,
    DCHAR_FIRED_FLASK,
    DCHAR_FIRED_BOLT,
    DCHAR_FIRED_CHUNK,
    DCHAR_FIRED_BOOK,
    DCHAR_FIRED_WEAPON,
    DCHAR_FIRED_ZAP,
    DCHAR_FIRED_BURST,
    DCHAR_FIRED_STICK,
    DCHAR_FIRED_TRINKET,
    DCHAR_FIRED_SCROLL,
    DCHAR_FIRED_DEBUG,
    DCHAR_FIRED_ARMOUR,
    DCHAR_FIRED_MISSILE,
    DCHAR_EXPLOSION,
    
    NUM_DCHAR_TYPES
};

// When adding:
// 
// * New stairs/portals: update grid_stair_direction.
// * Any: edit view.cc and add a glyph and colour for the feature.
// * Any: edit direct.cc and add a description for the feature.
// * Any: edit dat/descript.txt and add a long description if appropriate.
// * Any: check the grid_* functions in misc.cc and make sure
//   they return sane values for your new feature.
// * Any: edit dungeon.cc and add a symbol to map_feature() and
//        vault_grid() for the feature, if you want vault maps to
//        be able to use it.  If you do, also update
//        docs/level-design.txt with the new symbol.
// * Any: edit luadgn.cc and add the feature's name to the dngn_feature_names
//        array, if you want vault map Lua code to be able to use the
//        feature, and/or you want to be able to create the feature
//        using the "create feature by name" wizard command.
// Also take note of MINMOVE and MINSEE above.
//
enum dungeon_feature_type
{
    DNGN_UNSEEN,                       //    0
    DNGN_CLOSED_DOOR,
    DNGN_SECRET_DOOR,
    DNGN_WAX_WALL,
    DNGN_METAL_WALL,
    DNGN_GREEN_CRYSTAL_WALL,           //    5
    DNGN_ROCK_WALL,
    DNGN_STONE_WALL,
    DNGN_PERMAROCK_WALL,               //    8 - for undiggable walls
    DNGN_CLEAR_ROCK_WALL,              //    9 - Transparent
    DNGN_CLEAR_STONE_WALL,             //   10 - Transparent
    DNGN_CLEAR_PERMAROCK_WALL,         //   11 - Transparent
    DNGN_ORCISH_IDOL,                  //   12 - Can see past

    // XXX: lowest/highest grid value which is a wall
    DNGN_MINWALL = DNGN_WAX_WALL,
    DNGN_MAXWALL = DNGN_CLEAR_PERMAROCK_WALL,

    // Random wall types for big rooms
    DNGN_RNDWALL_MIN = DNGN_METAL_WALL,
    DNGN_RNDWALL_MAX = DNGN_STONE_WALL,

    // XXX: highest grid value which is opaque
    DNGN_MAXOPAQUE = DNGN_PERMAROCK_WALL,

    // XXX: lowest grid value which can be seen through
    DNGN_MINSEE = DNGN_CLEAR_ROCK_WALL,

    DNGN_GRANITE_STATUE = 21,          //   21
    DNGN_STATUE_RESERVED_1,
    DNGN_STATUE_RESERVED_2,

    // XXX: lowest grid value which can be passed by walking etc.
    DNGN_MINMOVE = 31,

    DNGN_LAVA = 61,                    //   61
    DNGN_DEEP_WATER,                   //   62
    DNGN_SHALLOW_WATER = 65,           //   65
    DNGN_WATER_STUCK,

    DNGN_FLOOR,                        //   67
    DNGN_FLOOR_SPECIAL,     // currently only used for colouring bazaars
    DNGN_FLOOR_RESERVED,
    DNGN_EXIT_HELL,                    //   70
    DNGN_ENTER_HELL,                   //   71
    DNGN_OPEN_DOOR,                    //   72
    DNGN_TRAP_MECHANICAL = 75,         //   75
    DNGN_TRAP_MAGICAL,
    DNGN_TRAP_NATURAL,
    DNGN_UNDISCOVERED_TRAP,            //   78

    DNGN_ENTER_SHOP = 80,              //   80
    DNGN_ENTER_LABYRINTH,

    DNGN_STONE_STAIRS_DOWN_I,
    DNGN_STONE_STAIRS_DOWN_II,
    DNGN_STONE_STAIRS_DOWN_III,
    DNGN_ROCK_STAIRS_DOWN,             //  85 - now escape hatch (Stonesoup 0.3)

    // corresponding up stairs (same order as above)
    DNGN_STONE_STAIRS_UP_I,
    DNGN_STONE_STAIRS_UP_II,
    DNGN_STONE_STAIRS_UP_III,
    DNGN_ROCK_STAIRS_UP,               //  89 - now escape hatch (Stonesoup 0.3)

    // Various gates
    DNGN_ENTER_DIS = 92,               //   92
    DNGN_ENTER_GEHENNA,
    DNGN_ENTER_COCYTUS,
    DNGN_ENTER_TARTARUS,               //   95
    DNGN_ENTER_ABYSS,
    DNGN_EXIT_ABYSS,
    DNGN_STONE_ARCH,
    DNGN_ENTER_PANDEMONIUM,
    DNGN_EXIT_PANDEMONIUM,             //  100
    DNGN_TRANSIT_PANDEMONIUM,          //  101

    DNGN_BUILDER_SPECIAL_WALL = 105,   //  105; builder() only
    DNGN_BUILDER_SPECIAL_FLOOR,        //  106; builder() only

    // Entrances to various branches
    DNGN_ENTER_FIRST_BRANCH = 110,     //  110
    DNGN_ENTER_ORCISH_MINES = DNGN_ENTER_FIRST_BRANCH,
    DNGN_ENTER_HIVE,
    DNGN_ENTER_LAIR,
    DNGN_ENTER_SLIME_PITS,
    DNGN_ENTER_VAULTS,
    DNGN_ENTER_CRYPT,                //  115
    DNGN_ENTER_HALL_OF_BLADES,
    DNGN_ENTER_ZOT,
    DNGN_ENTER_TEMPLE,
    DNGN_ENTER_SNAKE_PIT,
    DNGN_ENTER_ELVEN_HALLS,            //  120
    DNGN_ENTER_TOMB,
    DNGN_ENTER_SWAMP,                  //  122
    DNGN_ENTER_SHOALS,
    DNGN_ENTER_RESERVED_2,
    DNGN_ENTER_RESERVED_3,
    DNGN_ENTER_RESERVED_4,             // 126 
    DNGN_ENTER_LAST_BRANCH = DNGN_ENTER_RESERVED_4,

    // Exits from various branches
    // Order must be the same as above
    DNGN_RETURN_FROM_FIRST_BRANCH = 130, //  130
    DNGN_RETURN_FROM_ORCISH_MINES = DNGN_RETURN_FROM_FIRST_BRANCH,
    DNGN_RETURN_FROM_HIVE,
    DNGN_RETURN_FROM_LAIR,
    DNGN_RETURN_FROM_SLIME_PITS,
    DNGN_RETURN_FROM_VAULTS,
    DNGN_RETURN_FROM_CRYPT,            //  135
    DNGN_RETURN_FROM_HALL_OF_BLADES,
    DNGN_RETURN_FROM_ZOT,
    DNGN_RETURN_FROM_TEMPLE,
    DNGN_RETURN_FROM_SNAKE_PIT,
    DNGN_RETURN_FROM_ELVEN_HALLS,      //  140
    DNGN_RETURN_FROM_TOMB,
    DNGN_RETURN_FROM_SWAMP,               //  142
    DNGN_RETURN_FROM_SHOALS,
    DNGN_RETURN_RESERVED_2,
    DNGN_RETURN_RESERVED_3,
    DNGN_RETURN_RESERVED_4,             // 146
    DNGN_RETURN_FROM_LAST_BRANCH = DNGN_RETURN_RESERVED_4,

    // Portals to various places unknown.
    DNGN_ENTER_PORTAL_VAULT = 160,
    DNGN_EXIT_PORTAL_VAULT,

    // Order of altars must match order of gods (god_type)
    DNGN_ALTAR_FIRST_GOD = 180,        // 180
    DNGN_ALTAR_ZIN = DNGN_ALTAR_FIRST_GOD,
    DNGN_ALTAR_SHINING_ONE,
    DNGN_ALTAR_KIKUBAAQUDGHA,
    DNGN_ALTAR_YREDELEMNUL,
    DNGN_ALTAR_XOM,
    DNGN_ALTAR_VEHUMET,                //  185
    DNGN_ALTAR_OKAWARU,
    DNGN_ALTAR_MAKHLEB,
    DNGN_ALTAR_SIF_MUNA,
    DNGN_ALTAR_TROG,
    DNGN_ALTAR_NEMELEX_XOBEH,          //  190
    DNGN_ALTAR_ELYVILON,               //  191
    DNGN_ALTAR_LUGONU,
    DNGN_ALTAR_BEOGH,
    DNGN_ALTAR_LAST_GOD = DNGN_ALTAR_BEOGH,

    DNGN_BLUE_FOUNTAIN = 200,          //  200
    DNGN_DRY_FOUNTAIN_I,
    DNGN_SPARKLING_FOUNTAIN,           // aka 'Magic Fountain' {dlb}
    DNGN_DRY_FOUNTAIN_II,
    DNGN_DRY_FOUNTAIN_III,
    DNGN_DRY_FOUNTAIN_IV,              //  205
    DNGN_DRY_FOUNTAIN_V,
    DNGN_DRY_FOUNTAIN_VI,
    DNGN_DRY_FOUNTAIN_VII,
    DNGN_DRY_FOUNTAIN_VIII,
    DNGN_PERMADRY_FOUNTAIN = 210,  // added (from dungeon.cc/maps.cc) 22jan2000 {dlb}

    NUM_REAL_FEATURES,

    // Real terrain must all occur before 256 to guarantee it fits
    // into the unsigned char used for the grid!

    // There aren't really terrain, but they're passed in and used 
    // to get their appearance character so I'm putting them here for now.
    DNGN_ITEM_ORB        = 256,
    DNGN_INVIS_EXPOSED   = 257,
    DNGN_ITEM_WEAPON     = 258,
    DNGN_ITEM_ARMOUR     = 259,
    DNGN_ITEM_WAND       = 260,
    DNGN_ITEM_FOOD       = 261,
    DNGN_ITEM_UNUSED_1   = 262,
    DNGN_ITEM_SCROLL     = 263,
    DNGN_ITEM_RING       = 264,
    DNGN_ITEM_POTION     = 265,
    DNGN_ITEM_MISSILE    = 266,
    DNGN_ITEM_BOOK       = 267,
    DNGN_ITEM_UNUSED_2   = 268,
    DNGN_ITEM_STAVE      = 269,
    DNGN_ITEM_MISCELLANY = 270,
    DNGN_ITEM_CORPSE     = 271,
    DNGN_ITEM_GOLD       = 272,
    DNGN_ITEM_AMULET     = 273,
    DNGN_ITEM_DETECTED   = 274,

    DNGN_CLOUD           = 280,
    NUM_FEATURES,                 // for use in lookup table in view.cc

    DNGN_RANDOM,
    DNGN_START_OF_MONSTERS = 297  // don't go past here! see view.cc
};

enum floor_property_type
{
    FPROP_NONE,          // 0
    FPROP_SANCTUARY_1,
    FPROP_SANCTUARY_2,
    FPROP_BLOODY // bloody floor and sanctuary are exclusive, so that's okay
};

enum duration_type
{
    DUR_INVIS,
    DUR_CONF,
    DUR_PARALYSIS,
    DUR_SLOW,
    DUR_BEHELD,
    DUR_HASTE,
    DUR_MIGHT,
    DUR_LEVITATION,
    DUR_BERSERKER,
    DUR_POISONING,

    DUR_CONFUSING_TOUCH,
    DUR_SURE_BLADE,
    DUR_BACKLIGHT,
    DUR_DEATHS_DOOR,
    DUR_FIRE_SHIELD,

    DUR_BUILDING_RAGE,          // countdown to starting berserk
    DUR_EXHAUSTED,              // fatigue counter for berserk

    DUR_LIQUID_FLAMES,
    DUR_ICY_ARMOUR,
    DUR_REPEL_MISSILES,
    DUR_PRAYER,
    DUR_PIETY_POOL,             // distribute piety over time
    DUR_DIVINE_SHIELD,          // duration of TSO's Divine Shield
    DUR_REGENERATION,
    DUR_SWIFTNESS,
    DUR_STONEMAIL,
    DUR_CONTROLLED_FLIGHT,
    DUR_TELEPORT,
    DUR_CONTROL_TELEPORT,
    DUR_BREATH_WEAPON,
    DUR_TRANSFORMATION,
    DUR_DEATH_CHANNEL,
    DUR_DEFLECT_MISSILES,
    DUR_FORESCRY,
    DUR_SEE_INVISIBLE,
    DUR_WEAPON_BRAND,                  // general "branding" spell counter
    DUR_SILENCE,
    DUR_CONDENSATION_SHIELD,
    DUR_STONESKIN,
    DUR_REPEL_UNDEAD,
    DUR_GOURMAND,
    DUR_BARGAIN,
    DUR_INSULATION,
    DUR_RESIST_POISON,
    DUR_RESIST_FIRE,
    DUR_RESIST_COLD,
    DUR_SLAYING,
    DUR_STEALTH,
    DUR_MAGIC_SHIELD,
    DUR_SLEEP,
    DUR_SAGE,
    NUM_DURATIONS
};

// This list must match the enchant_names array in mon-util.cc or Crawl
// will CRASH, we kid you not.
enum enchant_type
{
    ENCH_NONE = 0,                     //    0
    ENCH_SLOW,
    ENCH_HASTE,
    ENCH_FEAR,
    ENCH_CONFUSION,
    ENCH_INVIS,                        //    5
    ENCH_POISON,
    ENCH_BERSERK,
    ENCH_ROT,
    ENCH_SUMMON,
    ENCH_ABJ,                          //   10
    ENCH_BACKLIGHT,
    ENCH_CHARM,
    ENCH_STICKY_FLAME,
    ENCH_GLOWING_SHAPESHIFTER,
    ENCH_SHAPESHIFTER,                 //   15
    ENCH_TP,
    ENCH_SLEEP_WARY,
    ENCH_SUBMERGED,
    ENCH_SHORT_LIVED,
    ENCH_PARALYSIS,                    //   20
    ENCH_SICK,
    ENCH_SLEEPY,         // Monster can't wake until this wears off.
    ENCH_FATIGUE,        // Post-berserk fatigue.
    ENCH_HELD,           // caught in a net
    ENCH_BATTLE_FRENZY,  // Monster is in a battle frenzy
    ENCH_NEUTRAL,

    // Update enchantment names in mon-util.cc when adding or removing
    // enchantments.
    NUM_ENCHANTMENTS
};

enum enchant_retval
{
    ERV_FAIL,
    ERV_NEW,
    ERV_INCREASED
};

enum energy_use_type
{
    EUT_MOVE,
    EUT_SWIM,
    EUT_ATTACK,
    EUT_MISSILE,
    EUT_SPELL,
    EUT_SPECIAL,
    EUT_ITEM,
    EUT_PICKUP
};

enum equipment_type
{
    EQ_NONE = -1,

    EQ_WEAPON,                         //    0
    EQ_CLOAK,
    EQ_HELMET,
    EQ_GLOVES,
    EQ_BOOTS,
    EQ_SHIELD,                         //    5
    EQ_BODY_ARMOUR,
    EQ_LEFT_RING,
    EQ_RIGHT_RING,
    EQ_AMULET,
    NUM_EQUIP,

    // these aren't actual equipment slots, they're categories for functions
    EQ_STAFF            = 100,         // weapon with base_type OBJ_STAVES
    EQ_RINGS,                          // check both rings
    EQ_RINGS_PLUS,                     // check both rings and sum plus
    EQ_RINGS_PLUS2,                    // check both rings and sum plus2
    EQ_ALL_ARMOUR                      // check all armour types
};

enum feature_flag_type
{
    FFT_NONE          = 0,
    FFT_NOTABLE       = 0x1,           // should be noted for dungeon overview
    FFT_EXAMINE_HINT  = 0x2            // could get an "examine-this" hint.
};

enum flush_reason_type
{
    FLUSH_ON_FAILURE,                  // spell/ability failed to cast
    FLUSH_BEFORE_COMMAND,              // flush before getting a command
    FLUSH_ON_MESSAGE,                  // flush when printing a message
    FLUSH_ON_WARNING_MESSAGE,          // flush on MSGCH_WARN messages
    FLUSH_ON_DANGER_MESSAGE,           // flush on MSGCH_DANGER messages
    FLUSH_ON_PROMPT,                   // flush on MSGCH_PROMPT messages
    FLUSH_ON_UNSAFE_YES_OR_NO_PROMPT,  // flush when !safe set to yesno()
    FLUSH_LUA,                         // flush when Lua wants to flush
    FLUSH_KEY_REPLAY_CANCEL,           // flush when key replay is cancelled
    FLUSH_ABORT_MACRO,                 // something wrong with macro being
                                       // processed, so stop it
    FLUSH_REPLAY_SETUP_FAILURE,        // setup for key replay failed
    NUM_FLUSH_REASONS
};

// The order of this enum must match the order of DNGN_ALTAR_FOO.
enum god_type
{
    GOD_NO_GOD,                        //    0  -- must be zero
    GOD_ZIN,
    GOD_SHINING_ONE,
    GOD_KIKUBAAQUDGHA,
    GOD_YREDELEMNUL,
    GOD_XOM,                           //    5
    GOD_VEHUMET,
    GOD_OKAWARU,
    GOD_MAKHLEB,
    GOD_SIF_MUNA,
    GOD_TROG,                          //   10
    GOD_NEMELEX_XOBEH,
    GOD_ELYVILON,
    GOD_LUGONU,
    GOD_BEOGH,
    NUM_GODS,                          // always after last god

    GOD_RANDOM  = 100
};

enum hunger_state                  // you.hunger_state
{
    HS_RAVENOUS,                       //    0: not used within code, really
    HS_STARVING,
    HS_NEAR_STARVING,
    HS_VERY_HUNGRY,
    HS_HUNGRY,
    HS_SATIATED,                       // "not hungry" state
    HS_FULL,
    HS_VERY_FULL,
    HS_ENGORGED                        //    8
};

enum item_status_flag_type  // per item flags: ie. ident status, cursed status
{
    ISFLAG_KNOW_CURSE        = 0x00000001,  // curse status
    ISFLAG_KNOW_TYPE         = 0x00000002,  // artefact name, sub/special types
    ISFLAG_KNOW_PLUSES       = 0x00000004,  // to hit/to dam/to AC/charges
    ISFLAG_KNOW_PROPERTIES   = 0x00000008,  // know special artefact properties
    ISFLAG_IDENT_MASK        = 0x0000000F,  // mask of all id related flags

    // these three masks are of the minimal flags set upon using equipment:
    ISFLAG_EQ_WEAPON_MASK    = 0x0000000B,  // mask of flags for weapon equip
    ISFLAG_EQ_ARMOUR_MASK    = 0x0000000F,  // mask of flags for armour equip
    ISFLAG_EQ_JEWELLERY_MASK = 0x0000000F,  // mask of flags for known jewellery

    ISFLAG_CURSED            = 0x00000100,  // cursed
    ISFLAG_RESERVED_1        = 0x00000200,  // reserved
    ISFLAG_RESERVED_2        = 0x00000400,  // reserved
    ISFLAG_RESERVED_3        = 0x00000800,  // reserved 
    ISFLAG_CURSE_MASK        = 0x00000F00,  // mask of all curse related flags

    ISFLAG_RANDART           = 0x00001000,  // special value is seed
    ISFLAG_UNRANDART         = 0x00002000,  // is an unrandart
    ISFLAG_ARTEFACT_MASK     = 0x00003000,  // randart or unrandart
    ISFLAG_DROPPED           = 0x00004000,  // dropped item (no autopickup)
    ISFLAG_THROWN            = 0x00008000,  // thrown missile weapon

    // these don't have to remain as flags
    ISFLAG_NO_DESC           = 0x00000000,  // used for clearing these flags
    ISFLAG_GLOWING           = 0x00010000,  // weapons or armour
    ISFLAG_RUNED             = 0x00020000,  // weapons or armour
    ISFLAG_EMBROIDERED_SHINY = 0x00040000,  // armour: depends on sub-type
    ISFLAG_COSMETIC_MASK     = 0x00070000,  // mask of cosmetic descriptions

    ISFLAG_NO_RACE           = 0x00000000,  // used for clearing these flags
    ISFLAG_ORCISH            = 0x01000000,  // low quality items
    ISFLAG_DWARVEN           = 0x02000000,  // strong and robust items
    ISFLAG_ELVEN             = 0x04000000,  // light and accurate items
    ISFLAG_RACIAL_MASK       = 0x07000000,  // mask of racial equipment types

    ISFLAG_NOTED_ID          = 0x08000000,
    ISFLAG_NOTED_GET         = 0x10000000,

    ISFLAG_BEEN_IN_INV       = 0x20000000, // Item has been in inventory
    ISFLAG_SUMMONED          = 0x40000000  // Item generated on a summon
};

enum job_type
{
    JOB_FIGHTER,                       //    0
    JOB_WIZARD,
    JOB_PRIEST,
    JOB_THIEF,
    JOB_GLADIATOR,
    JOB_NECROMANCER,                   //    5
    JOB_PALADIN,
    JOB_ASSASSIN,
    JOB_BERSERKER,
    JOB_HUNTER,
    JOB_CONJURER,                      //   10
    JOB_ENCHANTER,
    JOB_FIRE_ELEMENTALIST,
    JOB_ICE_ELEMENTALIST,
    JOB_SUMMONER,
    JOB_AIR_ELEMENTALIST,              //   15
    JOB_EARTH_ELEMENTALIST,
    JOB_CRUSADER,
    JOB_DEATH_KNIGHT,
    JOB_VENOM_MAGE,
    JOB_CHAOS_KNIGHT,                  //   20
    JOB_TRANSMUTER,
    JOB_HEALER,                        //   22
    JOB_QUITTER,                       //   23 -- this is job 'x', don't use
    JOB_REAVER,                        //   24
    JOB_STALKER,                       //   25
    JOB_MONK,
    JOB_WARPER,
    JOB_WANDERER,                      //   23
    NUM_JOBS,                          // always after the last job

    JOB_UNKNOWN = 100
};

// This order is *critical*. Don't mess with it (see mon_enchant)
enum kill_category
{
    KC_YOU,
    KC_FRIENDLY,
    KC_OTHER,
    KC_NCATEGORIES
};

enum killer_type                       // monster_die(), thing_thrown
{
    KILL_NONE = 0,
    KILL_YOU,                          //    1
    KILL_MON,
    KILL_YOU_MISSILE,
    KILL_MON_MISSILE,
    KILL_YOU_CONF,
    KILL_MISC,                         //    5
    KILL_RESET,                        // abjuration, etc.
    KILL_DISMISSED                     // only on new game startup
};

enum flight_type
{
    FL_NONE = 0,
    FL_FLY,
    FL_LEVITATE
};

enum level_area_type                   // you.level_type
{
    LEVEL_DUNGEON,                     //    0
    LEVEL_LABYRINTH,
    LEVEL_ABYSS,
    LEVEL_PANDEMONIUM,
    LEVEL_PORTAL_VAULT,

    NUM_LEVEL_AREA_TYPES
};

enum entry_cause_type
{
    EC_UNKNOWN,
    EC_SELF_EXPLICIT,
    EC_SELF_RISKY,    // i.e., wielding an id'd distorion weapon
    EC_SELF_ACCIDENT, // i.e., wielding an un-id'd distortion weapon
    EC_MISCAST,
    EC_GOD_RETRIUBTION,
    EC_GOD_ACT, // Xom sending the player somewhere for amusement
    EC_MONSTER,
    NUM_ENTRY_CAUSE_TYPES
};

// Can't change this order without breaking saves.
enum map_marker_type
{
    MAT_FEATURE,              // Stock marker.
    MAT_LUA_MARKER,
    MAT_CORRUPTION_NEXUS,
    MAT_WIZ_PROPS,
    NUM_MAP_MARKER_TYPES,
    MAT_ANY
};

enum menu_type
{
    MT_ANY = -1,
    
    MT_INVLIST,                        // List inventory
    MT_DROP,

    MT_PICKUP
};

enum mon_holy_type
{
    MH_HOLY,                           //    0 - was -1
    MH_NATURAL,                        //    1 - was 0
    MH_UNDEAD,                         //    2 - was 1
    MH_DEMONIC,                        //    3 - was 2
    MH_NONLIVING,                      //    golems and other constructs
    MH_PLANT                           //    plants
};

enum targ_mode_type
{
    TARG_ANY, 
    TARG_ENEMY,
    TARG_FRIEND,
    TARG_NUM_MODES
};

// note this order is very sensitive... look at mons_is_unique()
enum monster_type                      // (int) menv[].type
{
    MONS_GIANT_ANT,                    //    0
    MONS_GIANT_BAT,
    MONS_CENTAUR,
    MONS_RED_DEVIL,
    MONS_ETTIN,
    MONS_FUNGUS,                       //    5
    MONS_GOBLIN,
    MONS_HOUND,
    MONS_IMP,
    MONS_JACKAL,
    MONS_KILLER_BEE,                   //   10
    MONS_KILLER_BEE_LARVA,
    MONS_MANTICORE,
    MONS_NECROPHAGE,
    MONS_ORC,
    MONS_PHANTOM,                      //   15
    MONS_QUASIT,
    MONS_RAT,
    MONS_SCORPION,                     //   18
    //MONS_TUNNELING_WORM,      // deprecated and now officially removed {dlb}
    MONS_UGLY_THING = 20,              //   20
    MONS_FIRE_VORTEX,
    MONS_WORM,
    MONS_ABOMINATION_SMALL,
    MONS_YELLOW_WASP,
    MONS_ZOMBIE_SMALL,                 //   25
    MONS_ANGEL,
    MONS_GIANT_BEETLE,
    MONS_CYCLOPS,
    MONS_DRAGON,
    MONS_TWO_HEADED_OGRE,              //   30
    MONS_FIEND,
    MONS_GIANT_SPORE,
    MONS_HOBGOBLIN,
    MONS_ICE_BEAST,
    MONS_JELLY,                        //   35
    MONS_KOBOLD,
    MONS_LICH,
    MONS_MUMMY,
    MONS_GUARDIAN_NAGA,
    MONS_OGRE,                         //   40
    MONS_PLANT,
    MONS_QUEEN_BEE,
    MONS_RAKSHASA,
    MONS_SNAKE,
    MONS_TROLL,                        //   45
    MONS_UNSEEN_HORROR,
    MONS_VAMPIRE,
    MONS_WRAITH,
    MONS_ABOMINATION_LARGE,
    MONS_YAK,                          //   50
    MONS_ZOMBIE_LARGE,
    MONS_ORC_WARRIOR,
    MONS_KOBOLD_DEMONOLOGIST,
    MONS_ORC_WIZARD,
    MONS_ORC_KNIGHT,                   //   55
    //MONS_WORM_TAIL = 56, // deprecated and now officially removed {dlb}
    MONS_WYVERN = 57,                  //   57
    MONS_BIG_KOBOLD,
    MONS_GIANT_EYEBALL,
    MONS_WIGHT,                        //   60
    MONS_OKLOB_PLANT,
    MONS_WOLF_SPIDER,
    MONS_SHADOW,
    MONS_HUNGRY_GHOST,
    MONS_EYE_OF_DRAINING,              //   65
    MONS_BUTTERFLY,
    MONS_WANDERING_MUSHROOM,
    MONS_EFREET,
    MONS_BRAIN_WORM,
    MONS_GIANT_ORANGE_BRAIN,           //   70
    MONS_BOULDER_BEETLE,
    MONS_FLYING_SKULL,
    MONS_HELL_HOUND,
    MONS_MINOTAUR,
    MONS_ICE_DRAGON,                   //   75
    MONS_SLIME_CREATURE,
    MONS_FREEZING_WRAITH,
    MONS_RAKSHASA_FAKE,
    MONS_GREAT_ORB_OF_EYES,
    MONS_HELLION,                      //   80
    MONS_ROTTING_DEVIL,
    MONS_TORMENTOR,
    MONS_REAPER,
    MONS_SOUL_EATER,
    MONS_HAIRY_DEVIL,                  //   85
    MONS_ICE_DEVIL,
    MONS_BLUE_DEVIL,
    MONS_BEAST,
    MONS_IRON_DEVIL,                   //   89
    MONS_GLOWING_SHAPESHIFTER = 98,    //   98
    MONS_SHAPESHIFTER,
    MONS_GIANT_MITE,                   //  100
    MONS_STEAM_DRAGON,
    MONS_VERY_UGLY_THING,
    MONS_ORC_SORCERER,
    MONS_HIPPOGRIFF,
    MONS_GRIFFON,                      //  105
    MONS_HYDRA,
    MONS_SKELETON_SMALL,
    MONS_SKELETON_LARGE,
    MONS_HELL_KNIGHT,
    MONS_NECROMANCER,                  //  110
    MONS_WIZARD,
    MONS_ORC_PRIEST,
    MONS_ORC_HIGH_PRIEST,
    MONS_HUMAN,
    MONS_GNOLL,                        //  115
    MONS_CLAY_GOLEM,
    MONS_WOOD_GOLEM,
    MONS_STONE_GOLEM,
    MONS_IRON_GOLEM,
    MONS_CRYSTAL_GOLEM,                //  120
    MONS_TOENAIL_GOLEM,
    MONS_MOTTLED_DRAGON,
    MONS_EARTH_ELEMENTAL,
    MONS_FIRE_ELEMENTAL,
    MONS_AIR_ELEMENTAL,                //  125
    MONS_ICE_FIEND,
    MONS_SHADOW_FIEND,
    MONS_BROWN_SNAKE,
    MONS_GIANT_LIZARD,
    MONS_SPECTRAL_WARRIOR,             //  130
    MONS_PULSATING_LUMP,
    MONS_STORM_DRAGON,
    MONS_YAKTAUR,
    MONS_DEATH_YAK,
    MONS_ROCK_TROLL,                   //  135
    MONS_STONE_GIANT,
    MONS_FLAYED_GHOST,
    MONS_BUMBLEBEE,
    MONS_REDBACK,
    MONS_INSUBSTANTIAL_WISP,           //  140
    MONS_VAPOUR,
    MONS_OGRE_MAGE,
    MONS_SPINY_WORM,
    MONS_DANCING_WEAPON,
    MONS_TITAN,                        //  145
    MONS_GOLDEN_DRAGON,
    MONS_ELF,
    MONS_LINDWURM,
    MONS_ELEPHANT_SLUG,
    MONS_WAR_DOG,                      //  150
    MONS_GREY_RAT,
    MONS_GREEN_RAT,
    MONS_ORANGE_RAT,
    MONS_BLACK_SNAKE,
    MONS_SHEEP,                        //  155
    MONS_GHOUL,
    MONS_HOG,
    MONS_GIANT_MOSQUITO,
    MONS_GIANT_CENTIPEDE,
    MONS_IRON_TROLL,                   //  160
    MONS_NAGA,
    MONS_FIRE_GIANT,
    MONS_FROST_GIANT,
    MONS_FIREDRAKE,
    MONS_SHADOW_DRAGON,                //  165
    MONS_YELLOW_SNAKE,
    MONS_GREY_SNAKE,
    MONS_DEEP_TROLL,
    MONS_GIANT_BLOWFLY,
    MONS_RED_WASP,                     //  170
    MONS_SWAMP_DRAGON,
    MONS_SWAMP_DRAKE,
    MONS_DEATH_DRAKE,
    MONS_SOLDIER_ANT,
    MONS_HILL_GIANT,
    MONS_QUEEN_ANT,                    //  175
    MONS_ANT_LARVA,
    MONS_GIANT_FROG,
    MONS_GIANT_BROWN_FROG,
    MONS_SPINY_FROG,
    MONS_BLINK_FROG,                   //  180
    MONS_GIANT_COCKROACH,
    MONS_SMALL_SNAKE,                  //  182
    //jmf: new monsters
    MONS_SHUGGOTH, //jmf: added for evil spells
    MONS_WOLF,     //jmf: added
    MONS_WARG,     //jmf: added for orc mines
    MONS_BEAR,     //jmf: added bears!
    MONS_GRIZZLY_BEAR,
    MONS_POLAR_BEAR,
    MONS_BLACK_BEAR,  // 189
    MONS_SIMULACRUM_SMALL,
    MONS_SIMULACRUM_LARGE,
    MONS_MERFOLK,
    MONS_MERMAID,     // 193
    //jmf: end new monsters
    MONS_WHITE_IMP = 220,              //  220
    MONS_LEMURE,
    MONS_UFETUBUS,
    MONS_MANES,
    MONS_MIDGE,
    MONS_NEQOXEC,                      //  225
    MONS_ORANGE_DEMON,
    MONS_HELLWING,
    MONS_SMOKE_DEMON,
    MONS_YNOXINUL,
    MONS_EXECUTIONER,                  //  230
    MONS_GREEN_DEATH,
    MONS_BLUE_DEATH,
    MONS_BALRUG,
    MONS_CACODEMON,
    MONS_DEMONIC_CRAWLER,              //  235
    MONS_SUN_DEMON,
    MONS_SHADOW_IMP,
    MONS_SHADOW_DEMON,
    MONS_LOROCYPROCA,
    MONS_SHADOW_WRAITH,                //  240
    MONS_GIANT_AMOEBA,
    MONS_GIANT_SLUG,
    MONS_GIANT_SNAIL,
    MONS_SPATIAL_VORTEX,
    MONS_PIT_FIEND,                    //  245
    MONS_BORING_BEETLE,
    MONS_GARGOYLE,
    MONS_METAL_GARGOYLE,
    MONS_MOLTEN_GARGOYLE,
    MONS_PROGRAM_BUG,                  //  250
// BCR - begin first batch of uniques.
    MONS_MNOLEG,
    MONS_LOM_LOBON,
    MONS_CEREBOV,
    MONS_GLOORX_VLOQ,                  //  254
    MONS_MOLLUSC_LORD, //  255 - deprecated, but still referenced in code {dlb}
// BCR - End first batch of uniques.
    MONS_NAGA_MAGE = 260,              //  260
    MONS_NAGA_WARRIOR,
    MONS_ORC_WARLORD,
    MONS_DEEP_ELF_SOLDIER,
    MONS_DEEP_ELF_FIGHTER,
    MONS_DEEP_ELF_KNIGHT,              //  265
    MONS_DEEP_ELF_MAGE,
    MONS_DEEP_ELF_SUMMONER,
    MONS_DEEP_ELF_CONJURER,
    MONS_DEEP_ELF_PRIEST,
    MONS_DEEP_ELF_HIGH_PRIEST,         //  270
    MONS_DEEP_ELF_DEMONOLOGIST,
    MONS_DEEP_ELF_ANNIHILATOR,
    MONS_DEEP_ELF_SORCERER,
    MONS_DEEP_ELF_DEATH_MAGE,
    MONS_BROWN_OOZE,                   //  275
    MONS_AZURE_JELLY,
    MONS_DEATH_OOZE,
    MONS_ACID_BLOB,
    MONS_ROYAL_JELLY,
// BCR - begin second batch of uniques.
    MONS_TERENCE,                      //  280
    MONS_JESSICA,
    MONS_IJYB,
    MONS_SIGMUND,
    MONS_BLORK_THE_ORC,
    MONS_EDMUND,                       //  285
    MONS_PSYCHE,
    MONS_EROLCHA,
    MONS_DONALD,
    MONS_URUG,
    MONS_MICHAEL,                      //  290
    MONS_JOSEPH,
    MONS_SNORG, // was Anita - Snorg is correct 16jan2000 {dlb}
    MONS_ERICA,
    MONS_JOSEPHINE,
    MONS_HAROLD,                       //  295
    MONS_NORBERT,
    MONS_JOZEF,
    MONS_AGNES,
    MONS_MAUD,
    MONS_LOUISE,                       //  300
    MONS_FRANCIS,
    MONS_FRANCES,
    MONS_RUPERT,
    MONS_WAYNE,
    MONS_DUANE,                        //  305
    MONS_XTAHUA,
    MONS_NORRIS,
    MONS_FREDERICK,
    MONS_MARGERY,
    MONS_POLYPHEMUS,                   //  310
    MONS_BORIS,
// BCR - end second batch of uniques.

    MONS_DRACONIAN,

    // If adding more drac colours, sync up colour names in
    // mon-util.cc.
    MONS_BLACK_DRACONIAN,               // Should always be first colour.
    MONS_MOTTLED_DRACONIAN,
    MONS_YELLOW_DRACONIAN,
    MONS_GREEN_DRACONIAN,               // 315
    MONS_PURPLE_DRACONIAN,
    MONS_RED_DRACONIAN,
    MONS_WHITE_DRACONIAN,
    MONS_PALE_DRACONIAN,                // Should always be last colour.

    // Sync up with monplace.cc's draconian selection if adding more.
    MONS_DRACONIAN_CALLER,
    MONS_DRACONIAN_MONK, 
    MONS_DRACONIAN_ZEALOT,
    MONS_DRACONIAN_SHIFTER,
    MONS_DRACONIAN_ANNIHILATOR,
    MONS_DRACONIAN_KNIGHT,              // 325
    MONS_DRACONIAN_SCORCHER,
    
    MONS_MURRAY,
    MONS_TIAMAT,

    MONS_DEEP_ELF_BLADEMASTER,
    MONS_DEEP_ELF_MASTER_ARCHER,

    // The Lords of Hell:
    MONS_GERYON = 340,                 //  340
    MONS_DISPATER,
    MONS_ASMODEUS,
    MONS_ANTAEUS,
    MONS_ERESHKIGAL,                   //  344

    MONS_ANCIENT_LICH = 356,           //  356
    MONS_OOZE,                         //  357

    MONS_VAULT_GUARD = 360,            //  360
    MONS_CURSE_SKULL,
    MONS_VAMPIRE_KNIGHT,
    MONS_VAMPIRE_MAGE,
    MONS_SHINING_EYE,
    MONS_ORB_GUARDIAN,                 //  365
    MONS_DAEVA,
    MONS_SPECTRAL_THING,
    MONS_GREATER_NAGA,
    MONS_SKELETAL_DRAGON,
    MONS_TENTACLED_MONSTROSITY,        //  370
    MONS_SPHINX,
    MONS_ROTTING_HULK,
    MONS_GUARDIAN_MUMMY,
    MONS_GREATER_MUMMY,
    MONS_MUMMY_PRIEST,                 //  375
    MONS_CENTAUR_WARRIOR,
    MONS_YAKTAUR_CAPTAIN,
    MONS_KILLER_KLOWN,
    MONS_ELECTRIC_GOLEM, // replacing the guardian robot -- bwr
    MONS_BALL_LIGHTNING, // replacing the dorgi -- bwr
    MONS_ORB_OF_FIRE,    // Swords renamed to fit -- bwr
    MONS_QUOKKA,         // Quokka are a type of wallaby, returned -- bwr 382
    

    MONS_EYE_OF_DEVASTATION = 385,     //  385
    MONS_MOTH_OF_WRATH,
    MONS_DEATH_COB,
    MONS_CURSE_TOE,
    MONS_GOLD_MIMIC,
    MONS_WEAPON_MIMIC,                 //  390
    MONS_ARMOUR_MIMIC,
    MONS_SCROLL_MIMIC,
    MONS_POTION_MIMIC,
    MONS_HELL_HOG,
    MONS_SERPENT_OF_HELL,              //  395
    MONS_BOGGART,
    MONS_QUICKSILVER_DRAGON,
    MONS_IRON_DRAGON,
    MONS_SKELETAL_WARRIOR,             //  399
    MONS_PLAYER_GHOST,                 //  400
    MONS_PANDEMONIUM_DEMON,            //  401

    MONS_GIANT_NEWT,                   //  402
    MONS_GIANT_GECKO,                  //  403
    MONS_GIANT_IGUANA,                 //  404
    MONS_GILA_MONSTER,                 //  405
    MONS_KOMODO_DRAGON,                //  406

    // Lava monsters:
    MONS_LAVA_WORM = 420,              //  420
    MONS_LAVA_FISH,
    MONS_LAVA_SNAKE,
    MONS_SALAMANDER,                   //  423 mv: was another lava thing

    // Water monsters:
    MONS_BIG_FISH = 430,               //  430
    MONS_GIANT_GOLDFISH,
    MONS_ELECTRICAL_EEL,
    MONS_JELLYFISH,
    MONS_WATER_ELEMENTAL,
    MONS_SWAMP_WORM,                   //  435

    // Monsters which move through rock:
    MONS_ROCK_WORM = 440,

    // Statuary
    MONS_ORANGE_STATUE,
    MONS_SILVER_STATUE,
    MONS_ICE_STATUE,

    NUM_MONSTERS,                      // used for polymorph
    RANDOM_MONSTER = 1000, // used to distinguish between a random monster and using program bugs for error trapping {dlb}

    // A random draconian, either base coloured drac or specialised.
    RANDOM_DRACONIAN,
    
    // Any random base draconian colour.
    RANDOM_BASE_DRACONIAN,

    // Any random specialised draconian, such as a draconian knight.
    RANDOM_NONBASE_DRACONIAN,
    
    WANDERING_MONSTER = 2500 // only used in monster placement routines - forced limit checks {dlb}

};

enum beh_type
{
    BEH_SLEEP,                         //    0
    BEH_WANDER,
    BEH_SEEK,
    BEH_FLEE,
    BEH_CORNERED,
    BEH_PANIC,                         //  like flee but without running away
    BEH_INVESTIGATE,                   //  investigating an ME_DISTURB
    NUM_BEHAVIOURS,                    //  max # of legal states
    BEH_CHARMED,                       //  hostile-but-charmed; create only
    BEH_FRIENDLY,                      //  used during creation only
    BEH_NEUTRAL,                       //  creation only
    BEH_HOSTILE,                       //  creation only
    BEH_GOD_GIFT,                      //  creation only
    BEH_GUARD                          //  creation only - monster is guard
};

enum mon_attitude_type
{
    ATT_HOSTILE,                       // 0, default in most cases
    ATT_FRIENDLY,                      // created friendly (or tamed?)
    ATT_NEUTRAL
};

enum mon_flight_type
{
    FLY_NOT,
    FLY_POWERED,                        // wings, etc... paralysis == fall
    FLY_LEVITATION                      // doesn't require physical effort
};

// These are now saved in an unsigned long in the monsters struct.
enum monster_flag_type
{
    MF_CREATED_FRIENDLY   = 0x01,   // no benefit from killing
    MF_GOD_GIFT           = 0x02,   // player not penalized by its death
    MF_BATTY              = 0x04,   // flutters like a bat
    MF_JUST_SUMMONED      = 0x08,   // monster skips next available action
    MF_TAKING_STAIRS      = 0x10,   // is following player through stairs

    MF_INTERESTING        = 0x20,   // Player finds monster interesting
    MF_SEEN               = 0x40,   // Player already seen monster
    MF_DIVINE_PROTECTION  = 0x80,   // Monster has divine protection.
    
    MF_KNOWN_MIMIC        = 0x100,  // Mimic that has taken a swing at the PC,
                                    // or that the player has inspected with ?
    MF_BANISHED           = 0x200,  // Monster that has been banished.
    MF_HARD_RESET         = 0x400,  // Summoned, should not drop gear on reset
    MF_CONVERT_ATTEMPT    = 0x800,  // Orcs only: seen player and was converted
                                    // (or not)
    MF_WAS_IN_VIEW        = 0x1000, // Was in view during previous turn
    MF_BAND_MEMBER        = 0x2000  // Created as a member of a band
};

// Adding slots breaks saves. YHBW.
enum mon_inv_type           // (int) menv[].inv[]
{
    MSLOT_WEAPON,           // Primary weapon (melee)
    MSLOT_ALT_WEAPON,       // Alternate weapon, ranged or second melee weapon
                            // for monsters that can use two weapons.
    MSLOT_MISSILE,
    MSLOT_ARMOUR,
    MSLOT_SHIELD,
    MSLOT_MISCELLANY,
    MSLOT_POTION,
    MSLOT_WAND,
    MSLOT_SCROLL,
    MSLOT_GOLD,
    NUM_MONSTER_SLOTS
};

// XXX: These still need to be applied in mon-data.h
enum mon_spellbook_type
{
    MST_ORC_WIZARD_I     = 0,
    MST_ORC_WIZARD_II,
    MST_ORC_WIZARD_III,
    MST_GUARDIAN_NAGA    = 10,
    MST_LICH_I           = 20,
    MST_LICH_II,
    MST_LICH_III,
    MST_LICH_IV,
    MST_BURNING_DEVIL    = 30,
    MST_VAMPIRE          = 40,
    MST_VAMPIRE_KNIGHT,
    MST_VAMPIRE_MAGE,
    MST_EFREET           = 50,
    MST_KILLER_KLOWN,
    MST_BRAIN_WORM,
    MST_GIANT_ORANGE_BRAIN,
    MST_RAKSHASA,
    MST_GREAT_ORB_OF_EYES,              // 55
    MST_ORC_SORCERER,
    MST_STEAM_DRAGON,
    MST_HELL_KNIGHT_I,
    MST_HELL_KNIGHT_II,
    MST_NECROMANCER_I,                  // 60
    MST_NECROMANCER_II,
    MST_WIZARD_I,
    MST_WIZARD_II,
    MST_WIZARD_III,
    MST_WIZARD_IV,                      // 65
    MST_WIZARD_V,
    MST_ORC_PRIEST,
    MST_ORC_HIGH_PRIEST,
    MST_MOTTLED_DRAGON,
    MST_ICE_FIEND,                      // 70
    MST_SHADOW_FIEND,
    MST_TORMENTOR,
    MST_STORM_DRAGON,
    MST_WHITE_IMP,
    MST_YNOXINUL,                       // 75
    MST_NEQOXEC,
    MST_HELLWING,
    MST_SMOKE_DEMON,
    MST_CACODEMON,
    MST_GREEN_DEATH,                    // 80
    MST_BALRUG,
    MST_BLUE_DEATH,
    MST_GERYON,
    MST_DISPATER,
    MST_ASMODEUS,                       // 85
    MST_ERESHKIGAL,
    MST_ANTAEUS,                        // 87
    MST_MNOLEG                = 90,
    MST_LOM_LOBON,
    MST_CEREBOV,
    MST_GLOORX_VLOQ,
    MST_TITAN,
    MST_GOLDEN_DRAGON,                  // 95
    MST_DEEP_ELF_SUMMONER,
    MST_DEEP_ELF_CONJURER_I,
    MST_DEEP_ELF_CONJURER_II,
    MST_DEEP_ELF_PRIEST,
    MST_DEEP_ELF_HIGH_PRIEST,           // 100
    MST_DEEP_ELF_DEMONOLOGIST,
    MST_DEEP_ELF_ANNIHILATOR,
    MST_DEEP_ELF_SORCERER,
    MST_DEEP_ELF_DEATH_MAGE,
    MST_KOBOLD_DEMONOLOGIST,            // 105
    MST_NAGA,
    MST_NAGA_MAGE,
    MST_CURSE_SKULL,
    MST_SHINING_EYE,
    MST_FROST_GIANT,                    // 110
    MST_ANGEL,
    MST_DAEVA,
    MST_SHADOW_DRAGON,
    MST_SPHINX,
    MST_MUMMY,                          // 115
    MST_ELECTRIC_GOLEM,
    MST_ORB_OF_FIRE,
    MST_SHADOW_IMP,
    MST_GHOST,
    MST_HELL_HOG,                       // 120
    MST_SWAMP_DRAGON,
    MST_SWAMP_DRAKE,
    MST_SERPENT_OF_HELL,
    MST_BOGGART,
    MST_EYE_OF_DEVASTATION,             // 125
    MST_QUICKSILVER_DRAGON,
    MST_IRON_DRAGON,
    MST_SKELETAL_WARRIOR,
    MST_MYSTIC,
    MST_DEATH_DRAKE,                    // 130
    MST_DRAC_SCORCHER, // As Bioster would say.. pig*s
    MST_DRAC_CALLER,
    MST_DRAC_SHIFTER,
    MST_CURSE_TOE,
    MST_RUPERT,
    MST_ICE_STATUE,
    NUM_MSTYPES,
    MST_NO_SPELLS = 250
};

enum mutation_type
{
    MUT_TOUGH_SKIN,                    //    0
    MUT_STRONG,
    MUT_CLEVER,
    MUT_AGILE,
    MUT_GREEN_SCALES,
    MUT_BLACK_SCALES,                  //    5
    MUT_GREY_SCALES,
    MUT_BONEY_PLATES,
    MUT_REPULSION_FIELD,
    MUT_POISON_RESISTANCE,
    MUT_CARNIVOROUS,                   //   10
    MUT_HERBIVOROUS,
    MUT_HEAT_RESISTANCE,
    MUT_COLD_RESISTANCE,
    MUT_SHOCK_RESISTANCE,
    MUT_REGENERATION,                  //   15
    MUT_FAST_METABOLISM,
    MUT_SLOW_METABOLISM,
    MUT_WEAK,
    MUT_DOPEY,
    MUT_CLUMSY,                        //   20
    MUT_TELEPORT_CONTROL,
    MUT_TELEPORT,
    MUT_MAGIC_RESISTANCE,
    MUT_FAST,
    MUT_ACUTE_VISION,                  //   25
    MUT_DEFORMED,
    MUT_TELEPORT_AT_WILL,
    MUT_SPIT_POISON,
    MUT_MAPPING,
    MUT_BREATHE_FLAMES,                //   30
    MUT_BLINK,
    MUT_HORNS,
    MUT_STRONG_STIFF,
    MUT_FLEXIBLE_WEAK,
    MUT_SCREAM,                        //   35
    MUT_CLARITY,
    MUT_BERSERK,
    MUT_DETERIORATION,
    MUT_BLURRY_VISION,
    MUT_MUTATION_RESISTANCE,           //   40
    MUT_FRAIL,
    MUT_ROBUST,
    MUT_TORMENT_RESISTANCE,
    MUT_NEGATIVE_ENERGY_RESISTANCE,
    MUT_SUMMON_MINOR_DEMONS,           //   45
    MUT_SUMMON_DEMONS,
    MUT_HURL_HELLFIRE,
    MUT_CALL_TORMENT,
    MUT_RAISE_DEAD,
    MUT_CONTROL_DEMONS,                //   50
    MUT_PANDEMONIUM,
    MUT_DEATH_STRENGTH,
    MUT_CHANNEL_HELL,
    MUT_DRAIN_LIFE,
    MUT_THROW_FLAMES,                  //   55
    MUT_THROW_FROST,
    MUT_SMITE,
    MUT_CLAWS,                         
    MUT_FANGS,      // new in 0.3
    // hooves, talons, paws can replace feet
    MUT_HOOVES,                        //   60
    MUT_TALONS,     // new in 0.4
    MUT_PAWS,       // new in 0.4
    MUT_BREATHE_POISON,
    MUT_STINGER,
    MUT_BIG_WINGS,                     //   65
    MUT_BLUE_MARKS, // decorative, as in "mark of the devil"
    MUT_GREEN_MARKS,
    MUT_SAPROVOROUS,
    MUT_SHAGGY_FUR, // new in 0.4
    MUT_HIGH_MAGIC, // new in 0.4      --   70
    MUT_LOW_MAGIC,  // new in 0.4
    MUT_SLEEPINESS, // new in 0.4
    
    // several types of scales (affect AC and sometimes more)
    MUT_RED_SCALES = 75,               //   75
    MUT_NACREOUS_SCALES,
    MUT_GREY2_SCALES,
    MUT_METALLIC_SCALES,
    MUT_BLACK2_SCALES,
    MUT_WHITE_SCALES,                  //   80
    MUT_YELLOW_SCALES,
    MUT_BROWN_SCALES,
    MUT_BLUE_SCALES,
    MUT_PURPLE_SCALES,
    MUT_SPECKLED_SCALES,               //   85
    MUT_ORANGE_SCALES,
    MUT_INDIGO_SCALES,
    MUT_RED2_SCALES,
    MUT_IRIDESCENT_SCALES,
    MUT_PATTERNED_SCALES,              //   90
    NUM_MUTATIONS,

    RANDOM_MUTATION = 100,
    RANDOM_XOM_MUTATION = 101,
    RANDOM_GOOD_MUTATION = 102
};

enum object_class_type                 // (unsigned char) mitm[].base_type
{
    OBJ_WEAPONS,                       //    0
    OBJ_MISSILES,
    OBJ_ARMOUR,
    OBJ_WANDS,
    OBJ_FOOD,                          //    4
    OBJ_UNKNOWN_I = 5, // (use unknown) labeled as books in invent.cc {dlb}
    OBJ_SCROLLS = 6,                   //    6
    OBJ_JEWELLERY,
    OBJ_POTIONS,                       //    8
    OBJ_UNKNOWN_II = 9, // (use unknown, stackable) labeled as gems in invent.cc {dlb}
    OBJ_BOOKS = 10,                    //   10
    OBJ_STAVES,
    OBJ_ORBS,
    OBJ_MISCELLANY,
    OBJ_CORPSES,
    OBJ_GOLD, // important role as upper limit to chardump::dump_inventory() {dlb}
    OBJ_GEMSTONES, // found in itemname.cc, labeled as miscellaneous in invent.cc {dlb}
    NUM_OBJECT_CLASSES,
    OBJ_UNASSIGNED = 100,              // must remain set to 100 {dlb}
    OBJ_RANDOM = 255 // must remain set to 255 {dlb} - also used
                     // for blanket random sub_type .. see dungeon::items()
};

enum operation_types
{
    OPER_WIELD = 'w',
    OPER_QUAFF = 'q',
    OPER_DROP = 'd',
    OPER_EAT = 'e',
    OPER_TAKEOFF = 'T',
    OPER_WEAR = 'W',
    OPER_PUTON = 'P',
    OPER_REMOVE = 'R',
    OPER_READ = 'r',
    OPER_MEMORISE = 'M',
    OPER_ZAP = 'z',
    OPER_THROW = 't',
    OPER_EXAMINE = 'v',
    OPER_FIRE = 'f',
    OPER_PRAY = 'p',
    OPER_EVOKE = 'E',
    OPER_ANY = 0
};

enum orb_type
{
    ORB_ZOT                            //    0
};

enum player_size_type
{
    PSIZE_BODY,         // entire body size -- used for EV/size of target
    PSIZE_TORSO,        // torso only (hybrids -- size of parts that use equip)
    PSIZE_PROFILE       // profile only (for stealth checks)
};

enum pronoun_type
{
    PRONOUN_CAP,                        // 0
    PRONOUN_NOCAP,                      // 1
    PRONOUN_CAP_POSSESSIVE,             // 2
    PRONOUN_NOCAP_POSSESSIVE,           // 3
    PRONOUN_REFLEXIVE                   // 4 (reflexive is always lowercase)
};

enum randart_prop_type
{
    RAP_BRAND,                         //    0
    RAP_AC,
    RAP_EVASION,
    RAP_STRENGTH,
    RAP_INTELLIGENCE,
    RAP_DEXTERITY,                     //    5
    RAP_FIRE,
    RAP_COLD,
    RAP_ELECTRICITY,
    RAP_POISON,
    RAP_NEGATIVE_ENERGY,               //   10
    RAP_MAGIC,
    RAP_EYESIGHT,
    RAP_INVISIBLE,
    RAP_LEVITATE,
    RAP_BLINK,                         //   15
    RAP_CAN_TELEPORT,
    RAP_BERSERK,
    RAP_MAPPING,
    RAP_NOISES,
    RAP_PREVENT_SPELLCASTING,          //   20
    RAP_CAUSE_TELEPORTATION,
    RAP_PREVENT_TELEPORTATION,
    RAP_ANGRY,
    RAP_METABOLISM,
    RAP_MUTAGENIC,                     //   25
    RAP_ACCURACY,
    RAP_DAMAGE,
    RAP_CURSED,
    RAP_STEALTH,
    RAP_MAGICAL_POWER,                 //   30
    RAP_NUM_PROPERTIES
};

enum score_format_type
{
    SCORE_TERSE,                // one line
    SCORE_REGULAR,              // two lines (name, cause, blank)
    SCORE_VERBOSE               // everything (dates, times, god, etc)
};

enum shop_type // (unsigned char) env.sh_type[], item_in_shop(), in_a_shop()
{
    SHOP_WEAPON,                       //    0
    SHOP_ARMOUR,
    SHOP_WEAPON_ANTIQUE,
    SHOP_ARMOUR_ANTIQUE,
    SHOP_GENERAL_ANTIQUE,
    SHOP_JEWELLERY,                    //    5
    SHOP_WAND,
    SHOP_BOOK,
    SHOP_FOOD,
    SHOP_DISTILLERY,
    SHOP_SCROLL,                       //   10
    SHOP_GENERAL,
    NUM_SHOPS, // must remain last 'regular' member {dlb}
    SHOP_UNASSIGNED = 100,             // keep set at 100 for now {dlb}
    SHOP_RANDOM = 255                  // keep set at 255 for now {dlb}
};

// These are often addressed relative to each other (esp. delta SIZE_MEDIUM)
enum size_type
{
    SIZE_TINY,              // rat/bat
    SIZE_LITTLE,            // spriggan
    SIZE_SMALL,             // halfling/kobold/gnome
    SIZE_MEDIUM,            // human/elf/dwarf
    SIZE_LARGE,             // troll/ogre/centaur/naga
    SIZE_BIG,               // large quadrupeds
    SIZE_GIANT,             // giant 
    SIZE_HUGE,              // dragon
    NUM_SIZE_LEVELS,
    SIZE_CHARACTER          // transformations that don't change size
};

// [dshaligram] If you add a new skill, update skills2.cc, specifically
// the skills[] array and skill_display_order[]. New skills must go at the
// end of the list or in the unused skill numbers. NEVER rearrange this enum or
// move existing skills to new numbers; save file compatibility depends on this
// order.
enum skill_type
{
    SK_FIGHTING,                       //    0
    SK_SHORT_BLADES,
    SK_LONG_BLADES,
    SK_UNUSED_1,                       // SK_GREAT_SWORDS - now unused
    SK_AXES,
    SK_MACES_FLAILS,                   //    5
    SK_POLEARMS,
    SK_STAVES,
    SK_SLINGS,
    SK_BOWS,
    SK_CROSSBOWS,                      //   10
    SK_DARTS,
    SK_THROWING,
    SK_ARMOUR,
    SK_DODGING,
    SK_STEALTH,                        //   15
    SK_STABBING,
    SK_SHIELDS,
    SK_TRAPS_DOORS,
    SK_UNARMED_COMBAT,                 //   19
    SK_SPELLCASTING = 25,              //   25
    SK_CONJURATIONS,
    SK_ENCHANTMENTS,
    SK_SUMMONINGS,
    SK_NECROMANCY,
    SK_TRANSLOCATIONS,                 //   30
    SK_TRANSMIGRATION,
    SK_DIVINATIONS,
    SK_FIRE_MAGIC,
    SK_ICE_MAGIC,
    SK_AIR_MAGIC,                      //   35
    SK_EARTH_MAGIC,
    SK_POISON_MAGIC,
    SK_INVOCATIONS,
    SK_EVOCATIONS,
    NUM_SKILLS,                        // must remain last regular member

    SK_BLANK_LINE,                     // used for skill output
    SK_COLUMN_BREAK,                   // used for skill output
    SK_NONE
};

// order is important on these (see player_speed())
enum speed_type
{
    SPEED_SLOWED,
    SPEED_NORMAL,
    SPEED_HASTED
};

enum species_type
{
    SP_HUMAN = 1,                      //    1
    SP_HIGH_ELF,
    SP_GREY_ELF,
    SP_DEEP_ELF,                       //    5
    SP_SLUDGE_ELF,
    SP_MOUNTAIN_DWARF,
    SP_HALFLING,
    SP_HILL_ORC,                       //   10
    SP_KOBOLD,
    SP_MUMMY,
    SP_NAGA,
    SP_GNOME,
    SP_OGRE,                           //   15
    SP_TROLL,
    SP_OGRE_MAGE,
    SP_RED_DRACONIAN,
    SP_WHITE_DRACONIAN,
    SP_GREEN_DRACONIAN,                //   20
    SP_GOLDEN_DRACONIAN,
    SP_GREY_DRACONIAN,
    SP_BLACK_DRACONIAN,
    SP_PURPLE_DRACONIAN,
    SP_MOTTLED_DRACONIAN,              //   25
    SP_PALE_DRACONIAN,
    SP_UNK0_DRACONIAN,
    SP_UNK1_DRACONIAN,
    SP_BASE_DRACONIAN,
    SP_CENTAUR,                        //   30
    SP_DEMIGOD,
    SP_SPRIGGAN,
    SP_MINOTAUR,
    SP_DEMONSPAWN,
    SP_GHOUL,                          //   35
    SP_KENKU,
    SP_MERFOLK,
    SP_VAMPIRE,
    SP_ELF,                            // (placeholder)
    SP_HILL_DWARF,                     // (placeholder)
    NUM_SPECIES,                       // always after the last species

    SP_UNKNOWN  = 100
};

enum spell_type
{
    SPELL_IDENTIFY,                    //    0
    SPELL_TELEPORT_SELF,
    SPELL_CAUSE_FEAR,
    SPELL_CREATE_NOISE,
    SPELL_REMOVE_CURSE,
    SPELL_MAGIC_DART,                  //    5
    SPELL_FIREBALL,
    SPELL_SWAP,
    SPELL_APPORTATION,
    SPELL_TWIST,
    SPELL_FAR_STRIKE,                  //   10
    SPELL_DELAYED_FIREBALL,
    SPELL_STRIKING,
    SPELL_CONJURE_FLAME,
    SPELL_DIG,
    SPELL_BOLT_OF_FIRE,                //   15
    SPELL_BOLT_OF_COLD,
    SPELL_LIGHTNING_BOLT,
    SPELL_BOLT_OF_MAGMA,               //   18
    SPELL_POLYMORPH_OTHER = 20,        //   20
    SPELL_SLOW,
    SPELL_HASTE,
    SPELL_PARALYSE,
    SPELL_CONFUSE,
    SPELL_INVISIBILITY,                //   25
    SPELL_THROW_FLAME,
    SPELL_THROW_FROST,
    SPELL_CONTROLLED_BLINK,
    SPELL_FREEZING_CLOUD,
    SPELL_MEPHITIC_CLOUD,              //   30
    SPELL_RING_OF_FLAMES,
    SPELL_RESTORE_STRENGTH,
    SPELL_RESTORE_INTELLIGENCE,
    SPELL_RESTORE_DEXTERITY,
    SPELL_VENOM_BOLT,                  //   35
    SPELL_OLGREBS_TOXIC_RADIANCE,
    SPELL_TELEPORT_OTHER,
    SPELL_LESSER_HEALING,
    SPELL_GREATER_HEALING,
    SPELL_CURE_POISON_I,               //   40
    SPELL_PURIFICATION,
    SPELL_DEATHS_DOOR,
    SPELL_SELECTIVE_AMNESIA,
    SPELL_MASS_CONFUSION,
    SPELL_SMITING,                     //   45
    SPELL_REPEL_UNDEAD,
    SPELL_HOLY_WORD,
    SPELL_DETECT_CURSE,
    SPELL_SUMMON_SMALL_MAMMAL,
    SPELL_ABJURATION_I,                //   50
    SPELL_SUMMON_SCORPIONS,
    SPELL_LEVITATION,
    SPELL_BOLT_OF_DRAINING,
    SPELL_LEHUDIBS_CRYSTAL_SPEAR,
    SPELL_BOLT_OF_INACCURACY,          //   55
    SPELL_POISONOUS_CLOUD,
    SPELL_FIRE_STORM,
    SPELL_DETECT_TRAPS,
    SPELL_BLINK,
    SPELL_ISKENDERUNS_MYSTIC_BLAST,    //   60
    SPELL_SWARM,
    SPELL_SUMMON_HORRIBLE_THINGS,
    SPELL_ENSLAVEMENT,
    SPELL_MAGIC_MAPPING,
    SPELL_HEAL_OTHER,                  //   65
    SPELL_ANIMATE_DEAD,
    SPELL_PAIN,
    SPELL_EXTENSION,
    SPELL_CONTROL_UNDEAD,
    SPELL_ANIMATE_SKELETON,            //   70
    SPELL_VAMPIRIC_DRAINING,
    SPELL_SUMMON_WRAITHS,
    SPELL_DETECT_ITEMS,
    SPELL_BORGNJORS_REVIVIFICATION,
    SPELL_BURN,                        //   75
    SPELL_FREEZE,
    SPELL_SUMMON_ELEMENTAL,
    SPELL_OZOCUBUS_REFRIGERATION,
    SPELL_STICKY_FLAME,
    SPELL_SUMMON_ICE_BEAST,            //   80
    SPELL_OZOCUBUS_ARMOUR,
    SPELL_CALL_IMP,
    SPELL_REPEL_MISSILES,
    SPELL_BERSERKER_RAGE,
    SPELL_DISPEL_UNDEAD,               //   85
    SPELL_GUARDIAN,
    SPELL_PESTILENCE,
    SPELL_THUNDERBOLT,
    SPELL_FLAME_OF_CLEANSING,
    SPELL_SHINING_LIGHT,               //   90
    SPELL_SUMMON_DAEVA,
    SPELL_ABJURATION_II,
    SPELL_FULSOME_DISTILLATION,        //   93
    SPELL_POISON_ARROW,                //   94
    SPELL_TWISTED_RESURRECTION = 110,  //  110
    SPELL_REGENERATION,
    SPELL_BONE_SHARDS,
    SPELL_BANISHMENT,
    SPELL_CIGOTUVIS_DEGENERATION,
    SPELL_STING,                       //  115
    SPELL_SUBLIMATION_OF_BLOOD,
    SPELL_TUKIMAS_DANCE,
    SPELL_HELLFIRE,
    SPELL_SUMMON_DEMON,
    SPELL_DEMONIC_HORDE,               //  120
    SPELL_SUMMON_GREATER_DEMON,
    SPELL_CORPSE_ROT,
    SPELL_TUKIMAS_VORPAL_BLADE,
    SPELL_FIRE_BRAND,
    SPELL_FREEZING_AURA,               //  125
    SPELL_LETHAL_INFUSION,
    SPELL_CRUSH,
    SPELL_BOLT_OF_IRON,
    SPELL_STONE_ARROW,
    SPELL_TOMB_OF_DOROKLOHE,           //  130
    SPELL_STONEMAIL,
    SPELL_SHOCK,
    SPELL_SWIFTNESS,
    SPELL_FLY,
    SPELL_INSULATION,                  //  135
    SPELL_ORB_OF_ELECTROCUTION,
    SPELL_DETECT_CREATURES,
    SPELL_CURE_POISON_II,
    SPELL_CONTROL_TELEPORT,
    SPELL_POISON_AMMUNITION,           //  140
    SPELL_POISON_WEAPON,
    SPELL_RESIST_POISON,
    SPELL_PROJECTED_NOISE,
    SPELL_ALTER_SELF,
    SPELL_DEBUGGING_RAY,               //  145
    SPELL_RECALL,
    SPELL_PORTAL,
    SPELL_AGONY,
    SPELL_SPIDER_FORM,
    SPELL_DISRUPT,                     //  150
    SPELL_DISINTEGRATE,
    SPELL_BLADE_HANDS,
    SPELL_STATUE_FORM,
    SPELL_ICE_FORM,
    SPELL_DRAGON_FORM,                 //  155
    SPELL_NECROMUTATION,
    SPELL_DEATH_CHANNEL,
    SPELL_SYMBOL_OF_TORMENT,
    SPELL_DEFLECT_MISSILES,
    SPELL_ORB_OF_FRAGMENTATION,        //  160
    SPELL_ICE_BOLT,
    SPELL_ICE_STORM,
    SPELL_ARC,
    SPELL_AIRSTRIKE,
    SPELL_SHADOW_CREATURES,            //  165
    SPELL_CONFUSING_TOUCH,
    SPELL_SURE_BLADE,
//jmf: new spells
    SPELL_FLAME_TONGUE,
    SPELL_PASSWALL,
    SPELL_IGNITE_POISON,               //  170
    SPELL_STICKS_TO_SNAKES,
    SPELL_SUMMON_LARGE_MAMMAL,         // e.g. hound
    SPELL_SUMMON_DRAGON,
    SPELL_TAME_BEASTS,                 // charm/enslave but only animals
    SPELL_SLEEP,                       //  175
    SPELL_MASS_SLEEP,
    SPELL_DETECT_MAGIC,                //jmf: unfinished, perhaps useless
    SPELL_DETECT_SECRET_DOORS,
    SPELL_SEE_INVISIBLE,
    SPELL_FORESCRY,                    //  180
    SPELL_SUMMON_BUTTERFLIES,
    SPELL_WARP_BRAND,
    SPELL_SILENCE,
    SPELL_SHATTER,
    SPELL_DISPERSAL,                   //  185
    SPELL_DISCHARGE,
    SPELL_BEND,
    SPELL_BACKLIGHT,
    SPELL_INTOXICATE,   // confusion but only "smart" creatures
    SPELL_EVAPORATE,    // turn a potion into a cloud
    SPELL_ERINGYAS_SURPRISING_BOUQUET, // turn sticks into herbivore food
    SPELL_FRAGMENTATION,               // replacement for "orb of frag"
    SPELL_AIR_WALK,                    // "dematerialize" (air/transmigration)
    SPELL_SANDBLAST,     // mini-frag; can use stones for material comp   195
    SPELL_ROTTING,       // evil god power or necromantic transmigration
    SPELL_MAXWELLS_SILVER_HAMMER,      // vorpal-brand maces etc.
    SPELL_CONDENSATION_SHIELD,         // "shield" of icy vapour
    SPELL_SEMI_CONTROLLED_BLINK,       //jmf: to test effect             
    SPELL_STONESKIN,                   // 200
    SPELL_SIMULACRUM,
    SPELL_CONJURE_BALL_LIGHTNING,
    SPELL_CHAIN_LIGHTNING,
    SPELL_EXCRUCIATING_WOUNDS,
    SPELL_PORTAL_PROJECTILE,

    // Mostly monster-only spells after this point:
    SPELL_HELLFIRE_BURST,             // 205
    SPELL_VAMPIRE_SUMMON,
    SPELL_BRAIN_FEED,
    SPELL_FAKE_RAKSHASA_SUMMON,
    SPELL_STEAM_BALL,
    SPELL_SUMMON_UFETUBUS,            // 210
    SPELL_SUMMON_BEAST,
    SPELL_ENERGY_BOLT,
    SPELL_POISON_SPLASH,
    SPELL_SUMMON_UNDEAD,
    SPELL_CANTRIP,                    // 215
    SPELL_QUICKSILVER_BOLT,
    SPELL_METAL_SPLINTERS,
    SPELL_MIASMA,
    SPELL_SUMMON_DRAKES,
    SPELL_BLINK_OTHER,                // 220
    SPELL_SUMMON_MUSHROOMS,
    
    NUM_SPELLS,
    SPELL_NO_SPELL = 250               //  255 - added 22jan2000 {dlb}
};

enum slot_select_mode
{
    SS_FORWARD      = 0,
    SS_BACKWARD     = 1
};

enum stat_type
{
  STAT_STRENGTH,                     //    0
  STAT_DEXTERITY,
  STAT_INTELLIGENCE,
  NUM_STATS, // added for increase_stats() {dlb}
  STAT_ALL, // must remain after NUM_STATS -- added to handle royal jelly, etc. {dlb}
  STAT_RANDOM = 255 // leave at 255, added for increase_stats() handling {dlb}
};

enum targeting_type
{
    DIR_NONE,
    DIR_TARGET,
    DIR_DIR
};

enum torment_source_type
{
    TORMENT_GENERIC = -1,
    TORMENT_CARDS = -2,         // Symbol of torment
    TORMENT_SPWLD = -3,         // Special wield torment
    TORMENT_SCROLL = -4,
    TORMENT_SPELL = -5          // SPELL_SYMBOL_OF_TORMENT
};

enum trap_type                         // env.trap_type[]
{
    TRAP_DART,                         //    0
    TRAP_ARROW,
    TRAP_SPEAR,
    TRAP_AXE,
    TRAP_TELEPORT,
    TRAP_ALARM,                        //    5
    TRAP_BLADE,
    TRAP_BOLT,
    TRAP_NET,
    TRAP_ZOT,
    TRAP_NEEDLE,
    TRAP_SHAFT,
    NUM_TRAPS,                         // must remain last 'regular' member {dlb}
    TRAP_UNASSIGNED = 100,             // keep set at 100 for now {dlb}
    TRAP_INDEPTH = 253,                // Level-appropriate trap.
    TRAP_NONTELEPORT = 254,
    TRAP_RANDOM = 255                  // set at 255 to avoid potential conflicts {dlb}
};

// any change in this list warrants an increase in the
// version number in tutorial.cc
enum tutorial_event_type
{
    TUT_SEEN_FIRST_OBJECT,  // 0
    // seen certain items
    TUT_SEEN_POTION,
    TUT_SEEN_SCROLL,
    TUT_SEEN_WAND,
    TUT_SEEN_SPBOOK,
    TUT_SEEN_JEWELLERY,     // 5
    TUT_SEEN_MISC,
    TUT_SEEN_STAFF,
    TUT_SEEN_WEAPON,
    TUT_SEEN_MISSILES,
    TUT_SEEN_ARMOUR,        // 10
    TUT_SEEN_RANDART,
    TUT_SEEN_FOOD,
    TUT_SEEN_CARRION,
    // encountered dungeon features
    TUT_SEEN_STAIRS,        // 14
    TUT_SEEN_ESCAPE_HATCH,
    TUT_SEEN_TRAP,
    TUT_SEEN_ALTAR,         
    TUT_SEEN_SHOP,          
    TUT_SEEN_DOOR,
    // other 'first events'
    TUT_SEEN_MONSTER,       // 20
    TUT_MONSTER_BRAND,
    TUT_KILLED_MONSTER,
    TUT_NEW_LEVEL,       
    TUT_SKILL_RAISE,
    TUT_MAKE_CHUNKS,        // 25
    TUT_OFFER_CORPSE,
    TUT_NEW_ABILITY,
    TUT_FLEEING_MONSTER,
    TUT_ROTTEN_FOOD,
    // status changes
    TUT_YOU_ENCHANTED,      // 30
    TUT_YOU_SICK,
    TUT_YOU_POISON,
    TUT_YOU_CURSED,
    TUT_YOU_HUNGRY,
    TUT_YOU_STARVING,       // 35
    TUT_YOU_MUTATED,
    TUT_POSTBERSERK,
    // warning
    TUT_RUN_AWAY,           // 38
    TUT_RETREAT_CASTER,
    TUT_WIELD_WEAPON,
    TUT_NEED_HEALING,
    TUT_NEED_POISON_HEALING,
    // interface
    TUT_MULTI_PICKUP,       // 43
    TUT_HEAVY_LOAD,
    TUT_SHIFT_RUN,
    TUT_MAP_VIEW,
    TUT_DONE_EXPLORE,
    TUT_EVENTS_NUM          // 48
}; // for numbers higher than 48 change size of tutorial_events in externs.h

enum tutorial_types
{
    TUT_BERSERK_CHAR,
    TUT_MAGIC_CHAR,
    TUT_RANGER_CHAR,
    TUT_TYPES_NUM   // 3
};    

enum undead_state_type                // you.is_undead
{
    US_ALIVE = 0,
    US_HUNGRY_DEAD,
    US_UNDEAD
};

enum unique_item_status_type
{
    UNIQ_NOT_EXISTS = 0,
    UNIQ_EXISTS = 1,
    UNIQ_LOST_IN_ABYSS = 2
};

#ifdef WIZARD

enum wizard_option_type
{
    WIZ_NEVER,                         // protect player from accidental wiz
    WIZ_NO,                            // don't start character in wiz mode
    WIZ_YES                            // start character in wiz mode
};

#endif 

#endif // ENUM_H
