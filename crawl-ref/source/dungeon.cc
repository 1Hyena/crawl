/*
 *  File:       dungeon.cc
 *  Summary:    Functions used when building new levels.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *
 *   <9>     07-Aug-2001 MV     clean up of give_item; distribution of
 *                              wands, potions and scrolls
 *                              underground rivers and lakes
 *   <8>     02-Apr-2001 gdl    cleanup; nuked all globals
 *   <7>     06-Mar-2000 bwr    reduced vorpal weapon freq,
 *                              spellbooks now hold up to eight spells.
 *   <6>     11/06/99    cdl    random3 -> random2
 *   <5>      8/08/99    BWR    Upped rarity of unique artefacts
 *   <4>      7/13/99    BWR    Made pole arms of speed.
 *   <3>      5/22/99    BWR    Made named artefact weapons
 *                              rarer, Sword of Power esp.
 *   <2>      5/09/99    LRH    Replaced some sanity checking code in
 *                              spellbook_template with a corrected version
 *                                              using ASSERTs.
 *   <1>      -/--/--    LRH    Created
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <set>
#include <sstream>

#include "AppHdr.h"
#include "abyss.h"
#include "branch.h"
#include "cloud.h"
#include "defines.h"
#include "enum.h"
#include "externs.h"
#include "direct.h"
#include "dungeon.h"
#include "files.h"
#include "food.h"
#include "itemname.h"
#include "itemprop.h"
#include "items.h"
#include "makeitem.h"
#include "mapdef.h"
#include "mapmark.h"
#include "maps.h"
#include "message.h"
#include "misc.h"
#include "mon-util.h"
#include "mon-pick.h"
#include "monplace.h"
#include "monstuff.h"
#include "notes.h"
#include "place.h"
#include "player.h"
#include "randart.h"
#include "spells3.h"
#include "spl-book.h"
#include "state.h"
#include "stuff.h"
#include "tags.h"
#include "terrain.h"
#include "traps.h"
#include "travel.h"
#include "view.h"

#ifdef WIZARD
#include "cio.h" // for cancelable_get_line()
#endif

#define MAX_PIT_MONSTERS   10

struct pit_mons_def
{
    int type;
    int rare;
};

struct spec_room
{
    bool created;
    bool hooked_up;
    int x1;
    int y1;
    int x2;
    int y2;

    spec_room() : created(false), hooked_up(false), x1(0), y1(0), x2(0), y2(0)
    {
    }
};

struct dist_feat
{
    int dist;
    dungeon_feature_type feat;

    dist_feat(int _d = 0, dungeon_feature_type _f = DNGN_UNSEEN)
        : dist(_d), feat(_f)
        {
        }
};

// DUNGEON BUILDERS
static void build_dungeon_level(int level_number, int level_type);
static void reset_level();
static bool valid_dungeon_level(int level_number, int level_type);

static bool find_in_area(int sx, int sy, int ex, int ey,
                         dungeon_feature_type feature);
static bool make_box(int room_x1, int room_y1, int room_x2, int room_y2,
                     dungeon_feature_type floor=DNGN_UNSEEN,
                     dungeon_feature_type wall=DNGN_UNSEEN,
                     dungeon_feature_type avoid=DNGN_UNSEEN);
static void replace_area(int sx, int sy, int ex, int ey,
                         dungeon_feature_type replace,
                         dungeon_feature_type feature,
                         unsigned mmask = 0);
static builder_rc_type builder_by_type(int level_number, char level_type);
static builder_rc_type builder_by_branch(int level_number);
static builder_rc_type builder_normal(int level_number, char level_type, spec_room &s);
static builder_rc_type builder_basic(int level_number);
static void builder_extras(int level_number, int level_type);
static void builder_items(int level_number, char level_type, int items_wanted);
static void builder_monsters(int level_number, char level_type, int mon_wanted);
static void place_specific_stair(dungeon_feature_type stair,
                                 const std::string &tag = "",
                                 int dl = 0,
                                 bool vault_only = false);
static void place_branch_entrances(int dlevel, char level_type);
static void place_extra_vaults();
static void place_minivaults(const std::string &tag = "",
                             int fewest = -1, int most = -1,
                             bool force = false);
static void place_traps( int level_number );
static void place_fog_machines( int level_number );
static void prepare_swamp();
static void prepare_shoals( int level_number );
static void prepare_water( int level_number );
static void check_doors();
static void hide_doors();
static void make_trail(int xs, int xr, int ys, int yr,int corrlength,
                       int intersect_chance,
                       int no_corr, dungeon_feature_type begin,
                       dungeon_feature_type end=DNGN_UNSEEN);
static bool make_room(int sx,int sy,int ex,int ey,int max_doors, int doorlevel);
static void place_pool(dungeon_feature_type pool_type, unsigned char pool_x1,
                       unsigned char pool_y1, unsigned char pool_x2,
                       unsigned char pool_y2);
static void many_pools(dungeon_feature_type pool_type);
static bool join_the_dots(const coord_def &from, const coord_def &to,
                          unsigned mmask, bool early_exit = false);
static bool join_the_dots_rigorous(const coord_def &from,
                                   const coord_def &to,
                                   unsigned mapmask,
                                   bool early_exit = false);

static void build_river(dungeon_feature_type river_type); //mv
static void build_lake(dungeon_feature_type lake_type); //mv

static void spotty_level(bool seeded, int iterations, bool boxy);
static void bigger_room();
static void plan_main(int level_number, int force_plan);
static char plan_1();
static char plan_2();
static char plan_3();
static char plan_4(char forbid_x1, char forbid_y1, char forbid_x2,
                   char forbid_y2, dungeon_feature_type force_wall);
static char plan_5();
static char plan_6(int level_number);
static bool octa_room(spec_room &sr, int oblique_max,
                      dungeon_feature_type type_floor);
static void portal_vault_level(int level_number);
static void labyrinth_level(int level_number);
static void box_room(int bx1, int bx2, int by1, int by2,
                     dungeon_feature_type wall_type);
static int box_room_doors( int bx1, int bx2, int by1, int by2, int new_doors);
static void city_level(int level_number);
static void diamond_rooms(int level_number);

static void pick_float_exits(vault_placement &place,
                             std::vector<coord_def> &targets);
static void connect_vault(const vault_placement &vp);

// ITEM & SHOP FUNCTIONS
static void place_shops(int level_number, int nshops = 0);
static object_class_type item_in_shop(unsigned char shop_type);
static bool treasure_area(int level_number, unsigned char ta1_x,
                          unsigned char ta2_x, unsigned char ta1_y,
                          unsigned char ta2_y);
static void big_room(int level_number);
static void chequerboard(spec_room &sr, dungeon_feature_type target,
                         dungeon_feature_type floor1,
                         dungeon_feature_type floor2);
static void roguey_level(int level_number, spec_room &sr);
static void morgue(spec_room &sr);

// SPECIAL ROOM BUILDERS
static void special_room(int level_number, spec_room &sr);
static void specr_2(spec_room &sr);
static void beehive(spec_room &sr);
static void jelly_pit(int level_number, spec_room &sr);

// VAULT FUNCTIONS
static bool build_secondary_vault(int level_number, int vault,
                                  int rune_subst = -1,
                                  bool generating_level = true,
                                  bool clobber = false,
                                  bool make_no_exits = false,
                                  const coord_def &where = coord_def(-1, -1));
static bool build_vaults(int level_number, int vault_number,
                         int rune_subst = -1, bool build_only = false,
                         bool check_vault_place = false,
                         bool generating_level = true, bool clobber = false,
                         bool make_no_exits = false,
                         const coord_def &where = coord_def(-1, -1));
static bool build_minivaults(int level_number, int force_vault,
                             bool level_builder = true, bool clobber = false,
                             bool make_no_exits = false,
                             const coord_def &where = coord_def() );
static int vault_grid( vault_placement &,
                       int level_number, int vx, int vy, int altar_count,
                       FixedVector < object_class_type, 7 > &acq_item_class, 
                       int vgrid, std::vector<coord_def> &targets,
                       int &num_runes, int rune_subst = -1, bool foll = false);

static int dgn_random_map_for_place(bool wantmini);
static void dgn_load_colour_grid();
static void dgn_map_colour_fixup();

// ALTAR FUNCTIONS
static dungeon_feature_type pick_an_altar();
static void place_altar();
static void place_altars();

typedef std::list<coord_def> coord_list;

// MISC FUNCTIONS
static void dgn_set_floor_colours();

//////////////////////////////////////////////////////////////////////////
// Static data

// A mask of vaults and vault-specific flags.
map_mask dgn_map_mask;
std::vector<vault_placement> level_vaults;

static int vault_chance = 9;
static int minivault_chance = 3;
static bool dgn_level_vetoed = false;
static bool use_random_maps = true;
static bool dgn_check_connectivity = false;
static int  dgn_zones = 0;

struct coloured_feature
{
    dungeon_feature_type feature;
    int                  colour;

    coloured_feature() : feature(DNGN_UNSEEN), colour(BLACK) { }
    coloured_feature(dungeon_feature_type f, int c)
        : feature(f), colour(c)
    {
    }
};

struct dgn_colour_override_manager
{
    dgn_colour_override_manager()
    {
        dgn_load_colour_grid();
    }

    ~dgn_colour_override_manager()
    {
        dgn_map_colour_fixup();
    }
};

typedef FixedArray< coloured_feature, GXM, GYM > dungeon_colour_grid;
static std::auto_ptr<dungeon_colour_grid> dgn_colour_grid;

typedef std::map<std::string, std::string> callback_map;
static callback_map level_type_post_callbacks;

/**********************************************************************
 * builder() - kickoff for the dungeon generator.
 *********************************************************************/
bool builder(int level_number, int level_type)
{
    const std::set<std::string> uniq_tags = you.uniq_map_tags;
    const std::set<std::string> uniq_names = you.uniq_map_names;
    
    // N tries to build the level, after which we bail with a capital B.
    int tries = 20;
    while (tries-- > 0)
    {
#ifdef DEBUG_DIAGNOSTICS
        mapgen_report_map_build_start();
#endif
        
        dgn_level_vetoed = false;
        
        reset_level();

        // If we're getting low on available retries, disable random vaults
        // and minivaults (special levels will still be placed).
        if (tries < 5)
            use_random_maps = false;
        
        build_dungeon_level(level_number, level_type);
        dgn_set_floor_colours();

#ifdef DEBUG_DIAGNOSTICS
        if (dgn_level_vetoed)
            mapgen_report_map_veto();
#endif

        if (!dgn_level_vetoed && valid_dungeon_level(level_number, level_type))
        {
            dgn_map_colour_fixup();
            return (true);
        }

        you.uniq_map_tags  = uniq_tags;
        you.uniq_map_names = uniq_names;
    }

    if (!crawl_state.map_stat_gen)
    {
        // Failed to build level, bail out.
        save_game(true,
                  make_stringf("Unable to generate level for '%s'!",
                               level_id::current().describe().c_str()).c_str());
    }
    return (false);
}

void level_welcome_messages()
{
    for (int i = 0, size = level_vaults.size(); i < size; ++i)
    {
        const std::vector<std::string> &msgs =
            level_vaults[i].map.welcome_messages;
        for (int j = 0, msize = msgs.size(); j < msize; ++j)
            mpr(msgs[j].c_str());
    }
}

void level_clear_vault_memory()
{
    level_vaults.clear();
    dgn_map_mask.init(0);
}

static void dgn_load_colour_grid()
{
    dgn_colour_grid.reset(new dungeon_colour_grid);
    dungeon_colour_grid &dcgrid(*dgn_colour_grid);
    for (int y = Y_BOUND_1; y <= Y_BOUND_2; ++y)
        for (int x = X_BOUND_1; x <= X_BOUND_2; ++x)
            if (env.grid_colours[x][y] != BLACK)
                dcgrid[x][y] =
                    coloured_feature(grd[x][y], env.grid_colours[x][y]);
}

static void dgn_map_colour_fixup()
{
    if (!dgn_colour_grid.get())
        return;

    // If the original coloured feature has been changed, reset the colour.
    const dungeon_colour_grid &dcgrid(*dgn_colour_grid);
    for (int y = Y_BOUND_1; y <= Y_BOUND_2; ++y)
        for (int x = X_BOUND_1; x <= X_BOUND_2; ++x)
            if (dcgrid[x][y].colour != BLACK
                && grd[x][y] != dcgrid[x][y].feature)
            {
                env.grid_colours[x][y] = BLACK;
            }

    dgn_colour_grid.reset(NULL);
}

bool set_level_flags(unsigned long flags, bool silent)
{
    bool could_control = allow_control_teleport(true);
    bool could_map     = player_in_mappable_area();

    unsigned long old_flags = env.level_flags;
    env.level_flags |= flags;

    bool can_control = allow_control_teleport(true);
    bool can_map     = player_in_mappable_area();

    if (you.skills[SK_TRANSLOCATIONS] > 0
        && could_control && !can_control && !silent)
    {
        mpr("You sense the appearence of a powerful magical force "
            "which warps space.", MSGCH_WARN);
    }

    if (could_map && !can_map && !silent)
    {
        mpr("A powerful force appears that prevents you from "
            "remembering where you've been.", MSGCH_WARN);
    }

    return (old_flags != env.level_flags);
}

bool unset_level_flags(unsigned long flags, bool silent)
{
    bool could_control = allow_control_teleport(true);
    bool could_map     = player_in_mappable_area();

    unsigned long old_flags = env.level_flags;
    env.level_flags &= ~flags;

    bool can_control = allow_control_teleport(true);
    bool can_map     = player_in_mappable_area();

    if (you.skills[SK_TRANSLOCATIONS] > 0
        && !could_control && can_control && !silent)
    {
        // Isn't really a "recovery", but I couldn't think of where
        // else to send it.
        mpr("You sense the disappearance of a powerful magical force "
            "which warped space.", MSGCH_RECOVERY);
    }

    if (!could_map && can_map && !silent)
    {
        // Isn't really a "recovery", but I couldn't think of where
        // else to send it.
        mpr("You sense the disappearance of the force that prevented you "
            "from remembering where you've been.", MSGCH_RECOVERY);
    }

    return (old_flags != env.level_flags);
}

void dgn_set_grid_colour_at(const coord_def &c, int colour)
{
    if (colour != BLACK)
    {
        env.grid_colours(c) = colour;
        if (!dgn_colour_grid.get())
            dgn_colour_grid.reset( new dungeon_colour_grid );
        (*dgn_colour_grid)(c) = coloured_feature(grd(c), colour);
    }
}

static void dgn_register_vault(const map_def &map)
{
    if (!map.has_tag("allow_dup"))
        you.uniq_map_names.insert(map.name);

    std::vector<std::string> tags = split_string(" ", map.tags);
    for (int t = 0, ntags = tags.size(); t < ntags; ++t)
    {
        const std::string &tag = tags[t];
        if (tag.find("uniq_") == 0)
            you.uniq_map_tags.insert(tag);
    }
}

static inline bool dgn_grid_is_passable(dungeon_feature_type grid)
{
    // Rock wall check is superfluous, but is the most common case.
    return !(grid == DNGN_ROCK_WALL
             || (grid_is_solid(grid) && grid != DNGN_CLOSED_DOOR
                 && grid != DNGN_SECRET_DOOR));
}

static inline bool dgn_square_is_passable(const coord_def &c)
{
    return (!(dgn_map_mask(c) & MMT_OPAQUE) && dgn_grid_is_passable(grd(c)));
}

static inline void dgn_point_record_stub(const coord_def &) { }

template <class point_record>
static void dgn_fill_zone(
    const coord_def &c, int zone, point_record &prec,
    bool (*passable)(const coord_def &) = dgn_square_is_passable)
{
    // No bounds checks, assuming the level has at least one layer of
    // rock border.
    travel_point_distance[c.x][c.y] = zone;
    for (int yi = -1; yi <= 1; ++yi)
    {
        for (int xi = -1; xi <= 1; ++xi)
        {
            if (!xi && !yi)
                continue;

            const coord_def cp(c.x + xi, c.y + yi);
            if (travel_point_distance[cp.x][cp.y] || !passable(cp))
                continue;

            prec(cp);
            dgn_fill_zone(cp, zone, prec);
        }
    }
}

// Counts the number of mutually unreachable areas in the map,
// excluding isolated zones within vaults (we assume the vault author
// knows what she's doing). This is an easy way to check whether a map
// has isolated parts of the level that were not formerly isolated.
//
// All squares within vaults are treated as non-reachable, to simplify
// life, because vaults may change the level layout and isolate
// different areas without changing the number of isolated areas.
// Here's a before and after example of such a vault that would cause
// problems if we considered floor in the vault as non-isolating (the
// vault is represented as V for walls and o for floor squares in the
// vault).
//
// Before:
// 
//   xxxxx    xxxxx
//   x<..x    x.2.x
//   x.1.x    xxxxx  3 isolated zones
//   x>..x    x.3.x
//   xxxxx    xxxxx
//   
// After:
// 
//   xxxxx    xxxxx
//   x<1.x    x.2.x
//   VVVVVVVVVVoooV  3 isolated zones, but the isolated zones are different.
//   x>3.x    x...x
//   xxxxx    xxxxx
//
static int dgn_count_disconnected_zones()
{
    memset(travel_point_distance, 0, sizeof(travel_distance_grid_t));
    int nzones = 0;
    for (int y = 0; y < GYM; ++y)
    {
        for (int x = 0; x < GXM; ++x)
        {
            if (travel_point_distance[x][y]
                || !dgn_square_is_passable(coord_def(x, y)))
                continue;

            dgn_fill_zone(coord_def(x, y), ++nzones, dgn_point_record_stub);
        }
    }

    return (nzones);
}

static void fixup_pandemonium_stairs()
{
    for (int i = 0; i < GXM; i++)
    {
        for (int j = 0; j < GYM; j++)
        {
            if (grd[i][j] >= DNGN_STONE_STAIRS_UP_I
                && grd[i][j] <= DNGN_ROCK_STAIRS_UP)
            {
                if (one_chance_in( you.mutation[MUT_PANDEMONIUM] ? 5 : 50 ))
                    grd[i][j] = DNGN_EXIT_PANDEMONIUM;
                else
                    grd[i][j] = DNGN_FLOOR;
            }
            
            if (grd[i][j] >= DNGN_ENTER_LABYRINTH
                && grd[i][j] <= DNGN_ROCK_STAIRS_DOWN)
            {
                grd[i][j] = DNGN_TRANSIT_PANDEMONIUM;
            }
        }
    }
}

static void mask_vault(const vault_placement &place, unsigned mask)
{
    for (int y = place.pos.y + place.size.y - 1; y >= place.pos.y; --y)
        for (int x = place.pos.x + place.size.x - 1; x >= place.pos.x; --x)
            if (place.map.in_map(coord_def(x - place.pos.x, y - place.pos.y)))
                dgn_map_mask[x][y] |= mask;
}

static void register_place(const vault_placement &place)
{
    dgn_register_vault(place.map);
    
    mask_vault(place, MMT_VAULT | MMT_NO_DOOR);
    if (place.map.has_tag("no_monster_gen"))
        mask_vault(place, MMT_NO_MONS);

    if (place.map.has_tag("no_item_gen"))
        mask_vault(place, MMT_NO_ITEM);

    if (place.map.has_tag("no_pool_fixup"))
        mask_vault(place, MMT_NO_POOL);

    if (!place.map.has_tag("transparent"))
        mask_vault(place, MMT_OPAQUE);

    // Now do per-square by-symbol masking
    for (int y = place.pos.y + place.size.y - 1; y >= place.pos.y; --y)
        for (int x = place.pos.x + place.size.x - 1; x >= place.pos.x; --x)
            if (place.map.in_map(coord_def(x - place.pos.x, y - place.pos.y)))
            {
                int key = place.map.map.glyph(x - place.pos.x, y - place.pos.y);
                const keyed_mapspec* spec = place.map.mapspec_for_key(key);

                if (spec != NULL)
                {
                    dgn_map_mask[x][y] |= (short)spec->map_mask.flags_set;
                    dgn_map_mask[x][y] &= ~((short)spec->map_mask.flags_unset);
                }
            }
    
    set_branch_flags(place.map.branch_flags.flags_set, true);
    unset_branch_flags(place.map.branch_flags.flags_unset, true);

    set_level_flags(place.map.level_flags.flags_set, true);
    unset_level_flags(place.map.level_flags.flags_unset, true);

    if (place.map.floor_colour != BLACK)
        env.floor_colour = place.map.floor_colour;

    if (place.map.rock_colour != BLACK)
        env.rock_colour = place.map.rock_colour;
}

static bool ensure_vault_placed(bool vault_success)
{
    if (!vault_success)
        dgn_level_vetoed = true;
    else
        vault_chance = 0;
    return (vault_success);
}

static coord_def find_level_feature(int feat)
{
    for (int y = 1; y < GYM; ++y)
    {
        for (int x = 1; x < GXM; ++x)
        {
            if (grd[x][y] == feat)
                return coord_def(x, y);
        }
    }
    return coord_def(0, 0);
}

static bool has_connected_downstairs_from(const coord_def &c)
{
    flood_find<feature_grid, coord_predicate> ff(env.grid, in_bounds);
    ff.add_feat(DNGN_STONE_STAIRS_DOWN_I);
    ff.add_feat(DNGN_STONE_STAIRS_DOWN_II);
    ff.add_feat(DNGN_STONE_STAIRS_DOWN_III);
    ff.add_feat(DNGN_ROCK_STAIRS_DOWN);

    coord_def where = ff.find_first_from(c, dgn_map_mask);
    return (where.x || !ff.did_leave_vault());
}

static bool is_level_stair_connected()
{
    coord_def up = find_level_feature(DNGN_STONE_STAIRS_UP_I);
    if (up.x && up.y)
        return has_connected_downstairs_from(up);

    return (false);
}

static bool valid_dungeon_level(int level_number, int level_type)
{
    if (level_number == 0 && level_type == LEVEL_DUNGEON)
        return is_level_stair_connected();

    return (true);
}

static void reset_level()
{
    level_clear_vault_memory();
    dgn_colour_grid.reset(NULL);
    
    vault_chance     = 9;
    minivault_chance = 3;
    use_random_maps  = true;
    dgn_check_connectivity = false;
    dgn_zones        = 0;

    // blank level with DNGN_ROCK_WALL
    grd.init(DNGN_ROCK_WALL);
    env.grid_colours.init(BLACK);

    // delete all traps
    for (int i = 0; i < MAX_TRAPS; i++)
        env.trap[i].type = TRAP_UNASSIGNED;

    // initialize all items
    for (int i = 0; i < MAX_ITEMS; i++)
        init_item( i );

    // reset all monsters
    for (int i = 0; i < MAX_MONSTERS; i++)
        menv[i].type = -1;

    for (int i = 0; i < 20; i++)
        env.mons_alloc[i] = -1;

    mgrd.init(NON_MONSTER);
    igrd.init(NON_ITEM);

    // reset all shops
    for (int shcount = 0; shcount < MAX_SHOPS; shcount++)
        env.shop[shcount].type = SHOP_UNASSIGNED;

    // clear all markers
    env.markers.clear();

    // Set default level flags
    if (you.level_type == LEVEL_DUNGEON)
        env.level_flags = branches[you.where_are_you].default_level_flags;
    else if (you.level_type == LEVEL_LABYRINTH ||
             you.level_type == LEVEL_ABYSS)
    {
        env.level_flags = LFLAG_NO_TELE_CONTROL | LFLAG_NOT_MAPPABLE;

        if (!(you.level_type == LEVEL_LABYRINTH
              && you.species != SP_MINOTAUR))
        {
            env.level_flags |= LFLAG_NO_MAGIC_MAP;
        }
    }
    else
        env.level_flags = 0;

    env.floor_colour = BLACK;
    env.rock_colour  = BLACK;
}

static void build_layout_skeleton(int level_number, int level_type,
                                  spec_room &sr)
{
    builder_rc_type skip_build = builder_by_type(level_number, level_type);

    if (skip_build == BUILD_QUIT)       // quit immediately
        return;

    if (skip_build == BUILD_CONTINUE)
    {
        skip_build = builder_by_branch(level_number);

        if (skip_build == BUILD_QUIT || dgn_level_vetoed)
            return;
    }

    if (!dgn_level_vetoed && skip_build == BUILD_CONTINUE)
    {
        // do 'normal' building.  Well, except for the swamp and shoals.
        if (!player_in_branch(BRANCH_SWAMP) &&
            !player_in_branch(BRANCH_SHOALS))
            skip_build = builder_normal(level_number, level_type, sr);

        if (!dgn_level_vetoed && skip_build == BUILD_CONTINUE)
        {
            skip_build = builder_basic(level_number);
            if (!dgn_level_vetoed && skip_build == BUILD_CONTINUE)
                builder_extras(level_number, level_type);
        }
    }
}

static int num_items_wanted(int level_number)
{
    if (level_number > 5 && one_chance_in(500 - 5 * level_number))
        return 10 + random2avg( 90, 2 );  // rich level!
    else
        return 3 + roll_dice( 3, 11 );
}


static int num_mons_wanted(int level_type)
{
    if (level_type == LEVEL_ABYSS ||
        player_in_branch( BRANCH_ECUMENICAL_TEMPLE ))
        return 0;

    int mon_wanted = roll_dice( 3, 10 );

    if (player_in_hell())
        mon_wanted += roll_dice( 3, 8 );
    else if (player_in_branch( BRANCH_HALL_OF_BLADES ))
        mon_wanted += roll_dice( 6, 8 );
    
    if (mon_wanted > 60)
        mon_wanted = 60;

    return mon_wanted;
}

static void fixup_walls()
{
    // If level part of Dis -> all walls metal;
    // If part of vaults -> walls depend on level;
    // If part of crypt -> all walls stone:

    if (player_in_branch( BRANCH_DIS )
        || player_in_branch( BRANCH_VAULTS )
        || player_in_branch( BRANCH_CRYPT ))
    {
        // always the case with Dis {dlb}
        dungeon_feature_type vault_wall = DNGN_METAL_WALL;

        if (player_in_branch( BRANCH_VAULTS ))
        {
            vault_wall = DNGN_ROCK_WALL;
            const int bdepth = player_branch_depth();

            if ( bdepth > 2 )
                vault_wall = DNGN_STONE_WALL;

            if ( bdepth > 4 )
                vault_wall = DNGN_METAL_WALL;

            if ( bdepth > 6 && one_chance_in(10))
                vault_wall = DNGN_GREEN_CRYSTAL_WALL;
        }
        else if (player_in_branch( BRANCH_CRYPT ))
        {
            vault_wall = DNGN_STONE_WALL;
        }

        replace_area(0,0,GXM-1,GYM-1,DNGN_ROCK_WALL,vault_wall);
    }
}

static void fixup_branch_stairs()
{
    // Top level of branch levels - replaces up stairs
    // with stairs back to dungeon or wherever:
    if ( your_branch().exit_stairs != NUM_FEATURES &&
         player_branch_depth() == 1 &&
         you.level_type == LEVEL_DUNGEON )
    {
        const dungeon_feature_type exit = your_branch().exit_stairs;
        for (int x = 1; x < GXM; x++)
            for (int y = 1; y < GYM; y++)
                if (grd[x][y] >= DNGN_STONE_STAIRS_UP_I
                    && grd[x][y] <= DNGN_ROCK_STAIRS_UP)
                {
                    if (grd[x][y] == DNGN_STONE_STAIRS_UP_I)
                        env.markers.add(
                            new map_feature_marker(coord_def(x,y),
                                                   grd[x][y]));
                    grd[x][y] = exit;
                }
    }

    // bottom level of branch - replaces down stairs with up ladders:
    if ( player_branch_depth() == your_branch().depth &&
         you.level_type == LEVEL_DUNGEON &&
         you.where_are_you != BRANCH_VESTIBULE_OF_HELL )
    {
        for (int x = 1; x < GXM; x++)
            for (int y = 1; y < GYM; y++)
                if (grd[x][y] >= DNGN_STONE_STAIRS_DOWN_I
                    && grd[x][y] <= DNGN_ROCK_STAIRS_DOWN)
                    grd[x][y] = DNGN_ROCK_STAIRS_UP;
    }

    // hall of blades (1 level deal) - no down staircases, thanks!
    // XXX XXX why the special-casing?
    if (player_in_branch( BRANCH_HALL_OF_BLADES ))
    {
        for (int x = 1; x < GXM; x++)
        {
            for (int y = 1; y < GYM; y++)
            {
                if (grd[x][y] >= DNGN_STONE_STAIRS_DOWN_I
                    && grd[x][y] <= DNGN_ROCK_STAIRS_UP)
                {
                    grd[x][y] = DNGN_FLOOR;
                }
            }
        }
    }
}

static void fixup_duplicate_stairs()
{
    // This function ensures that there is no more than one of each up and down
    // stone stairs I, II, and III.  More than three stairs will result in
    // turning additional stairs into rock stairs (with an attempt to keep
    // level connectivity).

    const unsigned int max_stairs = 20;
    FixedVector<coord_def, max_stairs> up_stairs;
    FixedVector<coord_def, max_stairs> down_stairs;
    unsigned int num_up_stairs = 0;
    unsigned int num_down_stairs = 0;

    for (int x = 1; x < GXM; x++)
    {
        for (int y = 1; y < GYM; y++)
        {
            const coord_def c(x,y);
            if (grd(c) >= DNGN_STONE_STAIRS_DOWN_I && 
                grd(c) <= DNGN_STONE_STAIRS_DOWN_III &&
                num_down_stairs < max_stairs)
            {
                down_stairs[num_down_stairs++] = c;
            }
            else if (grd(c) >= DNGN_STONE_STAIRS_UP_I &&
                grd(c) <= DNGN_STONE_STAIRS_UP_III &&
                num_up_stairs < max_stairs)
            {
                up_stairs[num_up_stairs++] = c;
            }
        }
    }

    for (unsigned int i = 0; i < 2; i++)
    {
        FixedVector<coord_def, max_stairs>& stair_list = (i == 0) ?
            up_stairs : down_stairs;

        unsigned int num_stairs;
        dungeon_feature_type base;
        dungeon_feature_type replace;
        if (i == 0)
        {
            num_stairs = num_up_stairs;
            replace = DNGN_ROCK_STAIRS_UP;
            base = DNGN_STONE_STAIRS_UP_I;
        }
        else
        {
            num_stairs = num_down_stairs;
            replace = DNGN_ROCK_STAIRS_DOWN;
            base = DNGN_STONE_STAIRS_DOWN_I;
        }

        if (num_stairs > 3)
        {
            // Find pairwise stairs that are connected and turn one of them
            // into a rock stairs of the appropriate type.
            for (unsigned int s1 = 0; s1 < num_stairs; s1++)
            {
                if (num_stairs <= 3)
                    break;

                for (unsigned int s2 = s1 + 1; s2 < num_stairs; s2++)
                {
                    if (num_stairs <= 3)
                        break;

                    flood_find<feature_grid, coord_predicate> ff(env.grid,
                        in_bounds);

                    ff.add_feat(grd(stair_list[s2]));

                    // Ensure we're not searching for the feature at s1.
                    dungeon_feature_type save = grd(stair_list[s1]);
                    grd(stair_list[s1]) = DNGN_FLOOR;

                    coord_def where = ff.find_first_from(stair_list[s1],
                        dgn_map_mask);
                    if (where.x)
                    {
                        grd(stair_list[s2]) = replace;
                        num_stairs--;
                        stair_list[s2] = stair_list[num_stairs];
                        s2--;
                    }

                    grd(stair_list[s1]) = save;
                }
            }

            // If that doesn't work, remove random stairs.
            while (num_stairs > 3)
            {
                int remove = random2(num_stairs);
                grd(stair_list[remove]) = replace;

                stair_list[remove] = stair_list[--num_stairs];
            }
        }

        ASSERT(num_stairs <= 3);

        if (num_stairs <= 1)
            continue;

        // At this point, up_stairs and down_stairs contain no more than
        // three stairs.  Ensure that they are unique.
        for (int s = 0; s < (num_stairs == 3 ? 4 : 1); s++)
        {
            int s1 = s % num_stairs;
            int s2 = (s1 + 1) % num_stairs;
            ASSERT(grd(stair_list[s2]) >= base && 
                grd(stair_list[s2]) <= base + 3);

            if (grd(stair_list[s1]) == grd(stair_list[s2]))
            {
                grd(stair_list[s2]) = (dungeon_feature_type)(base + 
                    (grd(stair_list[s2])-base+1) % 3);
            }
        }
    }
}

static void dgn_verify_connectivity(unsigned nvaults)
{
    // After placing vaults, make sure parts of the level have not been
    // disconnected.
    if (!dgn_level_vetoed && dgn_zones && nvaults != level_vaults.size())
    {
        const int newzones = dgn_count_disconnected_zones();

#ifdef DEBUG_DIAGNOSTICS
        std::ostringstream vlist;
        for (unsigned i = nvaults; i < level_vaults.size(); ++i)
        {
            if (i > nvaults)
                vlist << ", ";
            vlist << level_vaults[i].map.name;
        }
        mprf(MSGCH_DIAGNOSTICS, "Dungeon has %d zones after placing %s.",
             newzones, vlist.str().c_str());
#endif
        if (newzones > dgn_zones)
        {
            dgn_level_vetoed = true;
#ifdef DEBUG_DIAGNOSTICS
            mprf(MSGCH_DIAGNOSTICS,
                 "VETO: %s broken by [%s] (had %d zones, "
                 "now have %d zones.",
                 level_id::current().describe().c_str(),
                 vlist.str().c_str(), dgn_zones, newzones);
#endif
        }
    }    
}

static void build_dungeon_level(int level_number, int level_type)
{
    spec_room sr;

    build_layout_skeleton(level_number, level_type, sr);

    if (you.level_type == LEVEL_LABYRINTH
        || you.level_type == LEVEL_PORTAL_VAULT
        || dgn_level_vetoed)
    {
        return;
    }
    
    // hook up the special room (if there is one, and it hasn't
    // been hooked up already in roguey_level())
    if (sr.created && !sr.hooked_up)
        specr_2(sr);

    // now place items, monster, gates, etc.
    // stairs must exist by this point. Some items and monsters
    // already exist.

    // time to make the swamp or shoals {dlb}:
    if (player_in_branch( BRANCH_SWAMP ))
        prepare_swamp();
    else if (player_in_branch(BRANCH_SHOALS))
        prepare_shoals( level_number );

    if (dgn_level_vetoed)
        return;
    
    check_doors();

    if (!player_in_branch( BRANCH_DIS ) && !player_in_branch( BRANCH_VAULTS ))
        hide_doors();

    // change pre-rock (105) to rock, and pre-floor (106) to floor
    replace_area( 0,0,GXM-1,GYM-1, DNGN_BUILDER_SPECIAL_WALL, DNGN_ROCK_WALL );
    replace_area( 0,0,GXM-1,GYM-1, DNGN_BUILDER_SPECIAL_FLOOR, DNGN_FLOOR );

    const unsigned nvaults = level_vaults.size();

    // Any further vaults must make sure not to disrupt level layout.
    dgn_check_connectivity = true;
    
    // Try to place minivaults that really badly want to be placed. Still
    // no guarantees, seeing this is a minivault.
    if (!player_in_branch(BRANCH_SHOALS))
        place_minivaults();
    place_branch_entrances( level_number, level_type );
    place_extra_vaults();
    dgn_verify_connectivity(nvaults);
    
    if (dgn_level_vetoed)
        return;
    
    place_traps(level_number);
    place_fog_machines(level_number);
    
    // place items
    builder_items(level_number, level_type, num_items_wanted(level_number));

    // place monsters
    builder_monsters(level_number, level_type, num_mons_wanted(level_type));

    // place shops, if appropriate
    if ( level_type == LEVEL_DUNGEON && your_branch().has_shops )
        place_shops(level_number);

    fixup_walls();
    fixup_branch_stairs();

    place_altars();

    link_items();

    if (!player_in_branch(BRANCH_COCYTUS) &&
        !player_in_branch(BRANCH_SWAMP) &&
        !player_in_branch(BRANCH_SHOALS))
        prepare_water( level_number );

    // Translate stairs for pandemonium levels:
    if (level_type == LEVEL_PANDEMONIUM)
        fixup_pandemonium_stairs();

    fixup_duplicate_stairs();
}                               // end builder()


static char fix_black_colour(char incol)
{
    if ( incol == BLACK )
        return LIGHTGREY;
    else
        return incol;
}

void dgn_set_colours_from_monsters()
{
    if (env.mons_alloc[9] < 0 || env.mons_alloc[9] == MONS_PROGRAM_BUG
        || env.mons_alloc[9] >= NUM_MONSTERS)
    {
        if (env.floor_colour == BLACK)
            env.floor_colour = LIGHTGREY;
    }
    else
        env.floor_colour = fix_black_colour(mcolour[env.mons_alloc[9]]);


    if (env.mons_alloc[8] < 0 || env.mons_alloc[8] == MONS_PROGRAM_BUG
        || env.mons_alloc[8] >= NUM_MONSTERS)
    {
        if (env.rock_colour == BLACK)
            env.rock_colour = BROWN;
    }
    else
        env.rock_colour = fix_black_colour(mcolour[env.mons_alloc[8]]);
}

static void dgn_set_floor_colours()
{
    unsigned char old_floor_colour = env.floor_colour;
    unsigned char old_rock_colour  = env.rock_colour;

    if (you.level_type == LEVEL_PANDEMONIUM || you.level_type == LEVEL_ABYSS)
    {
        dgn_set_colours_from_monsters();
    }
    else if (you.level_type == LEVEL_DUNGEON)
    {
        // level_type == LEVEL_DUNGEON
        // Hall of Zot colours handled in dat/zot.des
        const int youbranch = you.where_are_you;
        env.floor_colour = branches[youbranch].floor_colour;
        env.rock_colour = branches[youbranch].rock_colour;
    }

    if (old_floor_colour != BLACK)
        env.floor_colour = old_floor_colour;
    if (old_rock_colour != BLACK)
        env.rock_colour = old_rock_colour;

    if (env.floor_colour == BLACK)
        env.floor_colour = LIGHTGREY;
    if (env.rock_colour == BLACK)
        env.rock_colour  = BROWN;
}

static void check_doors()
{
    for (int x = 1; x < GXM-1; x++)
    {
        for (int y = 1; y < GYM-1; y++)
        {
            if (grd[x][y] != DNGN_CLOSED_DOOR)
                continue;

            int solid_count = 0;

            if (grid_is_solid( grd[x - 1][y] ))
                solid_count++;

            if (grid_is_solid( grd[x + 1][y] ))
                solid_count++;

            if (grid_is_solid( grd[x][y - 1] ))
                solid_count++;

            if (grid_is_solid( grd[x][y + 1] ))
                solid_count++;

            grd[x][y] = ((solid_count < 2) ? DNGN_FLOOR : DNGN_CLOSED_DOOR);
        }
    }
}                               // end check_doors()

static void hide_doors()
{
    unsigned char dx = 0, dy = 0;     // loop variables
    unsigned char wall_count = 0;     // clarifies inner loop {dlb}

    for (dx = 1; dx < GXM-1; dx++)
    {
        for (dy = 1; dy < GYM-1; dy++)
        {
            // only one out of four doors are candidates for hiding {gdl}:
            if (grd[dx][dy] == DNGN_CLOSED_DOOR && one_chance_in(4)
                && unforbidden(coord_def(dx, dy), MMT_NO_DOOR))
            {
                wall_count = 0;

                if (grd[dx - 1][dy] == DNGN_ROCK_WALL)
                    wall_count++;

                if (grd[dx + 1][dy] == DNGN_ROCK_WALL)
                    wall_count++;

                if (grd[dx][dy - 1] == DNGN_ROCK_WALL)
                    wall_count++;

                if (grd[dx][dy + 1] == DNGN_ROCK_WALL)
                    wall_count++;

                // if door is attached to more than one wall, hide it {dlb}:
                if (wall_count > 1)
                    grd[dx][dy] = DNGN_SECRET_DOOR;
            }
        }
    }
}                               // end hide_doors()

// Places a randomized ellipse with centre (x,y) and half axes a and b
static void place_ellipse(int x, int y, int a, int b,
                          dungeon_feature_type feat, int margin)
{
    for (int i = std::max(x-a,margin); i < std::min(x+a,GXM-margin); ++i)
        for (int j = std::max(y-b,margin); j < std::min(y+b, GYM-margin); ++j)
            if ( (x-i)*(x-i)*b*b + (y-j)*(y-j)*a*a <=
                 a*a*b*b/2 + roll_dice(2, a*a*b*b)/4 + random2(3) )
                grd[i][j] = feat;
}

static int count_feature_in_box(int x0, int y0, int x1, int y1,
                                dungeon_feature_type feat)
{
    int result = 0;
    for ( int i = x0; i < x1; ++i )
        for ( int j = y0; j < y1; ++j )
            if ( grd[i][j] == feat )
                ++result;
    return result;
}

static int count_antifeature_in_box(int x0, int y0, int x1, int y1,
                                    dungeon_feature_type feat)
{
    return (x1-x0)*(y1-y0) - count_feature_in_box(x0,y0,x1,y1,feat);
}

// count how many neighbours of grd[x][y] are the feature feat.
static int count_neighbours(int x, int y, dungeon_feature_type feat)
{
    return count_feature_in_box(x-1, y-1, x+2, y+2, feat);
}

static void replace_in_grid(int x1, int y1, int x2, int y2,
                            dungeon_feature_type oldfeat,
                            dungeon_feature_type newfeat)
{
    for ( int x = x1; x < x2; ++x )
        for ( int y = y1; y < y2; ++y )
            if ( grd[x][y] == oldfeat )
                grd[x][y] = newfeat;
}

static void connected_flood(int margin, int i, int j, bool taken[GXM][GYM])
{
    if ( i < margin || i >= GXM - margin ||
         j < margin || j >= GYM - margin ||
         taken[i][j] )
        return;

    taken[i][j] = true;
    for ( int idelta = -1; idelta <= 1; ++idelta )
        for ( int jdelta = -1; jdelta <= 1; ++jdelta )
            connected_flood(margin, i + idelta, j + jdelta, taken);
}

// yes, yes, this can probably use travel to avoid duplicating code.
static int count_connected(int margin)
{
    bool taken[GXM][GYM];

    for ( int i = margin; i < GXM - margin; ++i )
        for ( int j = margin; j < GYM - margin; ++j )
            taken[i][j] = (grd[i][j] == DNGN_DEEP_WATER ||
                           grd[i][j] == DNGN_SHALLOW_WATER);

    int count = 0;

    for ( int i = margin; i < GXM - margin; ++i )
        for ( int j = margin; j < GYM - margin; ++j )
            if ( !taken[i][j] )
            {
                ++count;
                connected_flood(margin,i,j,taken);
            }

    return count;
}

static void place_base_islands(int margin, int num_islands, int estradius,
                               coord_def centres[10])
{
    for (int i = 0; i < num_islands; ++i)
    {
        // smaller axis
        int b = (2 * estradius + roll_dice(3, estradius)) / 4;
        b = std::max(b,4);
        b = std::min(b, (GYM - margin) / 2);

        int a = b + roll_dice(2,b)/3; // more wide than tall
        a = std::min(a, (GXM - margin) / 2);       
        
        int island_distance = estradius*estradius * (2 + num_islands/3);
        
        bool centre_ok;
        do
        {
            centre_ok = true;

            centres[i].x = a + random2(GXM-2*a-1);
            centres[i].y = b + random2(GYM-2*b-1);          
            
            for (int j = 0; j < i; ++j)
            {
                // calculate the distance from the centers of
                // previous islands
                if ( distance(centres[i].x, centres[i].y,
                              centres[j].x, centres[j].y) < island_distance )
                {
                    centre_ok = false;
                    break;
                }                   
            }
            if ( random2(num_islands) && island_distance )
                --island_distance;
        } while ( !centre_ok );
        
        // place an ellipse around the new coordinate
        place_ellipse( centres[i].x, centres[i].y, a, b, DNGN_FLOOR, margin);
    }
}

static void prepare_shoals(int level_number)
{
    // dpeg's algorithm.
    // We could have just used spotty_level() and changed rock to
    // water, but this is much cooler. Right?
    const int margin = 6;

    const bool at_bottom = (your_branch().depth == player_branch_depth());

    int num_islands = player_branch_depth() + 1;
    
    if ( at_bottom )
        num_islands += random2(3);

    const int estradius = 50 / num_islands - (num_islands == 2 ? 5 : 0);

    int num_tries = 0;
    coord_def centres[10];
    do {
        for (int x = margin; x < GXM-margin; ++x)
            for (int y = margin; y < GYM-margin; ++y)
                grd[x][y] = DNGN_DEEP_WATER;
        place_base_islands(margin, num_islands, estradius, centres);
    } while ( ++num_tries < 100 &&
              count_connected(margin) != num_islands );
#ifdef DEBUG_DIAGNOSTICS
    mprf(MSGCH_DIAGNOSTICS, "Num tries: %d Connected components: %d",
         num_tries, count_connected(margin));
#endif
    
    // Adding shallow water at deep water adjacent to floor.
    // Randomisation: place shallow water if at least 1d(1d3) floor neighbours
    for ( int i = margin; i < GXM - margin; ++i)
        for ( int j = margin; j < GYM - margin; ++j)
            if (grd[i][j] == DNGN_DEEP_WATER &&
                count_neighbours(i, j, DNGN_FLOOR) > random2(random2(3)+1))
                grd[i][j] = DNGN_SHALLOW_WATER;

    // Placing sandbanks
    for (int banks = 0; banks < 8; ++banks)
    {
        int xsize = 3+random2(3);  // random rectangle
        int ysize = 3+random2(3);  
        int xb = random2(GXM - 2 * margin - 10) + margin + 2;
        int yb = random2(GYM - 2 * margin - 10) + margin + 2;

        bool ok_place = true;
        for ( int i = xb; i < xb + xsize; ++i )
            for ( int j = yb; j < yb + ysize; ++j )
                if ( grd[i][j] != DNGN_DEEP_WATER )
                    ok_place = false;

        if (ok_place)
        {
            for ( int i = xb; i < xb + xsize; ++i )
                for ( int j = yb; j < yb + ysize; ++j )
                    if ( !one_chance_in(3) )
                        grd[i][j] = DNGN_SHALLOW_WATER;
        }
    }

    // Fractalisation

    // LAVA is a placeholder for cells which will become shallow water
    // at the end of the current iteration.
    // WATER_STUCK is a placeholder for last iteration's generated water.
    replace_in_grid(margin, margin, GXM-margin, GYM-margin,
                    DNGN_SHALLOW_WATER, DNGN_WATER_STUCK);

    for ( int iteration = 0; iteration < 6; ++iteration )
    {
        for ( int x = margin; x < GXM - margin; ++x )
        {
            for ( int y = margin; y < GYM - margin; ++y )
            {
                if ( grd[x][y] == DNGN_DEEP_WATER )
                {
                    int badness = count_neighbours(x, y, DNGN_WATER_STUCK);
                    if ( random2(badness) >= 2 && coinflip() )
                        grd[x][y] = DNGN_LAVA;
                }
            }
        }
        replace_in_grid(margin, margin, GXM-margin, GYM-margin,
                        DNGN_LAVA, DNGN_WATER_STUCK);
    }
    replace_in_grid(margin, margin, GXM-margin, GYM-margin,
                    DNGN_WATER_STUCK, DNGN_SHALLOW_WATER);

    if ( at_bottom )
    {
        // Put all the stairs on one island
        grd[centres[0].x][centres[0].y] = DNGN_STONE_STAIRS_UP_I;
        grd[centres[0].x+1][centres[0].y] = DNGN_STONE_STAIRS_UP_II;
        grd[centres[0].x-1][centres[0].y] = DNGN_STONE_STAIRS_UP_III;
        
        // Place the rune
        int vaultidx;
        do {
            vaultidx = dgn_random_map_for_place(true);
        } while ( vaultidx == -1 ||
                  !map_by_index(vaultidx)->has_tag("has_rune") );

        build_minivaults( level_number, vaultidx, true, false, false,
                          centres[1] );

        for ( int i = 2; i < num_islands; ++i )
        {
            // Place (non-rune) minivaults on the other islands
            do {
                vaultidx = dgn_random_map_for_place(true);
            } while ( vaultidx == -1 ||
                      map_by_index(vaultidx)->has_tag("has_rune") );
            build_minivaults( level_number, vaultidx, true, false, false,
                              centres[i] );
        }
    }
    else
    {
        // Place stairs randomly. No elevators.   
        for ( int i = 0; i < 3; ++i )
        {
            int x, y;
            do {
                x = random2(GXM);
                y = random2(GYM);
            } while ( grd[x][y] != DNGN_FLOOR );
            grd[x][y] =
                static_cast<dungeon_feature_type>(
                    DNGN_STONE_STAIRS_DOWN_I + i );
            
            do {
                x = random2(GXM);
                y = random2(GYM);
            } while ( grd[x][y] != DNGN_FLOOR );
            grd[x][y] =
                static_cast<dungeon_feature_type>(
                    DNGN_STONE_STAIRS_UP_I + i);
        }
    }
}

static void prepare_swamp()
{
    const int margin = 10;

    for (int i = margin; i < (GXM - margin); i++)
    {
        for (int j = margin; j < (GYM - margin); j++)
        {
            // doors -> floors {dlb}
            if (grd[i][j] == DNGN_CLOSED_DOOR || grd[i][j] == DNGN_SECRET_DOOR)
                grd[i][j] = DNGN_FLOOR;

            // floors -> shallow water 1 in 3 times {dlb}
            if (grd[i][j] == DNGN_FLOOR && one_chance_in(3))
                grd[i][j] = DNGN_SHALLOW_WATER;

            // walls -> deep/shallow water or remain unchanged {dlb}
            if (grd[i][j] == DNGN_ROCK_WALL)
            {
                const int temp_rand = random2(6);

                if (temp_rand > 0)      // 17% chance unchanged {dlb}
                {
                    grd[i][j] = ((temp_rand > 2) ? DNGN_SHALLOW_WATER // 50%
                                                 : DNGN_DEEP_WATER);  // 33%
                }
            }
        }
    }
}                               // end prepare_swamp()

// Gives water which is next to ground/shallow water a chance of being
// shallow. Checks each water space.
static void prepare_water( int level_number )
{
    int i, j, k, l;             // loop variables {dlb}
    unsigned char which_grid;   // code compaction {dlb}

    for (i = 1; i < (GXM - 1); i++)
    {
        for (j = 1; j < (GYM - 1); j++)
        {
            if (!unforbidden(coord_def(i, j), MMT_NO_POOL))
                continue;
            
            if (grd[i][j] == DNGN_DEEP_WATER)
            {
                for (k = -1; k < 2; k++)
                {
                    for (l = -1; l < 2; l++)
                    {
                        if (k != 0 || l != 0)
                        {
                            which_grid = grd[i + k][j + l];

                            // must come first {dlb}
                            if (which_grid == DNGN_SHALLOW_WATER
                                && one_chance_in( 8 + level_number ))  
                            {
                                grd[i][j] = DNGN_SHALLOW_WATER;
                            }
                            else if (which_grid >= DNGN_FLOOR
                                     && random2(100) < 80 - level_number * 4)
                            {
                                grd[i][j] = DNGN_SHALLOW_WATER;
                            }
                        }
                    }
                }
            }
        }
    }
}                               // end prepare_water()

static bool find_in_area(int sx, int sy, int ex, int ey,
                         dungeon_feature_type feature)
{
    int x,y;

    if (feature != 0)
    {
        for(x = sx; x <= ex; x++)
        {
            for(y = sy; y <= ey; y++)
            {
                if (grd[x][y] == feature)
                    return (true);
            }
        }
    }

    return (false);
}

// stamp a box.  can avoid a possible type, and walls and floors can
// be different (or not stamped at all)
// Note that the box boundaries are INclusive.
static bool make_box(int room_x1, int room_y1, int room_x2, int room_y2,
                     dungeon_feature_type floor,
                     dungeon_feature_type wall,
                     dungeon_feature_type avoid)
{
    int bx,by;

    // check for avoidance
    if (find_in_area(room_x1, room_y1, room_x2, room_y2, avoid))
        return false;

    // draw walls
    if (wall != 0)
    {
        for(bx=room_x1; bx<=room_x2; bx++)
        {
            grd[bx][room_y1] = wall;
            grd[bx][room_y2] = wall;
        }
        for(by=room_y1+1; by<room_y2; by++)
        {
            grd[room_x1][by] = wall;
            grd[room_x2][by] = wall;
        }
    }

    // draw floor
    if (floor != 0)
    {
        for(bx=room_x1 + 1; bx < room_x2; bx++)
            for(by=room_y1 + 1; by < room_y2; by++)
                grd[bx][by] = floor;
    }

    return true;
}

// take care of labyrinth, abyss, pandemonium
// returns 1 if we should skip further generation,
// -1 if we should immediately quit, and 0 otherwise.
static builder_rc_type builder_by_type(int level_number, char level_type)
{
    if (level_type == LEVEL_PORTAL_VAULT)
    {
        portal_vault_level(level_number);
        return (BUILD_QUIT);
    }
    
    if (level_type == LEVEL_LABYRINTH)
    {
        labyrinth_level(level_number);
        return (BUILD_QUIT);
    }

    if (level_type == LEVEL_ABYSS)
    {
        generate_abyss();
        return (BUILD_SKIP);
    }

    if (level_type == LEVEL_PANDEMONIUM)
    {
        int which_demon = -1;
        // Could do spotty_level, but that doesn't always put all paired
        // stairs reachable from each other which isn't a problem in normal
        // dungeon but could be in Pandemonium
        if (one_chance_in(4))
        {
            do
            {
                which_demon = random2(4);

                // makes these things less likely as you find more
                if (one_chance_in(4))
                {
                    which_demon = -1;
                    break;
                }
            }
            while (you.unique_creatures[MONS_MNOLEG + which_demon]);
        }

        if (which_demon >= 0)
        {
            const char *pandemon_level_names[] =
            {
                "mnoleg", "lom_lobon", "cerebov", "gloorx_vloq"
            };

            const int vault = 
                random_map_for_tag(pandemon_level_names[which_demon], false);

            ASSERT(vault != -1);
            if (vault == -1)
                end(1, false, "Failed to find Pandemonium level %s!\n",
                    pandemon_level_names[which_demon]);

            ensure_vault_placed( build_vaults(level_number, vault) );
        }
        else
        {
            plan_main(level_number, 0);
            int vault = random_map_for_tag("pan", true);
            ASSERT( vault != -1 );
            if (vault == -1) 
                end(1, false, "Failed to build Pandemonium minivault!\n");
            
            build_minivaults(level_number, vault);
        }

        return BUILD_SKIP;
    }

    // must be normal dungeon
    return BUILD_CONTINUE;
}

static void portal_vault_level(int level_number)
{
    std::string trimmed_name = trimmed_string(you.level_type_name);
    ASSERT(!trimmed_name.empty());

    const char* level_name = trimmed_name.c_str();

    int vault = random_map_for_place(level_id::current(), false);

#ifdef WIZARD
    if (vault == -1 && you.wizard
        && random_map_for_tag(level_name, false) != -1)
    {
        char buf[80];

        do
        {
            mprf(MSGCH_PROMPT, "Which %s (ESC or ENTER for random): ",
                 level_name);
            if (cancelable_get_line(buf, sizeof buf))
                break;

            std::string name = buf;
            trim_string(name);

            if (name.empty())
                break;

            lowercase(name);
            name = replace_all(name, " ", "_");

            vault = find_map_by_name(you.level_type_name + "_" + name);

            if (vault == -1)
                mprf(MSGCH_DIAGNOSTICS, "No such %s, try again.",
                     level_name);
        } while (vault == -1);
    }
#endif

    if (vault == -1)
        vault = random_map_for_tag(level_name, false);

    if (vault != -1)
        ensure_vault_placed( build_vaults(level_number, vault) );
    else
    {
        plan_main(level_number, 0);
        place_minivaults(level_name, 1, 1, true);

        if (level_vaults.empty())
        {
            mprf(MSGCH_WARN, "No maps or tags named '%s'.",
                 level_name);
            ASSERT(false);
            end(-1);
        }
    }

    link_items();

    // TODO: Let portal vault map have arbitrary properties which can
    // be passed onto the callback.
    callback_map::const_iterator
        i = level_type_post_callbacks.find(you.level_type_name);

    if (i != level_type_post_callbacks.end())
        dlua.callfn(i->second.c_str(), 0, 0);
}

static int random_portal_vault(const std::string &tag)
{
    return random_map_for_tag(tag, false, true);
}

static bool place_portal_vault(int stair, const std::string &tag, int dlevel)
{
    const int vault = random_portal_vault(tag);
    if (vault == -1)
        return (false);

    return build_secondary_vault(dlevel, vault, stair);
}

static int dgn_random_map_for_place(bool wantmini)
{
    const level_id lid = level_id::current();
    int vault = random_map_for_place(lid, wantmini);
    
    // disallow entry vaults for tutorial (complicates things)
    if (vault == -1
        && lid.branch == BRANCH_MAIN_DUNGEON
        && lid.depth == 1 && !Options.tutorial_left)
    {
        vault = random_map_for_tag("entry", wantmini);
    }

    return (vault);
}

// returns BUILD_SKIP if we should skip further generation,
// BUILD_QUIT if we should immediately quit, and BUILD_CONTINUE
// otherwise.
static builder_rc_type builder_by_branch(int level_number)
{
    const int vault = dgn_random_map_for_place(false);

    if (vault != -1)
    {
        ensure_vault_placed( build_vaults(level_number, vault) );
        return BUILD_SKIP;
    }

    switch (you.where_are_you)
    {
    case BRANCH_LAIR:
        if (!one_chance_in(3))
            break;
    case BRANCH_HIVE:
    case BRANCH_SLIME_PITS:
    case BRANCH_ORCISH_MINES:
        spotty_level(false, 100 + random2(500), false);
        return BUILD_SKIP;

    default:
        break;
    }
    return BUILD_CONTINUE;
}

static void place_minivaults(const std::string &tag, int lo, int hi, bool force)
{
    const level_id curr = level_id::current();
    // Dungeon-style branches only, thankyouverymuch.
    if (curr.level_type != LEVEL_DUNGEON && !force)
        return;

    if (lo == -1)
        lo = hi = 1;
    
    int nvaults = random_range(lo, hi);
    if (!tag.empty())
    {
        for (int i = 0; i < nvaults; ++i)
        {
            const int vault = random_map_for_tag(tag, true);
            if (vault == -1)
                return;
            build_minivaults(you.your_level, vault);
        }
        return;
    }

    std::set<int> used;
    if (use_random_maps && minivault_chance && one_chance_in(minivault_chance))
    {
        const int vault = random_map_in_depth(level_id::current(), true);
        if (vault != -1)
        {
            build_minivaults(you.your_level, vault);
            used.insert(vault);
        }
    }
    
    int chance = you.your_level == 0? 50 : 100;
    while ((chance && random2(100) < chance) || nvaults-- > 0)
    {
        const int vault = dgn_random_map_for_place(true);
        if (vault == -1)
            break;

        // If we've already used this minivault and it doesn't want duplicates,
        // break.
        if (used.find(vault) != used.end()
            && !map_by_index(vault)->has_tag("allow_dup"))
        {
            break;
        }
        
        build_minivaults(you.your_level, vault);
        used.insert(vault);
        chance /= 4;
    }
}

// returns 1 if we should dispense with city building,
// 0 otherwise.  Also sets special_room if one is generated
// so that we can link it up later.
static builder_rc_type builder_normal(int level_number, char level_type,
                                      spec_room &sr)
{
    UNUSED( level_type );

    bool skipped = false;
    bool done_city = false;

    int vault = dgn_random_map_for_place(false); 

    // Can't have vaults on you.where_are_you != BRANCH_MAIN_DUNGEON levels
    if (vault == -1
        && use_random_maps
        && one_chance_in(vault_chance))
    {
        vault = random_map_in_depth(level_id::current());

        // We'll accept any kind of primary vault in the main dungeon,
        // but only ORIENT: encompass primary vaults in other
        // branches. Other kinds of vaults can still be placed in
        // other branches as secondary vaults.
        // 
        if (vault != -1 && !player_in_branch(BRANCH_MAIN_DUNGEON)
            && map_by_index(vault)->orient != MAP_ENCOMPASS)
        {
            vault = -1;
        }
    }

    if (vault != -1)
    {
        ensure_vault_placed( build_vaults(level_number, vault) );
        return BUILD_SKIP;
    }

    if (player_in_branch( BRANCH_DIS ))
    {
        city_level(level_number);
        return BUILD_SKIP;
    }

    if (player_in_branch( BRANCH_VAULTS ))
    {
        if (one_chance_in(3))
            city_level(level_number);
        else
            plan_main(level_number, 4);
        return BUILD_SKIP;
    }

    if (level_number > 7 && level_number < 23)
    {
        if (one_chance_in(16))
        {
            spotty_level(false, 0, coinflip());
            return BUILD_SKIP;
        }

        if (one_chance_in(16))
        {
            bigger_room();
            return BUILD_SKIP;
        }
    }

    if (level_number > 2 && level_number < 23 && one_chance_in(3))
    {
        plan_main(level_number, 0);
        minivault_chance = 3;
        return BUILD_SKIP;
    }

    if (one_chance_in(3))
        skipped = true;

    //V was 3
    if (!skipped && one_chance_in(7))
    {
        // sometimes roguey_levels generate a special room
        roguey_level(level_number, sr);
        minivault_chance = 4;
    }
    else
    {
        if (!skipped && level_number > 13 && one_chance_in(8))
        {
            if (one_chance_in(3))
                city_level(level_number);
            else
                plan_main(level_number, 4);
            done_city = true;
        }
    }

    // maybe create a special room, if roguey_level hasn't done it
    // already.
    if (!sr.created && level_number > 5 && !done_city && one_chance_in(5))
        special_room(level_number, sr);

    return BUILD_CONTINUE;
}

// returns 1 if we should skip extras(), otherwise 0
static builder_rc_type builder_basic(int level_number)
{
    int temp_rand;
    int doorlevel = random2(11);
    int corrlength = 2 + random2(14);
    int roomsize = 4 + random2(5) + random2(6);
    int no_corr = (one_chance_in(100) ? 500 + random2(500) : 30 + random2(200));
    int intersect_chance = (one_chance_in(20) ? 400 : random2(20));

    make_trail( 35, 30, 35, 20, corrlength, intersect_chance, no_corr,
                DNGN_STONE_STAIRS_DOWN_I, DNGN_STONE_STAIRS_UP_I );

    make_trail( 10, 15, 10, 15, corrlength, intersect_chance, no_corr,
                DNGN_STONE_STAIRS_DOWN_II, DNGN_STONE_STAIRS_UP_II );

    make_trail(50,20,10,15,corrlength,intersect_chance,no_corr,
        DNGN_STONE_STAIRS_DOWN_III, DNGN_STONE_STAIRS_UP_III);

    if (one_chance_in(4))
    {
        make_trail( 10, 20, 40, 20, corrlength, intersect_chance, no_corr,
                    DNGN_ROCK_STAIRS_DOWN );
    }

    if (one_chance_in(4))
    {
        make_trail( 50, 20, 40, 20, corrlength, intersect_chance, no_corr,
                    DNGN_ROCK_STAIRS_UP );
    }


    if (level_number > 1 && one_chance_in(16))
        big_room(level_number);

    if (random2(level_number) > 6 && one_chance_in(3))
        diamond_rooms(level_number);

    // make some rooms:
    int i, no_rooms, max_doors;
    int sx,sy,ex,ey, time_run;

    temp_rand = random2(750);
    time_run = 0;

    no_rooms = ((temp_rand > 63) ? (5 + random2avg(29, 2)) : // 91.47% {dlb}
                (temp_rand > 14) ? 100                       //  6.53% {dlb}
                                 : 1);                       //  2.00% {dlb}

    max_doors = 2 + random2(8);

    for (i = 0; i < no_rooms; i++)
    {
        sx = 8 + random2(50);
        sy = 8 + random2(40);
        ex = sx + 2 + random2(roomsize);
        ey = sy + 2 + random2(roomsize);

        if (!make_room(sx,sy,ex,ey,max_doors, doorlevel))
        {
            time_run++;
            i--;
        }

        if (time_run > 30)
        {
            time_run = 0;
            i++;
        }
    }

    // make some more rooms:
    no_rooms = 1 + random2(3);
    max_doors = 1;

    for (i = 0; i < no_rooms; i++)
    {
        sx = 8 + random2(55);
        sy = 8 + random2(45);
        ex = sx + 5 + random2(6);
        ey = sy + 5 + random2(6);

        if (!make_room(sx,sy,ex,ey,max_doors, doorlevel))
        {
            time_run++;
            i--;
        }

        if (time_run > 30)
        {
            time_run = 0;
            i++;
        }
    }

    return BUILD_CONTINUE;
}

static void builder_extras( int level_number, int level_type )
{
    UNUSED( level_type );

    if (one_chance_in(15))
        place_specific_stair(DNGN_ENTER_LABYRINTH, "lab_entry",
                             level_number, true);

    if (level_number > 6 && one_chance_in(10))
    {
        many_pools( level_number < 11 || coinflip() ?
                    DNGN_DEEP_WATER : DNGN_LAVA );
        return;
    }

    //mv: it's better to be here so other dungeon features
    // are not overriden by water
    dungeon_feature_type river_type =
        one_chance_in( 5 + level_number ) ? DNGN_SHALLOW_WATER 
        : DNGN_DEEP_WATER;

    if (level_number > 11 
        && (one_chance_in(5) || (level_number > 15 && !one_chance_in(5))))
    {
        river_type = DNGN_LAVA;
    }

    if (player_in_branch( BRANCH_GEHENNA ))
    {
        river_type = DNGN_LAVA;

        if (coinflip())
            build_river( river_type );
        else  
            build_lake( river_type );
    }
    else if (player_in_branch( BRANCH_COCYTUS ))
    {
        river_type = DNGN_DEEP_WATER;

        if (coinflip())
            build_river( river_type );
        else  
            build_lake( river_type );
    }

    if (level_number > 8 && one_chance_in(16))
        build_river( river_type );
    else if (level_number > 8 && one_chance_in(12))
    {
        build_lake( (river_type != DNGN_SHALLOW_WATER) ? river_type
                                                       : DNGN_DEEP_WATER );
    } 
}

static void place_traps(int level_number)
{
    int i;
    int num_traps = num_traps_for_place(level_number);

    ASSERT(num_traps >= 0);
    ASSERT(num_traps <= MAX_TRAPS);

    for (i = 0; i < num_traps; i++)
    {
        // traps can be placed in vaults
        if (env.trap[i].type != TRAP_UNASSIGNED)
            continue;

        int tries = 200;
        do
        {
            env.trap[i].x = random2(GXM);
            env.trap[i].y = random2(GYM);
        }
        while (grd[env.trap[i].x][env.trap[i].y] != DNGN_FLOOR
               && --tries > 0);

        if (tries <= 0)
            break;

        trap_type &trap_type = env.trap[i].type;
        trap_type = random_trap_for_place(level_number);

        grd[env.trap[i].x][env.trap[i].y] = DNGN_UNDISCOVERED_TRAP;
    }                           // end "for i"
}                               // end place_traps()

static void place_fog_machines(int level_number)
{
    int i;
    int num_fogs = num_fogs_for_place(level_number);

    ASSERT(num_fogs >= 0);

    for (i = 0; i < num_fogs; i++)
    {
        fog_machine_data data = random_fog_for_place(level_number);

        if (!valid_fog_machine_data(data))
        {
            mpr("Invalid fog machine data, bailing.", MSGCH_DIAGNOSTICS);
            return;
        }

        int tries = 200;
        int x, y;
        dungeon_feature_type feat;
        do
        {
            x = random2(GXM);
            y = random2(GYM);
            feat = grd[x][y];
        }
        while (feat <= DNGN_MAXWALL && --tries > 0);

        if (tries <= 0)
            break;

        place_fog_machine(data, x, y);
    }                           // end "for i"
}                               // end place_traps()

static void place_specific_feature(dungeon_feature_type feat)
{
    int sx, sy;

    do
    {
        sx = random_range(X_BOUND_1 + 1, X_BOUND_2 - 1);
        sy = random_range(Y_BOUND_1 + 1, Y_BOUND_2 - 1);
    }
    while(grd[sx][sy] != DNGN_FLOOR || mgrd[sx][sy] != NON_MONSTER);

    grd[sx][sy] = feat;
}

static void place_specific_stair(dungeon_feature_type stair,
                                 const std::string &tag,
                                 int dlevel,
                                 bool vault_only)
{
    if ((tag.empty() || !place_portal_vault(stair, tag, dlevel))
        && !vault_only)
    {
        place_specific_feature(stair);
    }
}

static void place_extra_vaults()
{
    if (!player_in_branch(BRANCH_MAIN_DUNGEON)
        && use_random_maps
        && vault_chance
        && one_chance_in(vault_chance))
    {
        int vault = random_map_in_depth(level_id::current());

        // ORIENT: encompass maps are unsuitable as secondary vaults.
        if (vault != -1 && map_by_index(vault)->orient == MAP_ENCOMPASS)
            vault = -1;
        
        if (vault != -1 && build_secondary_vault(you.your_level, vault, -1))
            vault_chance = 0;
    }
}

static void place_branch_entrances(int dlevel, char level_type)
{
    int sx, sy;

    if (level_type != LEVEL_DUNGEON)
        return;

    if (player_in_branch( BRANCH_MAIN_DUNGEON ))
    {
        // stair to HELL
        if (dlevel >= 20 && dlevel <= 27)
            place_specific_stair(DNGN_ENTER_HELL, "hell_entry", dlevel);

        // stair to PANDEMONIUM
        if (dlevel >= 20 && dlevel <= 50 && (dlevel == 23 || one_chance_in(4)))
            place_specific_stair(DNGN_ENTER_PANDEMONIUM, "pan_entry", dlevel);

        // stairs to ABYSS
        if (dlevel >= 20 && dlevel <= 30 && (dlevel == 24 || one_chance_in(3)))
            place_specific_stair(DNGN_ENTER_ABYSS, "abyss_entry", dlevel);

        // level 26: replaces all down stairs with staircases to Zot:
        if (dlevel == 26)
        {
            for (sx = 1; sx < GXM; sx++)
            {
                for (sy = 1; sy < GYM; sy++)
                {
                    if (grd[sx][sy] >= DNGN_STONE_STAIRS_DOWN_I
                            && grd[sx][sy] <= DNGN_ROCK_STAIRS_DOWN)
                    {
                        grd[sx][sy] = DNGN_ENTER_ZOT;
                    }
                }
            }
        }
    }

    // place actual branch entrances
    for (int i = 0; i < NUM_BRANCHES; ++i)
    {
        if ( branches[i].entry_stairs != NUM_FEATURES &&
             player_in_branch(branches[i].parent_branch) &&
             player_branch_depth() == branches[i].startdepth )
        {
            // place a stair
#ifdef DEBUG_DIAGNOSTICS
            mprf(MSGCH_DIAGNOSTICS, "Placing stair to %s",
                 branches[i].shortname);
#endif

            std::string entry_tag = std::string(branches[i].abbrevname);
            entry_tag += "_entry";
            lowercase(entry_tag);
            
            place_specific_stair(
                branches[i].entry_stairs,
                entry_tag,
                dlevel);
        }
    }
}

static void make_trail(int xs, int xr, int ys, int yr, int corrlength, 
                       int intersect_chance, int no_corr,
                       dungeon_feature_type begin, 
                       dungeon_feature_type end)
{
    int x_start, y_start;                   // begin point
    int x_ps, y_ps;                         // end point
    int finish = 0;
    int length = 0;
    int temp_rand;

    // temp positions
    int dir_x = 0;
    int dir_y = 0;       
    int dir_x2, dir_y2; 

    do
    {
        x_start = xs + random2(xr);
        y_start = ys + random2(yr);
    }
    while (grd[x_start][y_start] != DNGN_ROCK_WALL
                && grd[x_start][y_start] != DNGN_FLOOR);

    // assign begin feature
    if (begin != DNGN_UNSEEN)
        grd[x_start][y_start] = begin;
    x_ps = x_start;
    y_ps = y_start;

    // wander
    do                          // (while finish < no_corr)
    {
        dir_x2 = ((x_ps < 15) ? 1 : 0);

        if (x_ps > 65)
            dir_x2 = -1;

        dir_y2 = ((y_ps < 15) ? 1 : 0);

        if (y_ps > 55)
            dir_y2 = -1;

        temp_rand = random2(10);

        // Put something in to make it go to parts of map it isn't in now
        if (coinflip())
        {
            if (dir_x2 != 0 && temp_rand < 6)
                dir_x = dir_x2;

            if (dir_x2 == 0 || temp_rand >= 6)
                dir_x = (coinflip()? -1 : 1);

            dir_y = 0;
        }
        else
        {
            if (dir_y2 != 0 && temp_rand < 6)
                dir_y = dir_y2;

            if (dir_y2 == 0 || temp_rand >= 6)
                dir_y = (coinflip()? -1 : 1);

            dir_x = 0;
        }

        if (dir_x == 0 && dir_y == 0)
            continue;

        if (x_ps < X_BOUND_1 + 3)
        {
            dir_x = 1;
            dir_y = 0;
        }

        if (y_ps < Y_BOUND_1 + 3)
        {
            dir_y = 1;
            dir_x = 0;
        }

        if (x_ps > (X_BOUND_2 - 3))
        {
            dir_x = -1;
            dir_y = 0;
        }

        if (y_ps > (Y_BOUND_2 - 3))
        {
            dir_y = -1;
            dir_x = 0;
        }

        // corridor length.. change only when going vertical?
        if (dir_x == 0 || length == 0)
            length = random2(corrlength) + 2;

        int bi = 0;

        for (bi = 0; bi < length; bi++)
        {
            // Below, I've changed the values of the unimportant variable from
            // 0 to random2(3) - 1 to avoid getting stuck on the "stuck!" bit
            if (x_ps < X_BOUND_1 + 4)
            {
                dir_y = 0;      //random2(3) - 1;
                dir_x = 1;
            }

            if (x_ps > (X_BOUND_2 - 4))
            {
                dir_y = 0;      //random2(3) - 1;
                dir_x = -1;
            }

            if (y_ps < Y_BOUND_1 + 4)
            {
                dir_y = 1;
                dir_x = 0;      //random2(3) - 1;
            }

            if (y_ps > (Y_BOUND_2 - 4))
            {
                dir_y = -1;
                dir_x = 0;      //random2(3) - 1;
            }

            // don't interfere with special rooms
            if (grd[x_ps + dir_x][y_ps + dir_y] == DNGN_BUILDER_SPECIAL_WALL)
                break;

            // see if we stop due to intersection with another corridor/room
            if (grd[x_ps + 2 * dir_x][y_ps + 2 * dir_y] == DNGN_FLOOR
                && !one_chance_in(intersect_chance))
                break;

            x_ps += dir_x;
            y_ps += dir_y;

            if (grd[x_ps][y_ps] == DNGN_ROCK_WALL)
                grd[x_ps][y_ps] = DNGN_FLOOR;
        }

        if (finish == no_corr - 1 && grd[x_ps][y_ps] != DNGN_FLOOR)
            finish -= 2;

        finish++;
    }
    while (finish < no_corr);

    // assign end feature
    if (end != DNGN_UNSEEN)
        grd[x_ps][y_ps] = end;
}

static int good_door_spot(int x, int y)
{
    if ((!grid_is_solid(grd[x][y]) && grd[x][y] < DNGN_ENTER_PANDEMONIUM)
        || grd[x][y] == DNGN_CLOSED_DOOR)
    {
        return 1;
    }

    return 0;
}

// return TRUE if a room was made successfully
static bool make_room(int sx,int sy,int ex,int ey,int max_doors, int doorlevel)
{
    int find_door = 0;
    int diag_door = 0;
    int rx,ry;

    // check top & bottom for possible doors
    for (rx = sx; rx <= ex; rx++)
    {
        find_door += good_door_spot(rx,sy);
        find_door += good_door_spot(rx,ey);
    }

    // check left and right for possible doors
    for (ry = sy+1; ry < ey; ry++)
    {
        find_door += good_door_spot(sx,ry);
        find_door += good_door_spot(ex,ry);
    }

    diag_door += good_door_spot(sx,sy);
    diag_door += good_door_spot(ex,sy);
    diag_door += good_door_spot(sx,ey);
    diag_door += good_door_spot(ex,ey);

    if ((diag_door + find_door) > 1 && max_doors == 1)
        return false;

    if (find_door == 0 || find_door > max_doors)
        return false;

    // look for 'special' rock walls - don't interrupt them
    if (find_in_area(sx,sy,ex,ey,DNGN_BUILDER_SPECIAL_WALL))
        return false;

    // convert the area to floor
    for (rx=sx; rx<=ex; rx++)
    {
        for(ry=sy; ry<=ey; ry++)
        {
            if (grd[rx][ry] <= DNGN_FLOOR)
                grd[rx][ry] = DNGN_FLOOR;
        }
    }

    // put some doors on the sides (but not in corners),
    // where it makes sense to do so.
    for(ry=sy+1; ry<ey; ry++)
    {
        // left side
        if (grd[sx-1][ry] == DNGN_FLOOR
            && grid_is_solid(grd[sx-1][ry-1])
            && grid_is_solid(grd[sx-1][ry+1]))
        {
            if (random2(10) < doorlevel)
                grd[sx-1][ry] = DNGN_CLOSED_DOOR;
        }

        // right side
        if (grd[ex+1][ry] == DNGN_FLOOR
            && grid_is_solid(grd[ex+1][ry-1])
            && grid_is_solid(grd[ex+1][ry+1]))
        {
            if (random2(10) < doorlevel)
                grd[ex+1][ry] = DNGN_CLOSED_DOOR;
        }
    }

    // put some doors on the top & bottom
    for(rx=sx+1; rx<ex; rx++)
    {
        // top
        if (grd[rx][sy-1] == DNGN_FLOOR
            && grid_is_solid(grd[rx-1][sy-1])
            && grid_is_solid(grd[rx+1][sy-1]))
        {
            if (random2(10) < doorlevel)
                grd[rx][sy-1] = DNGN_CLOSED_DOOR;
        }

        // bottom
        if (grd[rx][ey+1] == DNGN_FLOOR
            && grid_is_solid(grd[rx-1][ey+1])
            && grid_is_solid(grd[rx+1][ey+1]))
        {
            if (random2(10) < doorlevel)
                grd[rx][ey+1] = DNGN_CLOSED_DOOR;
        }
    }

    return true;
}                               //end make_room()

static int pick_unique(int lev)
{
    int which_unique =
        ((lev > 19) ? random_range(MONS_LOUISE, MONS_BORIS) :
         (lev > 16) ? random_range(MONS_ERICA, MONS_FRANCES) :
         (lev > 13) ? random_range(MONS_URUG, MONS_JOZEF) :
         (lev >  9) ? random_range(MONS_PSYCHE, MONS_MICHAEL) :
         (lev >  7) ? random_range(MONS_BLORK_THE_ORC, MONS_EROLCHA) :
         (lev >  3) ? random_range(MONS_IJYB, MONS_EDMUND) :
                      random_range(MONS_TERENCE, MONS_SIGMUND));

    if (player_in_branch(BRANCH_VESTIBULE_OF_HELL) && one_chance_in(7))
        which_unique = MONS_MURRAY;

    if (player_in_branch(BRANCH_HALL_OF_ZOT) && one_chance_in(3))
        which_unique = MONS_TIAMAT;

    if (player_in_branch(BRANCH_SHOALS) && player_branch_depth() > 1 &&
        coinflip())
        which_unique = MONS_POLYPHEMUS;
        
    return (which_unique);
}

// Place uniques on the level.
// There is a hidden dependency on the player's actual
// location (through your_branch().)
// Return the number of uniques placed.
static int place_uniques(int level_number, char level_type)
{
    int not_used = 0;
    // Unique beasties:
    if (level_number <= 0 || level_type != LEVEL_DUNGEON ||
        !your_branch().has_uniques)
        return 0;

    int num_placed = 0;

    while(one_chance_in(3))
    {
        int which_unique = -1;   //     30 in total

        while(which_unique < 0 || you.unique_creatures[which_unique])
        {
            // sometimes, we just quit if a unique is already placed.
            if (which_unique >= 0 && !one_chance_in(3))
            {
                which_unique = -1;
                break;
            }

            which_unique = pick_unique(level_number);
        }

        // usually, we'll have quit after a few tries. Make sure we don't
        // create unique[-1] by accident.
        if (which_unique == -1)
            break;

        // note: unique_creatures 40 + used by unique demons
        if (place_monster( not_used, which_unique, level_number, 
                           BEH_SLEEP, MHITNOT, false, 1, 1, true,
                           PROX_ANYWHERE, 250, 0, MMT_NO_MONS ))
        {
#ifdef DEBUG_DIAGNOSTICS
            mprf(MSGCH_DIAGNOSTICS, "Placed %s",
                 menv[not_used].name(DESC_NOCAP_A).c_str());
#endif
            ++num_placed;
        }
    }
    return num_placed;
}

static int place_monster_vector(std::vector<int> montypes,
                                int level_number, int num_to_place)
{
    int result = 0;
    int not_used = 0;
    for (int i = 0; i < num_to_place; i++)
    {
        if (place_monster( not_used, montypes[random2(montypes.size())],
                           level_number, BEH_SLEEP, MHITNOT, 
                           false, 1, 1, true, PROX_ANYWHERE, 250, 0,
                           MMT_NO_MONS ))
        {
            ++result;
        }
    }
    return result;
}
                                

static void place_aquatic_monsters(int level_number, char level_type)
{
    int lava_spaces = 0, water_spaces = 0;
    std::vector<int> swimming_things(4u, NON_MONSTER);

    // count the number of lava and water tiles {dlb}:
    for (int x = 0; x < GXM; x++)
    {
        for (int y = 0; y < GYM; y++)
        {
            if (grd[x][y] == DNGN_LAVA)
                lava_spaces++;
            if (grd[x][y]==DNGN_DEEP_WATER || grd[x][y]==DNGN_SHALLOW_WATER)
                water_spaces++;
        }
    }

    if (lava_spaces > 49 && level_number > 6)
    {
        for (int i = 0; i < 4; i++)
        {
            swimming_things[i] = MONS_LAVA_WORM + random2(3);
            if (one_chance_in(30)) 
                swimming_things[i] = MONS_SALAMANDER;
        }

        place_monster_vector(swimming_things, level_number,
                             std::min(random2avg(9, 2) +
                                      (random2(lava_spaces) / 10), 15));
    }

    if (water_spaces > 49)
    {
        for (int i = 0; i < 4; i++)
        {
            swimming_things[i] = MONS_BIG_FISH + random2(4);
            if (player_in_branch( BRANCH_SWAMP ) && !one_chance_in(3))
                swimming_things[i] = MONS_SWAMP_WORM;
            else if (player_in_branch( BRANCH_SHOALS ))
            {
                if (one_chance_in(3))
                    swimming_things[i] = MONS_MERFOLK;
                else if (one_chance_in(5))
                    swimming_things[i] = MONS_MERMAID;
            }
        }

        if (level_number >= 25 && one_chance_in(5))
            swimming_things[0] = MONS_WATER_ELEMENTAL;

        if (player_in_branch( BRANCH_COCYTUS ))
            swimming_things[3] = MONS_WATER_ELEMENTAL;

        place_monster_vector(swimming_things, level_number,
                             std::min(random2avg(9, 2) +
                                      (random2(water_spaces) / 10), 15));
    }
}


static void builder_monsters(int level_number, char level_type, int mon_wanted)
{
    int not_used = 0;
    if (level_type == LEVEL_PANDEMONIUM ||
        player_in_branch(BRANCH_ECUMENICAL_TEMPLE))
        return;

    for (int i = 0; i < mon_wanted; i++)
        place_monster( not_used, RANDOM_MONSTER, level_number, BEH_SLEEP,
                       MHITNOT, false, 1, 1, true, PROX_ANYWHERE, 250, 0,
                       MMT_NO_MONS );

    place_uniques(level_number, level_type);

    if ( !player_in_branch(BRANCH_CRYPT) ) // no water creatures in the Crypt
        place_aquatic_monsters(level_number, level_type);
    else
    {
        if (one_chance_in(3))
            mons_place( MONS_CURSE_SKULL, BEH_SLEEP, MHITNOT, false, 0, 0 );

        if (one_chance_in(7))
            mons_place( MONS_CURSE_SKULL, BEH_SLEEP, MHITNOT, false, 0, 0 );
    }
}

static void builder_items(int level_number, char level_type, int items_wanted)
{
    UNUSED( level_type );

    int i = 0;
    object_class_type specif_type = OBJ_RANDOM;
    int items_levels = level_number;
    int item_no;

    if (player_in_branch( BRANCH_VAULTS ))
    {
        items_levels *= 15;
        items_levels /= 10;
    }
    else if (player_in_branch( BRANCH_ORCISH_MINES ))
    {
        specif_type = OBJ_GOLD; /* lots of gold in the orcish mines */
    }

    if (player_in_branch( BRANCH_VESTIBULE_OF_HELL )
        || player_in_hell()
        || player_in_branch( BRANCH_SLIME_PITS )
        || player_in_branch( BRANCH_HALL_OF_BLADES )
        || player_in_branch( BRANCH_ECUMENICAL_TEMPLE ))
    {
        /* No items in hell, the slime pits, the Hall */
        return;
    }
    else
    {
        for (i = 0; i < items_wanted; i++)
            items( 1, specif_type, OBJ_RANDOM, false, items_levels, 250,
                   MMT_NO_ITEM );

        // Make sure there's a very good chance of a knife being placed
        // in the first five levels, but not a guarantee of one.  The
        // intent of this is to reduce the advantage that "cutting"
        // starting weapons have.  -- bwr
        if (player_in_branch( BRANCH_MAIN_DUNGEON )
            && level_number < 5 && coinflip())
        {
            item_no = items( 0, OBJ_WEAPONS, WPN_KNIFE, false, 0, 250,
                             MMT_NO_ITEM );

            // Guarantee that the knife is uncursed and non-special
            if (item_no != NON_ITEM)
            {
                mitm[item_no].plus = 0;
                mitm[item_no].plus2 = 0;
                mitm[item_no].flags = 0;   // no id, no race/desc, no curse
                mitm[item_no].special = 0; // no ego type
            }
        }
    }
}

// the entire intent of this function is to find a
// hallway from a special room to a floor space somewhere,
// changing the special room wall (DNGN_BUILDER_SPECIAL_WALL)
// to a closed door, and normal rock wall to pre-floor.
// Anything that might otherwise block the hallway is changed
// to pre-floor.
static void specr_2(spec_room &sr)
{
    int bkout = 0;
    int cx = 0, cy = 0;
    int sx = 0, sy = 0;
    int dx = 0, dy = 0;
    int i,j;

    // paranoia -- how did we get here if there's no actual special room??
    if (!sr.created)
        return;

  grolko:

    if (bkout > 100)
        return;

    switch (random2(4))
    {
    case 0:
        // go up from north edge
        cx = sr.x1 + (random2(sr.x2 - sr.x1));
        cy = sr.y1;
        dx = 0;
        dy = -1;
        break;
    case 1:
        // go down from south edge
        cx = sr.x1 + (random2(sr.x2 - sr.x1));
        cy = sr.y2;
        dx = 0;
        dy = 1;
        break;
    case 2:
        // go left from west edge
        cy = sr.y1 + (random2(sr.y2 - sr.y1));
        cx = sr.x1;
        dx = -1;
        dy = 0;
        break;
    case 3:
        // go right from east edge
        cy = sr.y1 + (random2(sr.y2 - sr.y1));
        cx = sr.x2;
        dx = 1;
        dy = 0;
        break;
    }

    sx = cx;
    sy = cy;

    for (i = 0; i < 100; i++)
    {
        sx += dx;
        sy += dy;

        // quit if we run off the map before finding floor
        if (!in_bounds(sx, sy))
        {
            bkout++;
            goto grolko;
        }

        // look around for floor
        if (i > 0)
        {
            if (grd[sx + 1][sy] == DNGN_FLOOR)
                break;
            if (grd[sx][sy + 1] == DNGN_FLOOR)
                break;
            if (grd[sx - 1][sy] == DNGN_FLOOR)
                break;
            if (grd[sx][sy - 1] == DNGN_FLOOR)
                break;
        }
    }

    sx = cx;
    sy = cy;

    for (j = 0; j < i + 2; j++)
    {
        if (grd[sx][sy] == DNGN_BUILDER_SPECIAL_WALL)
            grd[sx][sy] = DNGN_CLOSED_DOOR;

        if (j > 0 && grd[sx + dx][sy + dy] > DNGN_MINWALL
            && grd[sx + dx][sy + dy] < DNGN_FLOOR)
            grd[sx][sy] = DNGN_BUILDER_SPECIAL_FLOOR;

        if (grd[sx][sy] == DNGN_ROCK_WALL)
            grd[sx][sy] = DNGN_BUILDER_SPECIAL_FLOOR;

        sx += dx;
        sy += dy;
    }

    sr.hooked_up = true;
}                               // end specr_2()

// Fill special room sr with monsters from the pit_list at density%...
// then place a "lord of the pit" of lord_type at (lordx, lordy).
static void fill_monster_pit( spec_room &sr, 
                          FixedVector<pit_mons_def, MAX_PIT_MONSTERS> &pit_list,
                          int density, int lord_type, int lordx, int lordy )
{
    int i, x, y;

    // make distribution cumulative
    for (i = 1; i < MAX_PIT_MONSTERS; i++)
    {
        // assuming that the first zero rarity is the end of the list:
        if (!pit_list[i].rare)
            break;

        pit_list[i].rare = pit_list[i].rare + pit_list[i - 1].rare;
    }

    const int num_types = i;
    const int rare_sum = pit_list[num_types - 1].rare;

    // calculate die_size, factoring in the density% of the pit
    const int die_size = (rare_sum * 100) / density;

#if DEBUG_DIAGNOSTICS
    for (i = 0; i < num_types; i++)
    {
        const int delta = ((i > 0) ? pit_list[i].rare - pit_list[i - 1].rare 
                                   : pit_list[i].rare);

        const float perc = (static_cast<float>( delta ) * 100.0) 
                                / static_cast<float>( rare_sum );

        mprf( MSGCH_DIAGNOSTICS, "%6.2f%%: %s", perc,
              mons_type_name( pit_list[i].type, DESC_PLAIN).c_str() );
    }
#endif

    // put the boss monster down
    if (lord_type != MONS_PROGRAM_BUG)
        mons_place( lord_type, BEH_SLEEP, MHITNOT, true, lordx, lordy );

    // place monsters and give them items {dlb}:
    for (x = sr.x1; x <= sr.x2; x++)
    {
        for (y = sr.y1; y <= sr.y2; y++)
        {
            // avoid the boss (or anyone else we may have dropped already)
            if (mgrd[x][y] != NON_MONSTER)
                continue;

            const int roll = random2( die_size );

            // density skip (no need to iterate)
            if (roll >= rare_sum)
                continue;

            // run throught the cumulative chances and place a monster
            for (i = 0; i < num_types; i++)
            {
                if (roll < pit_list[i].rare)
                {
                    mons_place( pit_list[i].type, BEH_SLEEP, MHITNOT,
                                true, x, y );
                    break;
                }
            }
        }
    }
}

static void special_room(int level_number, spec_room &sr)
{
    char spec_room_type = SROOM_LAIR_KOBOLD;
    int lev_mons;
    int thing_created = 0;
    int x, y;

    object_class_type obj_type = OBJ_RANDOM;  // used in calling items() {dlb}
    unsigned char i;        // general purpose loop variable {dlb}
    int temp_rand = 0;          // probability determination {dlb}

    FixedVector < int, 10 > mons_alloc; // was [20] {dlb}

    char lordx = 0, lordy = 0;

    // overwrites anything;  this function better be called early on during
    // creation..
    int room_x1 = 8 + random2(55);
    int room_y1 = 8 + random2(45);
    int room_x2 = room_x1 + 4 + random2avg(6,2);
    int room_y2 = room_y1 + 4 + random2avg(6,2);

    // do special walls & floor
    make_box( room_x1, room_y1, room_x2, room_y2, 
              DNGN_BUILDER_SPECIAL_FLOOR, DNGN_BUILDER_SPECIAL_WALL );

    // set up passed in spec_room structure
    sr.created = true;
    sr.hooked_up = false;
    sr.x1 = room_x1 + 1;
    sr.x2 = room_x2 - 1;
    sr.y1 = room_y1 + 1;
    sr.y2 = room_y2 - 1;

    if (level_number < 7)
        spec_room_type = SROOM_LAIR_KOBOLD;
    else
    {
        spec_room_type = random2(NUM_SPECIAL_ROOMS);

        if (level_number < 23 && one_chance_in(4))
            spec_room_type = SROOM_BEEHIVE;

        if ((level_number > 13 && spec_room_type == SROOM_LAIR_KOBOLD)
            || (level_number < 16 && spec_room_type == SROOM_MORGUE)
            || (level_number < 14 && spec_room_type == SROOM_JELLY_PIT)
            || (level_number < 17 && one_chance_in(4)))
        {
            spec_room_type = SROOM_LAIR_ORC;
        }

        if (level_number > 19 && coinflip())
            spec_room_type = SROOM_MORGUE;

        if (level_number > 13 &&
            one_chance_in(6 - (level_number > 23) - (level_number > 18)))
        {
            spec_room_type = SROOM_JELLY_PIT;
        }
    }

    switch (spec_room_type)
    {
    case SROOM_LAIR_ORC:
        // determine which monster array to generate {dlb}:
        lev_mons = ((level_number > 24) ? 3 :
                    (level_number > 15) ? 2 :
                    (level_number >  9) ? 1
                                        : 0);

        // fill with baseline monster type {dlb}:
        for (i = 0; i < 10; i++)
        {
            mons_alloc[i] = MONS_ORC;
        }

        // fill in with special monster types {dlb}:
        switch (lev_mons)
        {
        case 0:
            mons_alloc[9] = MONS_ORC_WARRIOR;
            break;
        case 1:
            mons_alloc[8] = MONS_ORC_WARRIOR;
            mons_alloc[9] = MONS_ORC_WARRIOR;
            break;
        case 2:
            mons_alloc[6] = MONS_ORC_KNIGHT;
            mons_alloc[7] = MONS_ORC_WARRIOR;
            mons_alloc[8] = MONS_ORC_WARRIOR;
            mons_alloc[9] = MONS_OGRE;
            break;
        case 3:
            mons_alloc[2] = MONS_ORC_WARRIOR;
            mons_alloc[3] = MONS_ORC_WARRIOR;
            mons_alloc[4] = MONS_ORC_WARRIOR;
            mons_alloc[5] = MONS_ORC_KNIGHT;
            mons_alloc[6] = MONS_ORC_KNIGHT;
            mons_alloc[7] = MONS_OGRE;
            mons_alloc[8] = MONS_OGRE;
            mons_alloc[9] = MONS_TROLL;
            break;
        }

        // place monsters and give them items {dlb}:
        for (x = sr.x1; x <= sr.x2; x++)
        {
            for (y = sr.y1; y <= sr.y2; y++)
            {
                if (one_chance_in(4))
                    continue;

                mons_place( mons_alloc[random2(10)], BEH_SLEEP, MHITNOT, 
                            true, x, y );
            }
        }
        break;

    case SROOM_LAIR_KOBOLD:
        lordx = sr.x1 + random2(sr.x2 - sr.x1);
        lordy = sr.y1 + random2(sr.y2 - sr.y1);

        // determine which monster array to generate {dlb}:
        lev_mons = ((level_number < 4) ? 0 :
                    (level_number < 6) ? 1 : (level_number < 9) ? 2 : 3);

        // fill with baseline monster type {dlb}:
        for (i = 0; i < 10; i++)
        {
            mons_alloc[i] = MONS_KOBOLD;
        }

        // fill in with special monster types {dlb}:
        // in this case, they are uniformly the same {dlb}:
        for (i = (7 - lev_mons); i < 10; i++)
        {
            mons_alloc[i] = MONS_BIG_KOBOLD;
        }

        // place monsters and give them items {dlb}:
        for (x = sr.x1; x <= sr.x2; x++)
        {
            for (y = sr.y1; y <= sr.y2; y++)
            {
                if (one_chance_in(4))
                    continue;

                // we'll put the boss down later.
                if (x == lordx && y == lordy)
                    continue;

                mons_place( mons_alloc[random2(10)], BEH_SLEEP, MHITNOT,
                            true, x, y );
            }
        }

        // put the boss monster down
        mons_place( MONS_BIG_KOBOLD, BEH_SLEEP, MHITNOT, true, lordx, lordy );

        break;

    case SROOM_TREASURY:
        // should only appear in deep levels, with a guardian
        // Maybe have several types of treasure room?
        // place treasure {dlb}:
        for (x = sr.x1; x <= sr.x2; x++)
        {
            for (y = sr.y1; y <= sr.y2; y++)
            {
                temp_rand = random2(11);

                obj_type = ((temp_rand > 8) ? OBJ_WEAPONS :       // 2 in 11
                            (temp_rand > 6) ? OBJ_ARMOUR :        // 2 in 11
                            (temp_rand > 5) ? OBJ_MISSILES :      // 1 in 11
                            (temp_rand > 4) ? OBJ_WANDS :         // 1 in 11
                            (temp_rand > 3) ? OBJ_SCROLLS :       // 1 in 11
                            (temp_rand > 2) ? OBJ_JEWELLERY :     // 1 in 11
                            (temp_rand > 1) ? OBJ_BOOKS :         // 1 in 11
                            (temp_rand > 0) ? OBJ_STAVES          // 1 in 11
                                            : OBJ_POTIONS);       // 1 in 11

                thing_created = items( 1, obj_type, OBJ_RANDOM, true,
                                       level_number * 3, MAKE_ITEM_RANDOM_RACE);

                if (thing_created != NON_ITEM)
                {
                    mitm[thing_created].x = x;
                    mitm[thing_created].y = y;
                }
            }
        }

        // place guardian {dlb}:
        mons_place( MONS_GUARDIAN_NAGA, BEH_SLEEP, MHITNOT, true, 
                    sr.x1 + random2( sr.x2 - sr.x1 ), 
                    sr.y1 + random2( sr.y2 - sr.y1 ) );

        break;

    case SROOM_BEEHIVE:
        beehive(sr);
        break;

    case SROOM_MORGUE:
        morgue(sr);
        break;

    case SROOM_JELLY_PIT:
        jelly_pit(level_number, sr);
        break;
    }
}                               // end special_room()

// fills a special room with bees
static void beehive(spec_room &sr)
{
    int x,y;

    for (x = sr.x1; x <= sr.x2; x++)
    {
        for (y = sr.y1; y <= sr.y2; y++)
        {
            if (coinflip())
                continue;

            const int i = get_item_slot();
            if (i == NON_ITEM)
                continue;

            mitm[i].quantity = 1;
            mitm[i].base_type = OBJ_FOOD;
            mitm[i].sub_type = (one_chance_in(25) ? FOOD_ROYAL_JELLY
                                                  : FOOD_HONEYCOMB);
            mitm[i].x = x;
            mitm[i].y = y;

            item_colour( mitm[i] );
        }
    }


    const int queenx = sr.x1 + random2(sr.x2 - sr.x1);
    const int queeny = sr.y1 + random2(sr.y2 - sr.y1);

    for (x = sr.x1; x <= sr.x2; x++)
    {
        for (y = sr.y1; y <= sr.y2; y++)
        {
            if (x == queenx && y == queeny)
                continue;

            // the hive is chock full of bees!

            mons_place( one_chance_in(7) ? MONS_KILLER_BEE_LARVA 
                                         : MONS_KILLER_BEE,
                        BEH_SLEEP, MHITNOT, true, x, y );
        }
    }

    mons_place( MONS_QUEEN_BEE, BEH_SLEEP, MHITNOT, true, queenx, queeny );
}                               // end beehive()

static bool safe_minivault_place(int v1x, int v1y,
                                 const vault_placement &place,
                                 bool clobber)
{
    if (clobber)
        return (true);
    
    const bool water_ok = place.map.has_tag("water_ok");
    const std::vector<std::string> &lines = place.map.map.get_lines();
    for (int vx = v1x; vx < v1x + place.size.x; vx++)
    {
        for (int vy = v1y; vy < v1y + place.size.y; vy++)
        {
            if (lines[vy - v1y][vx - v1x] == ' ')
                continue;

            if (dgn_map_mask[vx][vy])
                return (false);

            const dungeon_feature_type dfeat = grd[vx][vy];

            if ((dfeat != DNGN_FLOOR
                 && dfeat != DNGN_ROCK_WALL
                 && dfeat != DNGN_CLOSED_DOOR
                 && dfeat != DNGN_SECRET_DOOR
                 && (!water_ok
                     || (dfeat != DNGN_DEEP_WATER
                         && dfeat != DNGN_SHALLOW_WATER)))
                || igrd[vx][vy] != NON_ITEM
                || mgrd[vx][vy] != NON_MONSTER)
            {
                return (false);
            }
        }
    }
    return (true);
}

static bool connected_minivault_place(int v1x, int v1y,
                                      const vault_placement &place)
{
    /* must not be completely isolated: */
    const bool water_ok = place.map.has_tag("water_ok");
    const std::vector<std::string> &lines = place.map.map.get_lines();
    for (int vx = v1x; vx < v1x + place.size.x; vx++)
    {
        for (int vy = v1y; vy < v1y + place.size.y; vy++)
        {
            if (lines[vy - v1y][vx - v1x] == ' ')
                continue;
            
            if (grd[vx][vy] == DNGN_FLOOR
                || grd[vx][vy] == DNGN_CLOSED_DOOR
                || grd[vx][vy] == DNGN_SECRET_DOOR
                || (water_ok
                    && (grd[vx][vy] == DNGN_SHALLOW_WATER ||
                        grd[vx][vy] == DNGN_DEEP_WATER)))
                return (true);
        }
    }
    return (false);
}

static bool find_minivault_place(const vault_placement &place,
                                 int &v1x, int &v1y, bool clobber)
{
    // [ds] The margin around the edges of the map where the minivault
    // won't be placed. Purely arbitrary as far as I can see.
    const int margin = MAPGEN_BORDER * 2;

    /* find a target area which can be safely overwritten: */
    for (int tries = 0; tries < 600; ++tries)
    {
        v1x = random_range( margin, GXM - margin - place.size.x );
        v1y = random_range( margin, GYM - margin - place.size.y );

        if (!safe_minivault_place( v1x, v1y, place, clobber ))
            continue;

        if (connected_minivault_place(v1x, v1y, place))
            return (true);
    }
    return (false);
}

static bool build_minivaults(int level_number, int force_vault,
                             bool building_level, bool clobber,
                             bool make_no_exits, const coord_def &where)
{
    // for some weird reason can't put a vault on level 1, because monster equip
    // isn't generated.
    int altar_count = 0;

    FixedVector < object_class_type, 7 > acq_item_class;
    // hack - passing chars through '...' promotes them to ints, which
    // barfs under gcc in fixvec.h.  So don't.
    acq_item_class[0] = OBJ_WEAPONS;
    acq_item_class[1] = OBJ_ARMOUR;
    acq_item_class[2] = OBJ_WEAPONS;
    acq_item_class[3] = OBJ_JEWELLERY;
    acq_item_class[4] = OBJ_BOOKS;
    acq_item_class[5] = OBJ_STAVES;
    acq_item_class[6] = OBJ_MISCELLANY;

    if (dgn_check_connectivity && !dgn_zones)
        dgn_zones = dgn_count_disconnected_zones();
    
    map_type vgrid;
    vault_placement place;
    vault_main(vgrid, place, force_vault);

    int v1x, v1y;

    if (in_bounds(where)) // not map_bounds, minivaults should never touch edge
    {
        coord_def tl(where - place.size / 2);
        fit_region_into_map_bounds(tl, place.size);
        v1x = tl.x;
        v1y = tl.y;
    }
    else if (!find_minivault_place(place, v1x, v1y, clobber))
        return (false);

    place.pos = coord_def(v1x, v1y);

    level_vaults.push_back(place);

#ifdef DEBUG_DIAGNOSTICS
    if (crawl_state.map_stat_gen)
        mapgen_report_map_use(place.map);
#endif

    register_place(place);
    
    // these two are throwaways:
    int num_runes = 0;

    std::vector<coord_def> &target_connections = place.exits;
    
    // paint the minivault onto the grid
    for (int vx = v1x; vx < v1x + place.size.x; vx++)
    {
        for (int vy = v1y; vy < v1y + place.size.y; vy++)
        {
            const int feat = vgrid[vy - v1y][vx - v1x];
            if (feat == ' ')
                continue;
            const dungeon_feature_type oldgrid = grd[vx][vy];
            altar_count = vault_grid( place,
                                      level_number, vx, vy,
                                      altar_count,
                                      acq_item_class,
                                      feat, target_connections,
                                      num_runes );
            if (!building_level)
            {
                link_items();
                const dungeon_feature_type newgrid = grd[vx][vy];
                grd[vx][vy] = oldgrid;
                dungeon_terrain_changed(coord_def(vx, vy),
                                        newgrid,
                                        true, true);
                env.markers.remove_markers_at(coord_def(vx, vy), MAT_ANY);
            }
        }
    }

    place.map.map.apply_overlays(coord_def(v1x, v1y));

    if (!make_no_exits)
    {
        if (target_connections.empty() && place.map.has_tag("mini_float"))
            pick_float_exits(place, target_connections);
        
        if (!target_connections.empty())
            connect_vault(place);
    }

    return (true);
}                               // end build_minivaults()

static void build_rooms(const dgn_region_list &excluded,
                        const std::vector<coord_def> &connections_needed,
                        int nrooms)
{
    int which_room = 0;
    const bool exclusive = !one_chance_in(10);

    // Where did this magic number come from?
    const int maxrooms = 30;
    dgn_region rom[maxrooms];

    std::vector<coord_def> connections = connections_needed;

    for (int i = 0; i < nrooms; i++)
    {
        dgn_region &myroom = rom[which_room];

        int overlap_tries = 200;
        do
        {
            myroom.size.set(3 + random2(8), 3 + random2(8));
            myroom.pos.set(
                random_range(MAPGEN_BORDER,
                             GXM - MAPGEN_BORDER - 1 - myroom.size.x),
                random_range(MAPGEN_BORDER,
                             GYM - MAPGEN_BORDER - 1 - myroom.size.y));
        }
        while (myroom.overlaps(excluded, dgn_map_mask) && overlap_tries-- > 0);

        if (overlap_tries < 0)
            continue;

        if (connections.size())
        {
            const coord_def c = connections[0];
            if (join_the_dots(c, myroom.random_edge_point(), MMT_VAULT))
                connections.erase( connections.begin() );
        }
        
        if (i > 0 && exclusive)
        {
            const coord_def end = myroom.end();
            bool found_collision = false;
            for (int cnx = myroom.pos.x - 1;
                 cnx < end.x && !found_collision;
                 cnx++)
            {
                for (int cny = myroom.pos.y - 1;
                     cny < end.y;
                     cny++)
                {
                    if (grd[cnx][cny] != DNGN_ROCK_WALL)
                    {
                        found_collision = true;
                        break;
                    }
                }
            }

            if (found_collision)
                continue;
        }

        const coord_def end = myroom.end();
        replace_area(myroom.pos.x, myroom.pos.y, end.x, end.y,
                     DNGN_ROCK_WALL, DNGN_FLOOR);

        if (which_room > 0)
        {
            join_the_dots_rigorous(
                myroom.random_edge_point(),
                rom[which_room - 1].random_edge_point(),
                MMT_VAULT );
        }

        which_room++;

        if (which_room >= maxrooms)
            break;
    }
}

static int away_from_edge(int x, int left_edge, int right_edge)
{
    if (x < left_edge)
        return (1);
    else if (x > right_edge)
        return (-1);
    else
        return (coinflip()? 1 : -1);
}

static coord_def dig_away_dir(const vault_placement &place,
                              const coord_def &pos)
{
    // Figure out which way we need to go to dig our way out of the vault.
    bool x_edge = 
        pos.x == place.pos.x || pos.x == place.pos.x + place.size.x - 1;
    bool y_edge =
        pos.y == place.pos.y || pos.y == place.pos.y + place.size.y - 1;

    // Handle exits in non-rectangular areas.
    if (!x_edge && !y_edge)
    {
        const coord_def rel = pos - place.pos;
        for (int yi = -1; yi <= 1; ++yi)
            for (int xi = -1; xi <= 1; ++xi)
            {
                if (!xi == !yi)
                    continue;

                const coord_def mv(rel.x + xi, rel.y + yi);
                if (!place.map.in_map(mv))
                    return (mv - rel);
            }
    }

    if (x_edge && y_edge)
    {
        if (coinflip())
            x_edge = false;
        else
            y_edge = false;
    }
        
    coord_def dig_dir;
    if (x_edge)
    {
        if (place.size.x == 1)
            dig_dir.x =
                away_from_edge(pos.x,
                               MAPGEN_BORDER * 2,
                               GXM - MAPGEN_BORDER * 2);
        else
            dig_dir.x = pos.x == place.pos.x? -1 : 1;
    }

    if (y_edge)
    {
        if (place.size.y == 1)
            dig_dir.y =
                away_from_edge(pos.y,
                               MAPGEN_BORDER * 2,
                               GYM - MAPGEN_BORDER * 2);
        else
            dig_dir.y = pos.y == place.pos.y? -1 : 1;
    }

    return (dig_dir);
}

static void dig_away_from(vault_placement &place, const coord_def &pos)
{
    coord_def dig_dir = dig_away_dir(place, pos);
    coord_def dig_at = pos;
    bool dug = false;
    for (int i = 0; i < GXM; i++)
    {
        dig_at += dig_dir;
        
        if (dig_at.x < MAPGEN_BORDER || dig_at.x > (GXM - MAPGEN_BORDER - 1)
            || dig_at.y < MAPGEN_BORDER
            || dig_at.y > (GYM - MAPGEN_BORDER - 1))
        {
            break;
        }

        if (grd(dig_at) == DNGN_ROCK_WALL)
        {
            grd(dig_at) = DNGN_FLOOR;
            dug = true;
        }
        else if (grd(dig_at) == DNGN_FLOOR && i > 0)
        {
            // If the floor square has at least two non-solid squares,
            // we're done.
            int adjacent_count = 0;

            for (int yi = -1; yi <= 1; ++yi)
            {
                for (int xi = -1; xi <= 1; ++xi)
                {
                    if (!xi && !yi)
                        continue;
                    if (!grid_is_solid(dig_at + coord_def(xi, yi))
                        && ++adjacent_count >= 2)
                    {
                        return;
                    }
                }
            }
        }
    }
}

static void dig_vault_loose(
    vault_placement &place,
    std::vector<coord_def> &targets)
{
    for (int i = 0, size = targets.size(); i < size; ++i)
        dig_away_from(place, targets[i]);
}

static bool grid_needs_exit(int x, int y)
{
    return (!grid_is_solid(x, y)
            || grd[x][y] == DNGN_CLOSED_DOOR
            || grd[x][y] == DNGN_SECRET_DOOR);
}

static bool map_grid_is_on_edge(const vault_placement &place,
                                const coord_def &c)
{
    for (int xi = c.x - 1; xi <= c.x + 1; ++xi)
        for (int yi = c.y - 1; yi <= c.y + 1; ++yi)
            if (!place.map.in_map(coord_def(xi, yi) - place.pos))
                return (true);
    return (false);
}

static void pick_internal_float_exits(const vault_placement &place,
                                      std::vector<coord_def> &exits)
{
    for (int y = place.pos.y + 1; y < place.pos.y + place.size.y - 1; ++y)
        for (int x = place.pos.x + 1; x < place.pos.x + place.size.x - 1; ++x)
            if (grid_needs_exit(x, y)
                && map_grid_is_on_edge(place, coord_def(x, y)))
            {
                exits.push_back( coord_def(x, y) );
            }
}

static void pick_float_exits(vault_placement &place,
                             std::vector<coord_def> &targets)
{
    std::vector<coord_def> possible_exits;
    for (int y = place.pos.y; y < place.pos.y + place.size.y; ++y)
    {
        if (grid_needs_exit(place.pos.x, y))
            possible_exits.push_back( coord_def(place.pos.x, y) );
        if (grid_needs_exit(place.pos.x + place.size.x - 1, y))
            possible_exits.push_back(
                coord_def(place.pos.x + place.size.x - 1, y) );
    }

    for (int x = place.pos.x + 1; x < place.pos.x + place.size.x - 1; ++x)
    {
        if (grid_needs_exit(x, place.pos.y))
            possible_exits.push_back( coord_def(x, place.pos.y) );
        if (grid_needs_exit(x, place.pos.y + place.size.y - 1))
            possible_exits.push_back(
                coord_def(x, place.pos.y + place.size.y - 1) );
    }

    pick_internal_float_exits(place, possible_exits);

    if (possible_exits.empty())
    {
#ifdef DEBUG_DIAGNOSTICS
        mprf(MSGCH_WARN, "Unable to find exit from %s", place.map.name.c_str());
#endif
        return;
    }

    const int npoints = possible_exits.size();
    int nexits = npoints < 6? npoints : npoints / 8 + 1;
    if (nexits > 10)
        nexits = 10;
    while (nexits-- > 0)
    {
        int which_exit = random2( possible_exits.size() );
        targets.push_back( possible_exits[which_exit] );
        possible_exits.erase( possible_exits.begin() + which_exit );
    }
}

static std::vector<coord_def> external_connection_points(
    const vault_placement &place,
    const std::vector<coord_def> &target_connections)
{
    std::vector<coord_def> ex_connection_points;
    
    // Giving target_connections directly to build_rooms causes
    // problems with long, skinny vaults where paths to the exit
    // tend to cut through the vault. By backing out of the vault
    // one square, we improve connectibility.
    for (int i = 0, size = target_connections.size(); i < size; ++i)
    {
        const coord_def &p = target_connections[i];
        ex_connection_points.push_back(p + dig_away_dir(place, p));
    }

    return (ex_connection_points);
}

static coord_def find_random_grid(int grid, unsigned mask)
{
    for (int i = 0; i < 100; ++i)
    {
        coord_def c( random_range(MAPGEN_BORDER,
                                  GXM - MAPGEN_BORDER - 1),
                     random_range(MAPGEN_BORDER,
                                  GYM - MAPGEN_BORDER - 1) );

        if (unforbidden(c, mask) && grd(c) == grid)
            return c;
    }
    return coord_def(0, 0);
}

static void connect_vault(const vault_placement &vp)
{
    std::vector<coord_def> exc = external_connection_points(vp, vp.exits);
    for (int i = 0, size = exc.size(); i < size; ++i)
    {
        const coord_def &p = exc[i];
        const coord_def floor = find_random_grid(DNGN_FLOOR, MMT_VAULT);

        if (!floor.x && !floor.y)
            continue;

        join_the_dots(p, floor, MMT_VAULT, true);
    }
}

static dungeon_feature_type dgn_find_rune_subst(const std::string &tag)
{
    const std::string suffix("_entry");
    const std::string::size_type psuffix = tag.find(suffix);
    if (psuffix == std::string::npos)
        return (DNGN_FLOOR);
    const std::string key = tag.substr(0, psuffix);
    if (key == "bzr")
        return (DNGN_ENTER_PORTAL_VAULT);
    else if (key == "lab")
        return (DNGN_ENTER_LABYRINTH);
    else if (key == "hell")
        return (DNGN_ENTER_HELL);
    else if (key == "pan")
        return (DNGN_ENTER_PANDEMONIUM);
    else if (key == "abyss")
        return (DNGN_ENTER_ABYSS);
    else
    {
        for (int i = 0; i < NUM_BRANCHES; ++i)
        {
            if (branches[i].entry_stairs != NUM_FEATURES
                && !strcasecmp(branches[i].abbrevname, key.c_str()))
            {
                return (branches[i].entry_stairs);
            }
        }
    }
    return (DNGN_FLOOR);
}

static dungeon_feature_type dgn_find_rune_subst_tags(const std::string &tags)
{
    std::vector<std::string> words = split_string(" ", tags);
    for (int i = 0, size = words.size(); i < size; ++i)
    {
        const dungeon_feature_type feat = dgn_find_rune_subst(words[i]);
        if (feat != DNGN_FLOOR)
            return (feat);
    }
    return (DNGN_FLOOR);
}

// Places a map on the current level (minivault or regular vault).
// 
// You can specify the centre of the map using "where" for floating vaults
// and minivaults. "where" is ignored for other vaults. XXX: it might be
// nice to specify a square that is not the centre, but is identified by
// a marker in the vault to be placed.
//
// NOTE: encompass maps will destroy the existing level!
// 
// generating_level: If true, assumes that this is in the middle of normal
//                   level generation, and does not link items or handle
//                   changing terrain.
// clobber: If true, assumes the newly placed vault can clobber existing
//          items and monsters (items may be destroyed, monsters may be
//          teleported).
bool dgn_place_map(int map, bool generating_level, bool clobber,
                   bool make_no_exits, const coord_def &where)
{
    const dgn_colour_override_manager colour_man;
    
    const map_def *mdef = map_by_index(map);
    bool did_map = false;
    bool fixup = false;

    if (mdef->orient == MAP_ENCOMPASS && !generating_level)
    {
        if (clobber)
        {
            // For encompass maps, clear the entire level.
            generating_level = true;
            fixup = true;
            reset_level();
            dungeon_events.clear();
        }
        else
        {
            mprf(MSGCH_DIAGNOSTICS,
                 "Cannot generate encompass map '%s' without clobber=true",
                 mdef->name.c_str());
            return (false);
        }
    }

    if (mdef->is_minivault())
        did_map =
            build_minivaults(you.your_level, map, generating_level, clobber,
                             make_no_exits, where);
    else
    {
        dungeon_feature_type rune_subst = DNGN_FLOOR;
        if (mdef->has_tag_suffix("_entry"))
            rune_subst = dgn_find_rune_subst_tags(mdef->tags);
        did_map = build_secondary_vault(you.your_level, map, rune_subst,
                                        generating_level, clobber,
                                        make_no_exits, where);
    }

    // Activate any markers within the map.
    if (did_map && !generating_level)
    {
        const vault_placement &vp = level_vaults[level_vaults.size() - 1];
        for (int y = vp.pos.y; y < vp.pos.y + vp.size.y; ++y)
        {
            for (int x = vp.pos.x; x < vp.pos.x + vp.size.x; ++x)
            {
                std::vector<map_marker *> markers =
                    env.markers.get_markers_at(coord_def(x, y));
                for (int i = 0, size = markers.size(); i < size; ++i)
                    markers[i]->activate();

                if (!see_grid(x, y))
                    set_terrain_changed(x, y);
            }
        }
    }

    if (fixup)
    {
        link_items();
        env.markers.activate_all();

        // Force teleport to place the player somewhere sane.
        you_teleport_now(false, false);
    }

    if (fixup || !generating_level)
        setup_environment_effects();
    return (did_map);
}

/*
 * Places a vault somewhere in an already built level if possible.
 * Returns true if the vault was successfully placed.
 */
static bool build_secondary_vault(int level_number, int vault,
                                  int rune_subst, bool generating_level,
                                  bool clobber, bool no_exits,
                                  const coord_def &where)
{
    if (build_vaults(level_number, vault, rune_subst, true, true,
                     generating_level, clobber, no_exits, where))
    {
        const vault_placement &vp = level_vaults[ level_vaults.size() - 1 ];
        connect_vault(vp);

        return (true);
    }
    return (false);
}

static bool build_vaults(int level_number, int force_vault, int rune_subst,
                         bool build_only, bool check_collisions,
                         bool generating_level, bool clobber,
                         bool make_no_exits, const coord_def &where)
{
    int altar_count = 0;
    FixedVector < char, 10 > stair_exist;
    char stx, sty;

    FixedVector < object_class_type, 7 > acq_item_class;
    // hack - passing chars through '...' promotes them to ints, which
    // barfs under gcc in fixvec.h.  So don't. -- GDL
    acq_item_class[0] = OBJ_WEAPONS;
    acq_item_class[1] = OBJ_ARMOUR;
    acq_item_class[2] = OBJ_WEAPONS;
    acq_item_class[3] = OBJ_JEWELLERY;
    acq_item_class[4] = OBJ_BOOKS;
    acq_item_class[5] = OBJ_STAVES;
    acq_item_class[6] = OBJ_MISCELLANY;

    if (dgn_check_connectivity && !dgn_zones)
        dgn_zones = dgn_count_disconnected_zones();
    
    map_type vgrid;
    vault_placement place;
    std::vector<coord_def> &target_connections = place.exits;

    if (map_bounds(where))
        place.pos = where;
    
    const int gluggy = vault_main(vgrid, place, force_vault,
                                  check_collisions, clobber);

    if (gluggy == MAP_NONE || !gluggy)
        return (false);

    int vx, vy;
    int  num_runes = 0;

    dgn_region this_vault(place.pos, place.size);
    // note: assumes *no* previous item (I think) or monster (definitely)
    // placement
    for (vx = place.pos.x; vx < place.pos.x + place.size.x; vx++)
    {
        for (vy = place.pos.y; vy < place.pos.y + place.size.y; vy++)
        {
            if (vgrid[vy][vx] == ' ')
                continue;

            const dungeon_feature_type oldgrid = grd[vx][vy];
            altar_count = vault_grid( place,
                                      level_number, vx, vy, altar_count, 
                                      acq_item_class,
                                      vgrid[vy][vx],
                                      target_connections,
                                      num_runes,
                                      rune_subst );
            if (!generating_level)
            {
                // Have to link items each square at a time, or
                // dungeon_terrain_changed could blow up.
                link_items();
                const dungeon_feature_type newgrid = grd[vx][vy];
                grd[vx][vy] = oldgrid;
                dungeon_terrain_changed(coord_def(vx, vy), newgrid,
                                        true, true);
                env.markers.remove_markers_at(coord_def(vx, vy), MAT_ANY);
            }
        }
    }

    place.map.map.apply_overlays(place.pos);
    register_place(place);

    if (gluggy == MAP_FLOAT && target_connections.empty())
        pick_float_exits(place, target_connections);

    if (make_no_exits)
        target_connections.clear();

    // Must do this only after target_connections is finalised, or the vault
    // exits will not be correctly set.
    level_vaults.push_back(place);

#ifdef DEBUG_DIAGNOSTICS
    if (crawl_state.map_stat_gen)
        mapgen_report_map_use(place.map);
#endif    

    // If the map takes the whole screen or we were only requested to
    // build the vault, our work is done.
    if (gluggy == MAP_ENCOMPASS || build_only)
        return (true);

    // Does this level require Dis treatment (metal wallification)?
    // XXX: Change this so the level definition can explicitly state what
    // kind of wallification it wants.
    const bool dis_wallify = place.map.has_tag("dis");

    const int v1x = place.pos.x;
    const int v1y = place.pos.y;
    const int v2x = place.pos.x + place.size.x - 1;
    const int v2y = place.pos.y + place.size.y - 1;

#ifdef DEBUG_DIAGNOSTICS
    mprf(MSGCH_DIAGNOSTICS,
            "Vault: (%d,%d)-(%d,%d); Dis: %s",
            v1x, v1y, v2x, v2y,
            dis_wallify? "yes" : "no");
#endif

    if (dis_wallify)
    {
        plan_4(v1x, v1y, v2x, v2y, DNGN_METAL_WALL);
    }
    else
    {
        dgn_region_list excluded_regions;
        excluded_regions.push_back(
            dgn_region::absolute(v1x, v1y, v2x, v2y));

        int nrooms = random_range(15, 90);
        
        // Try harder for floating vaults, which tend to complicate room
        // building somewhat.
        if (gluggy == MAP_FLOAT)
            nrooms += 10;

        std::vector<coord_def> ex_connection_points =
            external_connection_points(place, target_connections);
        
        build_rooms(excluded_regions, ex_connection_points, nrooms);

        // Excavate and connect the vault to the rest of the level.
        dig_vault_loose(place, target_connections);
    }

    unsigned char pos_x, pos_y;

    for (stx = 0; stx < 10; stx++)
        stair_exist[stx] = 0;

    for (stx = 0; stx < GXM; stx++)
    {
        for (sty = 0; sty < GYM; sty++)
        {
            if (grd[stx][sty] >= DNGN_STONE_STAIRS_DOWN_I
                    && grd[stx][sty] <= DNGN_ROCK_STAIRS_UP)
            {
                stair_exist[grd[stx][sty] - DNGN_STONE_STAIRS_DOWN_I] = 1;
            }
        }
    }

    if (player_in_branch( BRANCH_DIS ))
    {
        for (sty = 0; sty < 5; sty++)
            stair_exist[sty] = 1;

        for (sty = 6; sty < 10; sty++)
            stair_exist[sty] = 0;
    }

    // Don't create any new up stairs on dungeon level 1.
    bool no_up_stairs = player_branch_depth() == 1 && 
        you.level_type == LEVEL_DUNGEON;

    for (int j = 0; j < (coinflip()? 4 : 3); j++)
    {
        for (int i = 0; i < (no_up_stairs ? 1 : 2); i++)
        {
            const dungeon_feature_type stair =
                static_cast<dungeon_feature_type>(
                    j + ((i == 0) ? DNGN_STONE_STAIRS_DOWN_I
                         : DNGN_STONE_STAIRS_UP_I));

            if (stair_exist[stair - DNGN_STONE_STAIRS_DOWN_I] == 1)   
                continue;

            do
            {
                pos_x = random_range(X_BOUND_1 + 1, X_BOUND_2 - 1);
                pos_y = random_range(Y_BOUND_1 + 1, Y_BOUND_2 - 1);
            }
            while (grd[pos_x][pos_y] != DNGN_FLOOR
                   || (pos_x >= v1x && pos_x <= v2x && pos_y >= v1y
                       && pos_y <= v2y));

            grd[pos_x][pos_y] = stair;
        }
    }

    return (true);
}                               // end build_vaults()

static void dgn_place_item_explicit(const item_spec &spec,
                                     int x, int y, int level)
{
    // Dummy object?
    if (spec.base_type == OBJ_UNASSIGNED)
        return;

    if (spec.level >= 0)
        level = spec.level;
    else
    {
        switch (spec.level)
        {
        case ISPEC_GOOD:
            level = 5 + level * 2;
            break;
        case ISPEC_SUPERB:
            level = MAKE_GOOD_ITEM;
            break;
        }
    }

    const int item_made =
        items( spec.allow_uniques, spec.base_type, spec.sub_type, true, 
               level, spec.race, 0, spec.ego );
    
    if (item_made != NON_ITEM && item_made != -1)
    {
        mitm[item_made].x = x;
        mitm[item_made].y = y;

        if (is_stackable_item(mitm[item_made]) && spec.qty > 0)
            mitm[item_made].quantity = spec.qty;
    }
}

static void dgn_place_multiple_items(item_list &list,
                                      int x, int y, int level)
{
    const int size = list.size();
    for (int i = 0; i < size; ++i)
        dgn_place_item_explicit(list.get_item(i), x, y, level);
}

static void dgn_place_item_explicit(int index, int x, int y,
                                     vault_placement &place,
                                     int level)
{
    item_list &sitems = place.map.items;
    
    if (index < 0 || index >= static_cast<int>(sitems.size()))
    {
        // Non-fatal, but we warn even in non-debug mode so there's incentive
        // to fix the problem.
        mprf(MSGCH_DIAGNOSTICS, "Map '%s' requested invalid item index: %d",
             place.map.name.c_str(), index);
        return;
    }

    const item_spec spec = sitems.get_item(index);
    dgn_place_item_explicit(spec, x, y, level);
}

static void dgn_give_mon_spec_items(mons_spec &mspec,
                                    const int mindex,
                                    const int mid,
                                    const int monster_level)
{
    monsters &mon(menv[mindex]);

    unwind_var<int> save_speedinc(mon.speed_increment);

    // Get rid of existing equipment.
    for (int i = 0; i < NUM_MONSTER_SLOTS; i++)
    {
        if (mon.inv[i] != NON_ITEM)
        {
            item_def &item(mitm[mon.inv[i]]);
            mon.unequip(item, i, 0, true);
            destroy_item(mon.inv[i], true);
            mon.inv[i] = NON_ITEM;
        }
    }

    item_make_species_type racial = MAKE_ITEM_RANDOM_RACE;

    if (mons_genus(mid) == MONS_ORC)
        racial = MAKE_ITEM_ORCISH;
    else if (mons_genus(mid) == MONS_ELF)
        racial = MAKE_ITEM_ELVEN;

    item_list &list = mspec.items;

    const int size = list.size();
    for (int i = 0; i < size; ++i)
    {
        item_spec spec = list.get_item(i);

        if (spec.base_type == OBJ_UNASSIGNED)
            continue;

        // Don't give monster a randart, and don't radnomly give
        // monster an ego item.
        if (spec.base_type == OBJ_ARMOUR || spec.base_type == OBJ_WEAPONS
            || spec.base_type == OBJ_MISSILES)
        {
            spec.allow_uniques = 0;
            if (spec.ego == 0)
                spec.ego = SP_FORBID_EGO;
        }

        // Gives orcs and elves appropriate racial gear, unless
        // otherwise specified.
        if (spec.race == MAKE_ITEM_RANDOM_RACE)
        {
            // But don't automatically give elves elven boots or
            // elven cloaks.
            if (racial != MAKE_ITEM_ELVEN || spec.base_type != OBJ_ARMOUR
                || (spec.sub_type != ARM_CLOAK
                    && spec.sub_type != ARM_BOOTS))
            {
                spec.race = racial;
            }
        }

        int item_level = monster_level;

        if (spec.level >= 0)
            item_level = spec.level;
        else
        {
            switch(spec.level)
            {
            case ISPEC_GOOD:
                item_level = 5 + item_level * 2;
                break;
            case ISPEC_SUPERB:
                item_level = MAKE_GOOD_ITEM;
                break;
            }
        }

        const int item_made =
            items( spec.allow_uniques, spec.base_type, spec.sub_type, true,
                   item_level, spec.race, 0, spec.ego );

        if (item_made != NON_ITEM && item_made != -1)
        {
            item_def &item(mitm[item_made]);
            mon.pickup_item(item, 0, true);
        }
    }
}
                                    

bool dgn_place_monster(mons_spec &mspec,
                       int monster_level, int vx, int vy,
                       bool generate_awake)
{
    if (mspec.mid != -1)
    {
        const int mid = mspec.mid;
        const bool m_generate_awake = generate_awake || mspec.generate_awake;

        const int mlev = mspec.mlevel;
        if (mlev)
        {
            if (mlev > 0)
                monster_level = mlev;
            else if (mlev == -8)
                monster_level = 4 + monster_level * 2;
            else if (mlev == -9)
                monster_level += 5;
        }
        
        if (mid != RANDOM_MONSTER && mid < NUM_MONSTERS)
        {
            if (mons_is_unique(mid) && you.unique_creatures[mid])
                return (false);
            
            const habitat_type habitat = mons_habitat(mid);
            if (habitat != HT_NORMAL)
                grd[vx][vy] = habitat2grid(habitat);
        }

        int mindex = NON_MONSTER;
        const bool placed =
            place_monster( mindex, mid, monster_level,
                           m_generate_awake? BEH_WANDER : BEH_SLEEP,
                           MHITNOT, true, vx, vy, false,
                           PROX_ANYWHERE, mspec.monnum);
        if (placed && mindex != -1 && mindex != NON_MONSTER)
        {
            if (mspec.colour != BLACK)
                menv[mindex].colour = mspec.colour;

            if (mspec.items.size() > 0)
                dgn_give_mon_spec_items(mspec, mindex, mid, monster_level);
        }
        return (placed);
    }
    return (false);    
}

static bool dgn_place_monster(
    const vault_placement &place,
    mons_spec &mspec,
    int monster_level,
    int vx, int vy)
{
    const bool generate_awake =
        mspec.generate_awake || place.map.has_tag("generate_awake");
    return dgn_place_monster(mspec, monster_level, vx, vy,
                             generate_awake);
}

static bool dgn_place_one_monster(
    const vault_placement &place,
    mons_list &mons,
    int monster_level,
    int vx, int vy)
{
    for (int i = 0, size = mons.size(); i < size; ++i)
    {
        mons_spec spec = mons.get_monster(i);
        if (dgn_place_monster(place, spec, monster_level, vx, vy))
        {
            return (true);
        }
    }
    return (false);
}

static monster_type random_evil_statue()
{
    switch (random2(3))
    {
    case 0: return MONS_ORANGE_STATUE;
    case 1: return MONS_SILVER_STATUE;
    case 2: return MONS_ICE_STATUE;
    }
    return (MONS_PROGRAM_BUG);
}

// Grr, keep this in sync with vault_grid.
dungeon_feature_type map_feature(map_def *map, const coord_def &c, int rawfeat)
{
    if (rawfeat == -1)
        rawfeat = map->glyph_at(c);

    if (rawfeat == ' ')
        return (NUM_FEATURES);
    
    keyed_mapspec *mapsp = map? map->mapspec_for_key(rawfeat) : NULL;
    if (mapsp)
    {
        const feature_spec f = mapsp->get_feat();
        if (f.feat >= 0)
            return static_cast<dungeon_feature_type>(f.feat);
        else if (f.glyph >= 0)
            return map_feature(NULL, c, rawfeat);
        else if (f.shop >= 0)
            return (DNGN_ENTER_SHOP);
        else if (f.trap >= 0)
            return (DNGN_UNDISCOVERED_TRAP);

        return (DNGN_FLOOR);
    }

    return ((rawfeat == 'x') ? DNGN_ROCK_WALL :
            (rawfeat == 'X') ? DNGN_PERMAROCK_WALL :
            (rawfeat == 'c') ? DNGN_STONE_WALL :
            (rawfeat == 'v') ? DNGN_METAL_WALL :
            (rawfeat == 'b') ? DNGN_GREEN_CRYSTAL_WALL :
            (rawfeat == 'a') ? DNGN_WAX_WALL :
            (rawfeat == 'm') ? DNGN_CLEAR_ROCK_WALL : 
            (rawfeat == 'n') ? DNGN_CLEAR_STONE_WALL :
            (rawfeat == 'o') ? DNGN_CLEAR_PERMAROCK_WALL :
            (rawfeat == '+') ? DNGN_CLOSED_DOOR :
            (rawfeat == '=') ? DNGN_SECRET_DOOR :
            (rawfeat == 'w') ? DNGN_DEEP_WATER :
            (rawfeat == 'W') ? DNGN_SHALLOW_WATER :
            (rawfeat == 'l') ? DNGN_LAVA :
            (rawfeat == '>') ? DNGN_ROCK_STAIRS_DOWN :
            (rawfeat == '<') ? DNGN_ROCK_STAIRS_UP :
            (rawfeat == '}') ? DNGN_STONE_STAIRS_DOWN_I :
            (rawfeat == '{') ? DNGN_STONE_STAIRS_UP_I :
            (rawfeat == ')') ? DNGN_STONE_STAIRS_DOWN_II :
            (rawfeat == '(') ? DNGN_STONE_STAIRS_UP_II :
            (rawfeat == ']') ? DNGN_STONE_STAIRS_DOWN_III :
            (rawfeat == '[') ? DNGN_STONE_STAIRS_UP_III :
            (rawfeat == 'A') ? DNGN_STONE_ARCH :
            (rawfeat == 'B') ? DNGN_ALTAR_ZIN : 
            (rawfeat == 'C') ? pick_an_altar() :   // f(x) elsewhere {dlb}
            (rawfeat == 'F') ? DNGN_GRANITE_STATUE :
            (rawfeat == 'I') ? DNGN_ORCISH_IDOL :
            (rawfeat == 'G') ? DNGN_GRANITE_STATUE :
            (rawfeat == 'T') ? DNGN_BLUE_FOUNTAIN :
            (rawfeat == 'U') ? DNGN_SPARKLING_FOUNTAIN :
            (rawfeat == 'V') ? DNGN_PERMADRY_FOUNTAIN :
            (rawfeat == '\0')? DNGN_ROCK_WALL :
            DNGN_FLOOR); // includes everything else
}

// returns altar_count - seems rather odd to me to force such a return
// when I believe the value is only used in the case of the ecumenical
// temple - oh, well... {dlb}
static int vault_grid( vault_placement &place,
                       int level_number,
                       int vx, int vy,
                       int altar_count,
                       FixedVector < object_class_type, 7 > &acq_item_class, 
                       int vgrid,
                       std::vector<coord_def> &targets,
                       int &num_runes,
                       int rune_subst,
                       bool following )
{
    int not_used;

    keyed_mapspec *mapsp = following? NULL : place.map.mapspec_for_key(vgrid);
    if (mapsp)
    {
        const feature_spec f = mapsp->get_feat();
        if (f.feat >= 0)
        {
            grd[vx][vy] = static_cast<dungeon_feature_type>( f.feat );
            vgrid = -1;
        }
        else if (f.glyph >= 0)
        {
            altar_count = vault_grid( place, level_number, vx, vy,
                                      altar_count, acq_item_class,
                                      f.glyph, targets, num_runes,
                                      rune_subst, true );
        }
        else if (f.shop >= 0)
            place_spec_shop(level_number, vx, vy, f.shop);
        else if (f.trap >= 0)
        {
            const trap_type trap =
                f.trap == TRAP_INDEPTH? random_trap_for_place(level_number)
                : static_cast<trap_type>(f.trap);
            
            place_specific_trap(vx, vy, trap);
        }
        else
            grd[vx][vy] = DNGN_FLOOR;

        mons_list &mons = mapsp->get_monsters();
        dgn_place_one_monster(place, mons, level_number, vx, vy);

        item_list &items = mapsp->get_items();
        dgn_place_multiple_items(items, vx, vy, level_number);

        return (altar_count);
    }

    if (vgrid == 'F' && one_chance_in(100))
    {
        vgrid = '.';
        place_monster( not_used, random_evil_statue(), 30, BEH_HOSTILE,
                       MHITNOT, true, vx, vy, false);
    }
    
    // first, set base tile for grids {dlb}:
    grd[vx][vy] =
                  ((vgrid == -1)  ? grd[vx][vy] : 
                   (vgrid == 'x') ? DNGN_ROCK_WALL :
                   (vgrid == 'X') ? DNGN_PERMAROCK_WALL :
                   (vgrid == 'c') ? DNGN_STONE_WALL :
                   (vgrid == 'v') ? DNGN_METAL_WALL :
                   (vgrid == 'b') ? DNGN_GREEN_CRYSTAL_WALL :
                   (vgrid == 'a') ? DNGN_WAX_WALL :
                   (vgrid == 'm') ? DNGN_CLEAR_ROCK_WALL : 
                   (vgrid == 'n') ? DNGN_CLEAR_STONE_WALL :
                   (vgrid == 'o') ? DNGN_CLEAR_PERMAROCK_WALL :
                   (vgrid == '+') ? DNGN_CLOSED_DOOR :
                   (vgrid == '=') ? DNGN_SECRET_DOOR :
                   (vgrid == 'w') ? DNGN_DEEP_WATER :
                   (vgrid == 'W') ? DNGN_SHALLOW_WATER :
                   (vgrid == 'l') ? DNGN_LAVA :
                   (vgrid == '>') ? DNGN_ROCK_STAIRS_DOWN :
                   (vgrid == '<') ? DNGN_ROCK_STAIRS_UP :
                   (vgrid == '}') ? DNGN_STONE_STAIRS_DOWN_I :
                   (vgrid == '{') ? DNGN_STONE_STAIRS_UP_I :
                   (vgrid == ')') ? DNGN_STONE_STAIRS_DOWN_II :
                   (vgrid == '(') ? DNGN_STONE_STAIRS_UP_II :
                   (vgrid == ']') ? DNGN_STONE_STAIRS_DOWN_III :
                   (vgrid == '[') ? DNGN_STONE_STAIRS_UP_III :
                   (vgrid == 'A') ? DNGN_STONE_ARCH :
                   (vgrid == 'B') ?
                   static_cast<dungeon_feature_type>(
                       DNGN_ALTAR_FIRST_GOD + altar_count) :// see below
                   (vgrid == 'C') ? pick_an_altar() :   // f(x) elsewhere {dlb}
                   (vgrid == 'F') ? DNGN_GRANITE_STATUE :
                   (vgrid == 'I') ? DNGN_ORCISH_IDOL :
                   (vgrid == 'G') ? DNGN_GRANITE_STATUE :
                   (vgrid == 'T') ? DNGN_BLUE_FOUNTAIN :
                   (vgrid == 'U') ? DNGN_SPARKLING_FOUNTAIN :
                   (vgrid == 'V') ? DNGN_PERMADRY_FOUNTAIN :
                   (vgrid == '\0')? DNGN_ROCK_WALL :
                                    DNGN_FLOOR); // includes everything else

    // then, handle oddball grids {dlb}:
    switch (vgrid)
    {
    case 'B':
        altar_count++;
        break;
    case '@':
        targets.push_back( coord_def(vx, vy) );
        break;
    case '^':
        place_specific_trap(vx, vy, TRAP_RANDOM);
        break;
    case '~':
        place_specific_trap(vx, vy, random_trap_for_place(level_number));
        break;
    }

    if ((vgrid == '=' || vgrid == '+')
        && (vx == place.pos.x || vy == place.pos.y
            || vx == place.pos.x + place.size.x - 1
            || vy == place.pos.y + place.size.y - 1))
    {
        targets.push_back( coord_def(vx, vy) );
    }

    // then, handle grids that place "stuff" {dlb}:
    switch (vgrid)              // yes, I know this is a bit ugly ... {dlb}
    {
    case 'R':
    case '$':
    case '%':
    case '*':
    case '|':
    case 'P':                   // possible rune
    case 'O':                   // definite rune
    case 'Z':                   // definite orb
        {
            int item_made = NON_ITEM;
            object_class_type which_class = OBJ_RANDOM;
            unsigned char which_type = OBJ_RANDOM;
            int which_depth;
            bool possible_rune = one_chance_in(3);      // lame, I know {dlb}
            int spec = 250;

            if (vgrid == 'R')
            {
                which_class = OBJ_FOOD;
                which_type = (one_chance_in(3) ? FOOD_ROYAL_JELLY
                                               : FOOD_HONEYCOMB);
            }
            else if (vgrid == '$')
            {
                which_class = OBJ_GOLD;
                which_type = OBJ_RANDOM;
            }
            else if (vgrid == '%' || vgrid == '*')
            {
                which_class = OBJ_RANDOM;
                which_type = OBJ_RANDOM;
            }
            else if (vgrid == 'Z')
            {
                which_class = OBJ_ORBS;
                which_type = ORB_ZOT;
            }
            else if (vgrid == '|' 
                    || (vgrid == 'P' && (!possible_rune || num_runes > 0))
                    || (vgrid == 'O' && num_runes > 0))
            {
                which_class = acq_item_class[random2(7)];
                which_type = OBJ_RANDOM;
            }
            else              // for 'P' (1 out of 3 times) {dlb}
            {
                if (rune_subst != -1)
                {
                    grd[vx][vy] = static_cast<dungeon_feature_type>(rune_subst);
                    break;
                }
                
                which_class = OBJ_MISCELLANY;
                which_type = MISC_RUNE_OF_ZOT;
                num_runes++;

                if (you.level_type == LEVEL_PANDEMONIUM)
                {
                    if (place.map.has_tag("mnoleg"))
                        spec = RUNE_MNOLEG;
                    else if (place.map.has_tag("lom_lobon"))
                        spec = RUNE_LOM_LOBON;
                    else if (place.map.has_tag("gloorx_vloq"))
                        spec = RUNE_GLOORX_VLOQ;
                    else if (place.map.has_tag("cerebov"))
                        spec = RUNE_CEREBOV;
                    else
                        spec = RUNE_DEMONIC;
                }
                else if (you.level_type == LEVEL_ABYSS)
                    spec = RUNE_ABYSSAL;
                else 
                    spec = you.where_are_you;
            }

            which_depth = ((vgrid == '|'
                            || vgrid == 'P'
                            || vgrid == 'O'
                            || vgrid == 'Z') ? MAKE_GOOD_ITEM :
                           (vgrid == '*')    ? 5 + (level_number * 2)
                                             : level_number);
            
            item_made = items( 1, which_class, which_type, true, 
                               which_depth, spec );          

            if (item_made != NON_ITEM)
            {
                mitm[item_made].x = vx;
                mitm[item_made].y = vy;
            }
        }
        break;
    }

    // defghijk - items
    if (vgrid >= 'd' && vgrid <= 'k')
    {
        dgn_place_item_explicit(vgrid - 'd', vx, vy, place, level_number);
    }

    if (vgrid == 'S' || vgrid == 'H')
    {
        const int mtype = 
            vgrid == 'H'? MONS_ORANGE_STATUE : MONS_SILVER_STATUE;

        grd[vx][vy] = DNGN_FLOOR;

        place_monster( not_used, mtype, 30, BEH_HOSTILE,
                       MHITNOT, true, vx, vy, false);
    }

    // finally, handle grids that place monsters {dlb}:
    if (vgrid >= '0' && vgrid <= '9')
    {
        int monster_level;
        mons_spec monster_type_thing(RANDOM_MONSTER);

        monster_level = ((vgrid == '8') ? (4 + (level_number * 2)) :
                          (vgrid == '9') ? (5 + level_number) : level_number);

        if (monster_level > 30) // very high level monsters more common here
            monster_level = 30;

        if (vgrid != '8' && vgrid != '9' && vgrid != '0')
            monster_type_thing = place.map.mons.get_monster(vgrid - '1');

        dgn_place_monster(place, monster_type_thing, monster_level, vx, vy);
    }

    // again, this seems odd, given that this is just one of many
    // vault types {dlb}
    return (altar_count);
}                               // end vault_grid()

static void replace_area(
    int sx, int sy, int ex, int ey, dungeon_feature_type replace,
    dungeon_feature_type feature, unsigned mapmask)
{
    int x,y;
    for(x=sx; x<=ex; x++)
        for(y=sy; y<=ey; y++)
            if (grd[x][y] == replace && unforbidden(coord_def(x, y), mapmask))
                grd[x][y] = feature;
}

// With apologies to Metallica.
bool unforbidden(const coord_def &c, unsigned mask)
{
    return (!mask || !(dgn_map_mask(c) & mask));
}

struct coord_comparator
{
    coord_def target;
    coord_comparator(const coord_def &t) : target(t) { }

    static int dist(const coord_def &a, const coord_def &b)
    {
        const coord_def del = a - b;
        return std::abs(del.x) * GYM + std::abs(del.y);
    }
    
    bool operator () (const coord_def &a, const coord_def &b) const
    {
        return dist(a, target) < dist(b, target);
    }
};

typedef std::set<coord_def, coord_comparator> coord_set;

static void jtd_init_surrounds(coord_set &coords,
                              unsigned mapmask,
                              const coord_def &c)
{
    for (int yi = -1; yi <= 1; ++yi)
    {
        for (int xi = -1; xi <= 1; ++xi)
        {
            if (!xi == !yi)
                continue;
            const coord_def cx(c.x + xi, c.y + yi);
            if (!in_bounds(cx) || travel_point_distance[cx.x][cx.y]
                || !unforbidden(cx, mapmask))
            {
                continue;
            }
            coords.insert(cx);

            travel_point_distance[cx.x][cx.y] = (-xi + 2) * 4 + (-yi + 2);
        }
    }
}

static bool join_the_dots_pathfind(coord_set &coords,
                                   const coord_def &from,
                                   const coord_def &to,
                                   unsigned mapmask,
                                   bool early_exit)
{
    coord_def curr = from;
    while (true)
    {
        int &tpd = travel_point_distance[curr.x][curr.y];
        tpd = !tpd? -1000 : -tpd;
        
        if (curr == to)
            break;
        
        jtd_init_surrounds(coords, mapmask, curr);

        if (coords.empty())
            break;
        
        curr = *coords.begin();
        coords.erase(coords.begin());
    }

    if (curr != to)
        return (false);

    while (curr != from)
    {
        if (unforbidden(curr, mapmask))
            grd(curr) = DNGN_FLOOR;
        const int dist = travel_point_distance[curr.x][curr.y];
        ASSERT(dist < 0 && dist != -1000);
        curr += coord_def(-dist / 4 - 2, (-dist % 4) - 2);
    }
    if (unforbidden(curr, mapmask))
        grd(curr) = DNGN_FLOOR;
    
    return (true);
}

static bool join_the_dots_rigorous(const coord_def &from,
                                   const coord_def &to,
                                   unsigned mapmask,
                                   bool early_exit)
{
    memset(travel_point_distance, 0, sizeof(travel_distance_grid_t));

    const coord_comparator comp(to);
    coord_set coords(comp);
    const bool found =
        join_the_dots_pathfind(coords, from, to, mapmask, early_exit);

    return (found);
}

static bool join_the_dots(
    const coord_def &from,
    const coord_def &to,
    unsigned mapmask,
    bool early_exit)
{
    if (from == to)
        return (true);

    int join_count = 0;

    coord_def at = from;
    do
    {
        join_count++;

        const dungeon_feature_type feat = grd(at);
        if (early_exit && at != from && is_traversable(feat))
            return (true);

        if (unforbidden(at, MMT_VAULT) && !is_traversable(feat))
            grd(at) = DNGN_FLOOR;

        if (join_count > 10000) // just insurance
            return (false);

        if (at.x < to.x
            && unforbidden(coord_def(at.x + 1, at.y), mapmask))
        {
            at.x++;
            continue;
        }

        if (at.x > to.x
            && unforbidden(coord_def(at.x - 1, at.y), mapmask))
        {
            at.x--;
            continue;
        }

        if (at.y > to.y
            && unforbidden(coord_def(at.x, at.y - 1), mapmask))
        {
            at.y--;
            continue;
        }

        if (at.y < to.y
            && unforbidden(coord_def(at.x, at.y + 1), mapmask))
        {
            at.y++;
            continue;
        }

        // If we get here, no progress can be made anyway. Why use the
        // join_count insurance?
        break;
    }
    while (at != to && in_bounds(at));

    if (in_bounds(at) && unforbidden(at, mapmask))
        grd(at) = DNGN_FLOOR;

    return (at == to);
}                               // end join_the_dots()

static void place_pool(dungeon_feature_type pool_type, unsigned char pool_x1,
                       unsigned char pool_y1, unsigned char pool_x2,
                       unsigned char pool_y2)
{
    int i, j;
    unsigned char left_edge, right_edge;

    // don't place LAVA pools in crypt.. use shallow water instead.
    if (pool_type == DNGN_LAVA 
        && (player_in_branch(BRANCH_CRYPT) || player_in_branch(BRANCH_TOMB)))
    {
        pool_type = DNGN_SHALLOW_WATER;
    }

    if (pool_x1 >= pool_x2 - 4 || pool_y1 >= pool_y2 - 4)
        return;

    left_edge = pool_x1 + 2 + random2(pool_x2 - pool_x1);
    right_edge = pool_x2 - 2 - random2(pool_x2 - pool_x1);

    for (j = pool_y1 + 1; j < pool_y2 - 1; j++)
    {
        for (i = pool_x1 + 1; i < pool_x2 - 1; i++)
        {
            if (i >= left_edge && i <= right_edge && grd[i][j] == DNGN_FLOOR)
                grd[i][j] = pool_type;
        }

        if (j - pool_y1 < (pool_y2 - pool_y1) / 2 || one_chance_in(4))
        {
            if (left_edge > pool_x1 + 1)
                left_edge -= random2(3);

            if (right_edge < pool_x2 - 1)
                right_edge += random2(3);
        }

        if (left_edge < pool_x2 - 1
            && (j - pool_y1 >= (pool_y2 - pool_y1) / 2
                || left_edge <= pool_x1 + 2 || one_chance_in(4)))
        {
            left_edge += random2(3);
        }

        if (right_edge > pool_x1 + 1
            && (j - pool_y1 >= (pool_y2 - pool_y1) / 2
                || right_edge >= pool_x2 - 2 || one_chance_in(4)))
        {
            right_edge -= random2(3);
        }
    }
}                               // end place_pool()

static void many_pools(dungeon_feature_type pool_type)
{

    if (player_in_branch( BRANCH_COCYTUS ))
        pool_type = DNGN_DEEP_WATER;
    else if (player_in_branch( BRANCH_GEHENNA ))
        pool_type = DNGN_LAVA;

    const int num_pools = 20 + random2avg(9, 2);
    int pools = 0;

    for ( int timeout = 0; pools < num_pools && timeout < 30000; ++timeout )
    {
        const int i = random_range(X_BOUND_1 + 1, X_BOUND_2 - 21);
        const int j = random_range(Y_BOUND_1 + 1, Y_BOUND_2 - 21);
        const int k = i + 2 + roll_dice( 2, 9 );
        const int l = j + 2 + roll_dice( 2, 9 );

        if ( count_antifeature_in_box(i, j, k, l, DNGN_FLOOR) == 0 )
        {
            place_pool(pool_type, i, j, k, l);
            pools++;
        }
    }
}                               // end many_pools()

//jmf: generate altar based on where you are, or possibly randomly
static dungeon_feature_type pick_an_altar()
{
    dungeon_feature_type altar_type;
    int temp_rand;              // probability determination {dlb}

    if (player_in_branch( BRANCH_SLIME_PITS ) 
        || player_in_branch( BRANCH_ECUMENICAL_TEMPLE )
        || you.level_type == LEVEL_LABYRINTH)
    {
        // no extra altars in temple, none at all in slime pits or labyrinth
        altar_type = DNGN_FLOOR;
    }
    else if (you.level_type == LEVEL_DUNGEON && !one_chance_in(5))
    {
        switch (you.where_are_you)
        {
        case BRANCH_CRYPT:
            altar_type = (coinflip() ? DNGN_ALTAR_KIKUBAAQUDGHA
                                     : DNGN_ALTAR_YREDELEMNUL);
            break;

        case BRANCH_ORCISH_MINES:       // violent gods
            temp_rand = random2(10); // 50% chance of Beogh

            altar_type = ((temp_rand == 0) ? DNGN_ALTAR_VEHUMET :
                          (temp_rand == 1) ? DNGN_ALTAR_MAKHLEB :
                          (temp_rand == 2) ? DNGN_ALTAR_OKAWARU :
                          (temp_rand == 3) ? DNGN_ALTAR_TROG :
                          (temp_rand == 4) ? DNGN_ALTAR_XOM
                                           : DNGN_ALTAR_BEOGH);
            break;

        case BRANCH_VAULTS: // "lawful" gods
            temp_rand = random2(7);

            altar_type = ((temp_rand == 0) ? DNGN_ALTAR_ELYVILON :
                          (temp_rand == 1) ? DNGN_ALTAR_SIF_MUNA :
                          (temp_rand == 2) ? DNGN_ALTAR_SHINING_ONE :
                          (temp_rand == 3 || temp_rand == 4) ? DNGN_ALTAR_OKAWARU
                                           : DNGN_ALTAR_ZIN);
            break;

        case BRANCH_HALL_OF_BLADES:
            altar_type = DNGN_ALTAR_OKAWARU;
            break;

        case BRANCH_ELVEN_HALLS:    // "magic" gods
            temp_rand = random2(4);

            altar_type = ((temp_rand == 0) ? DNGN_ALTAR_VEHUMET :
                          (temp_rand == 1) ? DNGN_ALTAR_SIF_MUNA :
                          (temp_rand == 2) ? DNGN_ALTAR_XOM
                                           : DNGN_ALTAR_MAKHLEB);
            break;

        case BRANCH_TOMB:
            altar_type = DNGN_ALTAR_KIKUBAAQUDGHA;
            break;

        default:
            do
            {
                altar_type =
                    static_cast<dungeon_feature_type>(DNGN_ALTAR_FIRST_GOD +
                                                      random2(NUM_GODS - 1));
            }
            while (altar_type == DNGN_ALTAR_NEMELEX_XOBEH
                   || altar_type == DNGN_ALTAR_LUGONU
                   || altar_type == DNGN_ALTAR_BEOGH);
            break;
        }
    }
    else
    {
        // Note: this case includes Pandemonium or the Abyss.
        temp_rand = random2(9);

        altar_type = ((temp_rand == 0) ? DNGN_ALTAR_ZIN :
                      (temp_rand == 1) ? DNGN_ALTAR_SHINING_ONE :
                      (temp_rand == 2) ? DNGN_ALTAR_KIKUBAAQUDGHA :
                      (temp_rand == 3) ? DNGN_ALTAR_XOM :
                      (temp_rand == 4) ? DNGN_ALTAR_OKAWARU :
                      (temp_rand == 5) ? DNGN_ALTAR_MAKHLEB :
                      (temp_rand == 6) ? DNGN_ALTAR_SIF_MUNA :
                      (temp_rand == 7) ? DNGN_ALTAR_TROG
                                       : DNGN_ALTAR_ELYVILON);
    }

    return (altar_type);
}                               // end pick_an_altar()

static void place_altars()
{
    // No altars before level 5.
    if (you.your_level < 4)
        return;

    if ( you.level_type == LEVEL_DUNGEON )
    {
        int prob = your_branch().altar_chance;
        while (prob)
        {
            if (random2(100) >= prob)
                break;

#ifdef DEBUG_DIAGNOSTICS
            mprf(MSGCH_DIAGNOSTICS, "Placing an altar");
#endif
            place_altar();
            // Reduce the chance and try to place another.
            prob /= 5;
        }
    }
}

static void place_altar()
{
    for ( int numtry = 0; numtry < 5000; ++numtry )
    {
        int px = 15 + random2(55);
        int py = 15 + random2(45);

        const int numfloors = count_feature_in_box(px-2, py-2, px+3, py+3,
                                                   DNGN_FLOOR);
        const int numgood =
            count_feature_in_box(px-2, py-2, px+3, py+3, DNGN_ROCK_WALL) +
            count_feature_in_box(px-2, py-2, px+3, py+3, DNGN_CLOSED_DOOR) +
            count_feature_in_box(px-2, py-2, px+3, py+3, DNGN_SECRET_DOOR) +
            count_feature_in_box(px-2, py-2, px+3, py+3, DNGN_FLOOR);

        if ( numgood < 5*5 || numfloors == 0 )
            continue;

        bool mon_there = false;

        for (int i = px - 2; i <= px + 2; i++)
            for (int j = py - 2; j <= py + 2; j++)
                if (mgrd[i][j] != NON_MONSTER)
                    mon_there = true;

        if ( mon_there )
            continue;

        for (int i = px - 2; i <= px + 2; i++)
            for (int j = py - 2; j <= py + 2; j++)
                grd[i][j] = DNGN_FLOOR;
        grd[px][py] = pick_an_altar();
        break;
    }
}                               // end place_altar()

static void place_shops(int level_number, int nshops)
{
    int temp_rand = 0;          // probability determination {dlb}
    int timeout = 0;

    unsigned char shop_place_x = 0;
    unsigned char shop_place_y = 0;

    bool allow_bazaars = false;

    temp_rand = random2(125);

    if (!nshops)
    {
#if DEBUG_SHOPS
        nshops = MAX_SHOPS;
#else
        nshops = ((temp_rand > 28) ? 0 :                        // 76.8%
                  (temp_rand > 4)  ? 1                          // 19.2% 
                  : random_range(1, MAX_RANDOM_SHOPS));          //  4.0% 

        if (nshops == 0 || level_number < 3)
            return;
#endif
        allow_bazaars = true;
    }

    for (int i = 0; i < nshops; i++)
    {
        timeout = 0;

        do
        {
            shop_place_x = random_range(X_BOUND_1 + 1, X_BOUND_2 - 1);
            shop_place_y = random_range(Y_BOUND_1 + 1, Y_BOUND_2 - 1);

            timeout++;

            if (timeout > 10000)
                return;
        }
        while (grd[shop_place_x][shop_place_y] != DNGN_FLOOR);

        if (allow_bazaars && level_number > 9 && level_number < 27
            && one_chance_in(30 - level_number))
        {
            place_specific_stair(DNGN_ENTER_PORTAL_VAULT,
                                 "bzr_entry",
                                 level_number, true);
            allow_bazaars = false;
        }
        else
        {
            place_spec_shop(level_number,
                            shop_place_x, shop_place_y, SHOP_RANDOM);
        }
    }
}                               // end place_shops()

static bool need_varied_selection(shop_type shop)
{
    return (shop == SHOP_BOOK);
}

void place_spec_shop( int level_number, 
                      int shop_x, int shop_y,
                      int force_s_type, bool representative )
{
    int orb = 0;
    int i = 0;
    int j = 0;                  // loop variable
    int item_level;

    bool note_status = notes_are_active();
    activate_notes(false);

    for (i = 0; i < MAX_SHOPS; i++)
    {
        if (env.shop[i].type == SHOP_UNASSIGNED)
            break;
    }

    if (i == MAX_SHOPS)
        return;

    for (j = 0; j < 3; j++)
    {
        env.shop[i].keeper_name[j] = 1 + random2(200);
    }

    env.shop[i].level = level_number * 2;

    env.shop[i].type = static_cast<shop_type>(
        (force_s_type != SHOP_RANDOM) ? force_s_type 
                                      : random2(NUM_SHOPS));

    if (env.shop[i].type == SHOP_FOOD)
    {
        env.shop[i].greed = 10 + random2(5);
    }
    else if (env.shop[i].type != SHOP_WEAPON_ANTIQUE
        && env.shop[i].type != SHOP_ARMOUR_ANTIQUE
        && env.shop[i].type != SHOP_GENERAL_ANTIQUE)
    {
        env.shop[i].greed = 10 + random2(5) + random2(level_number / 2);
    }
    else
    {
        env.shop[i].greed = 15 + random2avg(19, 2) + random2(level_number);
    }
    
    // allow bargains in bazaars, prices randomly between 60% and 95%
    if (you.level_type == LEVEL_PORTAL_VAULT && you.level_type_name == "bazaar")
    {
        // need to calculate with factor as greed (unsigned char) is capped at 255
        int factor = random2(8)+12;
#ifdef DEBUG_DIAGNOSTICS
        mprf(MSGCH_DIAGNOSTICS, "shop type %d: original greed = %d, factor = %d",
                                env.shop[i].type, env.shop[i].greed, factor);
        mprf(MSGCH_DIAGNOSTICS, "discount at shop %d is %d%%", i, (20-factor)*5);
#endif
        factor *= env.shop[i].greed;
        factor /= 20;
        env.shop[i].greed = factor;
    }

    int plojy = 5 + random2avg(12, 3);
    if (representative)
        plojy = env.shop[i].type == SHOP_WAND? NUM_WANDS : 16;

    // for books shops, store how many copies of a given book are on display
    int stocked[NUM_BOOKS];
    if (need_varied_selection(env.shop[i].type))
    {
        for (int k=0; k < NUM_BOOKS; k++)
             stocked[k] = 0;
    }

    for (j = 0; j < plojy; j++)
    {
        if (env.shop[i].type != SHOP_WEAPON_ANTIQUE
            && env.shop[i].type != SHOP_ARMOUR_ANTIQUE
            && env.shop[i].type != SHOP_GENERAL_ANTIQUE)
        {
            item_level = level_number + random2((level_number + 1) * 2);
        }
        else
        {
            item_level = level_number + random2((level_number + 1) * 3);
        }

        // make bazaar items more valuable (up to double value)
        if (you.level_type == LEVEL_PORTAL_VAULT && you.level_type_name == "bazaar")
        {
            int help = random2(item_level) + 1;
            item_level += help;
            
            if (item_level > level_number * 5)
                item_level = level_number * 5;
        }

        // don't generate gold in shops!  This used to be possible with
        // General Stores (see item_in_shop() below)   (GDL)
        while (true)
        {
            const int subtype = representative? j : OBJ_RANDOM;
            orb = items( 1, item_in_shop(env.shop[i].type), subtype, true,
                         one_chance_in(4)? MAKE_GOOD_ITEM : item_level,
                         MAKE_ITEM_RANDOM_RACE );

            // try for a better selection
            if (orb != NON_ITEM && need_varied_selection(env.shop[i].type))
            {
                if (!one_chance_in(stocked[mitm[orb].sub_type] + 1))
                    orb = NON_ITEM; // try again
            }

            if (orb != NON_ITEM
                && mitm[orb].base_type != OBJ_GOLD
                && (env.shop[i].type != SHOP_GENERAL_ANTIQUE
                    || (mitm[orb].base_type != OBJ_MISSILES
                        && mitm[orb].base_type != OBJ_FOOD)))
            {
                break;
            }

            // reset object and try again
            if (orb != NON_ITEM)
            {
                mitm[orb].base_type = OBJ_UNASSIGNED;
                mitm[orb].quantity = 0;
            }
        }

        if (orb == NON_ITEM)
            break;

        // increase stock of this subtype by 1, unless it is an artefact
        // (allow for several artefacts of the same underlying subtype)
        // - the latter is currently unused but would apply to e.g. jewellery
        if (need_varied_selection(env.shop[i].type) && !is_artefact(mitm[orb]))
            stocked[mitm[orb].sub_type]++;

        if (representative && mitm[orb].base_type == OBJ_WANDS)
            mitm[orb].plus = 7;

        // set object 'position' (gah!) & ID status
        mitm[orb].x = 0;
        mitm[orb].y = 5 + i;

        if (env.shop[i].type != SHOP_WEAPON_ANTIQUE
            && env.shop[i].type != SHOP_ARMOUR_ANTIQUE
            && env.shop[i].type != SHOP_GENERAL_ANTIQUE)
        {
            set_ident_flags( mitm[orb], ISFLAG_IDENT_MASK );
        }
    }

    env.shop[i].x = shop_x;
    env.shop[i].y = shop_y;

    grd[shop_x][shop_y] = DNGN_ENTER_SHOP;

    activate_notes(note_status);
}                               // end place_spec_shop()

static object_class_type item_in_shop(unsigned char shop_type)
{
    switch (shop_type)
    {
    case SHOP_WEAPON:
        if (one_chance_in(5))
            return (OBJ_MISSILES);
        // *** deliberate fall through here  {dlb} ***
    case SHOP_WEAPON_ANTIQUE:
        return (OBJ_WEAPONS);

    case SHOP_ARMOUR:
    case SHOP_ARMOUR_ANTIQUE:
        return (OBJ_ARMOUR);

    case SHOP_GENERAL:
    case SHOP_GENERAL_ANTIQUE:
        return (OBJ_RANDOM);

    case SHOP_JEWELLERY:
        return (OBJ_JEWELLERY);

    case SHOP_WAND:
        return (OBJ_WANDS);

    case SHOP_BOOK:
        return (OBJ_BOOKS);

    case SHOP_FOOD:
        return (OBJ_FOOD);

    case SHOP_DISTILLERY:
        return (OBJ_POTIONS);

    case SHOP_SCROLL:
        return (OBJ_SCROLLS);
    }

    return (OBJ_RANDOM);
}                               // end item_in_shop()

static void spotty_level(bool seeded, int iterations, bool boxy)
{
    // assumes starting with a level full of rock walls (1)
    int i, j, k, l;

    if (!seeded)
    {
        for (i = DNGN_STONE_STAIRS_DOWN_I; i < DNGN_ROCK_STAIRS_UP; i++)
        {
            if (i == DNGN_ROCK_STAIRS_DOWN 
                || (i == DNGN_STONE_STAIRS_UP_I 
                    && !player_in_branch( BRANCH_SLIME_PITS )))
            {
                continue;
            }

            do
            {
                j = random_range(X_BOUND_1 + 4, X_BOUND_2 - 4);
                k = random_range(Y_BOUND_1 + 4, Y_BOUND_2 - 4);
            }
            while (grd[j][k] != DNGN_ROCK_WALL
                    && grd[j + 1][k] != DNGN_ROCK_WALL);

            grd[j][k] = static_cast<dungeon_feature_type>(i);

            // creating elevators
            if (i == DNGN_STONE_STAIRS_DOWN_I
                && !player_in_branch( BRANCH_SLIME_PITS ))
            {
                grd[j + 1][k] = DNGN_STONE_STAIRS_UP_I;
            }

            if (grd[j][k - 1] == DNGN_ROCK_WALL)
                grd[j][k - 1] = DNGN_FLOOR;
            if (grd[j][k + 1] == DNGN_ROCK_WALL)
                grd[j][k + 1] = DNGN_FLOOR;
            if (grd[j - 1][k] == DNGN_ROCK_WALL)
                grd[j - 1][k] = DNGN_FLOOR;
            if (grd[j + 1][k] == DNGN_ROCK_WALL)
                grd[j + 1][k] = DNGN_FLOOR;
        }
    }                           // end if !seeded

    l = iterations;

    // boxy levels have more clearing, so they get fewer iterations:
    if (l == 0)
        l = 200 + random2( (boxy ? 750 : 1500) );

    for (i = 0; i < l; i++)
    {
        do
        {
            j = random_range(X_BOUND_1 + 4, X_BOUND_2 - 4);
            k = random_range(Y_BOUND_1 + 4, Y_BOUND_2 - 4);
        }
        while (grd[j][k] == DNGN_ROCK_WALL
               && grd[j - 1][k] == DNGN_ROCK_WALL
               && grd[j + 1][k] == DNGN_ROCK_WALL
               && grd[j][k - 1] == DNGN_ROCK_WALL
               && grd[j][k + 1] == DNGN_ROCK_WALL
               && grd[j - 2][k] == DNGN_ROCK_WALL
               && grd[j + 2][k] == DNGN_ROCK_WALL
               && grd[j][k - 2] == DNGN_ROCK_WALL
               && grd[j][k + 2] == DNGN_ROCK_WALL);

        if (grd[j][k] == DNGN_ROCK_WALL)
            grd[j][k] = DNGN_FLOOR;
        if (grd[j][k - 1] == DNGN_ROCK_WALL)
            grd[j][k - 1] = DNGN_FLOOR;
        if (grd[j][k + 1] == DNGN_ROCK_WALL)
            grd[j][k + 1] = DNGN_FLOOR;
        if (grd[j - 1][k] == DNGN_ROCK_WALL)
            grd[j - 1][k] = DNGN_FLOOR;
        if (grd[j + 1][k] == DNGN_ROCK_WALL)
            grd[j + 1][k] = DNGN_FLOOR;

        if (boxy)
        {
            if (grd[j - 1][k - 1] == DNGN_ROCK_WALL)
                grd[j - 1][k - 1] = DNGN_FLOOR;
            if (grd[j + 1][k + 1] == DNGN_ROCK_WALL)
                grd[j + 1][k + 1] = DNGN_FLOOR;
            if (grd[j - 1][k + 1] == DNGN_ROCK_WALL)
                grd[j - 1][k + 1] = DNGN_FLOOR;
            if (grd[j + 1][k - 1] == DNGN_ROCK_WALL)
                grd[j + 1][k - 1] = DNGN_FLOOR;
        }
    }
}                               // end spotty_level()

static void bigger_room()
{
    unsigned char i, j;

    for (i = 10; i < (GXM - 10); i++)
    {
        for (j = 10; j < (GYM - 10); j++)
        {
            if (grd[i][j] == DNGN_ROCK_WALL)
                grd[i][j] = DNGN_FLOOR;
        }
    }

    many_pools(DNGN_DEEP_WATER);

    if (one_chance_in(3))
    {       
        if (coinflip())
            build_river( DNGN_DEEP_WATER );
        else
            build_lake( DNGN_DEEP_WATER );
    }

    int pair_count = coinflip() ? 4 : 3;

    for (j = 0; j < pair_count; j++)
    {
        for (i = 0; i < 2; i++)
        {
            place_specific_stair(
                static_cast<dungeon_feature_type>(
                    j + ((i==0) ? DNGN_STONE_STAIRS_DOWN_I
                         : DNGN_STONE_STAIRS_UP_I) ) );
        }
    }
}                               // end bigger_room()

// various plan_xxx functions
static void plan_main(int level_number, int force_plan)
{
    // possible values for do_stairs:
    //  0 - stairs already done
    //  1 - stairs already done, do spotty
    //  2 - no stairs
    //  3 - no stairs, do spotty
    int do_stairs = 0;
    dungeon_feature_type special_grid = (one_chance_in(3) ? DNGN_METAL_WALL
                                         : DNGN_STONE_WALL);
    int i,j;

    if (!force_plan)
        force_plan = 1 + random2(12);

    do_stairs = ((force_plan == 1) ? plan_1() :
                 (force_plan == 2) ? plan_2() :
                 (force_plan == 3) ? plan_3() :
                 (force_plan == 4) ? plan_4(0, 0, 0, 0, NUM_FEATURES) :
                 (force_plan == 5) ? (one_chance_in(9) ? plan_5()
                                                       : plan_3()) :
                 (force_plan == 6) ? plan_6(level_number)
                                   : plan_3());

    if (do_stairs == 3 || do_stairs == 1)
        spotty_level(true, 0, coinflip());

    if (do_stairs == 2 || do_stairs == 3)
    {
        int pair_count = coinflip()?4:3;

        for (j = 0; j < pair_count; j++)
        {
            for (i = 0; i < 2; i++)
            {
                place_specific_stair(
                    static_cast<dungeon_feature_type>(
                        j + ((i==0) ? DNGN_STONE_STAIRS_DOWN_I
                             : DNGN_STONE_STAIRS_UP_I) ) );
            }
        }
    }

    if (one_chance_in(20))
        replace_area(0,0,GXM-1,GYM-1,DNGN_ROCK_WALL,special_grid);
}                               // end plan_main()

static char plan_1()
{
    int temp_rand = 0;          // probability determination {dlb}

    unsigned char width = (10 - random2(7));    // value range of [4,10] {dlb}

    replace_area(10, 10, (GXM - 10), (10 + width), DNGN_ROCK_WALL, DNGN_FLOOR);
    replace_area(10, (60 - width), (GXM - 10), (GYM - 10),
        DNGN_ROCK_WALL, DNGN_FLOOR);
    replace_area(10, 10, (10 + width), (GYM - 10), DNGN_ROCK_WALL, DNGN_FLOOR);
    replace_area((60 - width), 10, (GXM - 10), (GYM - 10),
        DNGN_ROCK_WALL, DNGN_FLOOR);

    // possible early returns {dlb}:
    temp_rand = random2(4);

    if (temp_rand > 2)          // 25% chance {dlb}
        return 3;
    else if (temp_rand > 1)     // 25% chance {dlb}
        return 2;
    else                        // 50% chance {dlb}
    {
        unsigned char width2 = (coinflip()? (1 + random2(5)) : 5);

        replace_area(10, (35 - width2), (GXM - 10), (35 + width2),
                   DNGN_ROCK_WALL, DNGN_FLOOR);
        replace_area((40 - width2), 10, (40 + width2), (GYM - 10),
                   DNGN_ROCK_WALL, DNGN_FLOOR);
    }

    // possible early returns {dlb}:
    temp_rand = random2(4);

    if (temp_rand > 2)          // 25% chance {dlb}
        return 3;
    else if (temp_rand > 1)     // 25% chance {dlb}
        return 2;
    else                        // 50% chance {dlb}
    {
        temp_rand = random2(15);

        if (temp_rand > 7)      // 7 in 15 odds {dlb}
        {
            spec_room sr;
            sr.x1 = 25;
            sr.y1 = 25;
            sr.x2 = (GXM - 25);
            sr.y2 = (GYM - 25);

            int oblique_max = 0;
            if (coinflip())
                oblique_max = 5 + random2(20);

            temp_rand = random2(7);

            dungeon_feature_type floor_type =
                ((temp_rand > 1) ? DNGN_FLOOR :   // 5/7
                 (temp_rand > 0) ? DNGN_DEEP_WATER// 1/7
                                 : DNGN_LAVA);    // 1/7
            octa_room(sr, oblique_max, floor_type);
        }
    }

    // final return {dlb}:
    return (one_chance_in(5) ? 3 : 2);
}                               // end plan_1()

// just a cross:
static char plan_2()
{
    char width2 = (5 - random2(5));     // value range of [1,5] {dlb}

    replace_area(10, (35 - width2), (GXM - 10), (35 + width2),
                                            DNGN_ROCK_WALL, DNGN_FLOOR);
    replace_area((40 - width2), 10, (40 + width2), (GYM - 10),
                                            DNGN_ROCK_WALL, DNGN_FLOOR);

    return (one_chance_in(4) ? 2 : 3);
}                               // end plan_2()

static char plan_3()
{

    /* Draws a room, then another and links them together, then another and etc
       Of course, this can easily end up looking just like a make_trail level.
     */
    int i;
    int roomsss = 30 + random2(90);

    bool exclusive = (one_chance_in(10) ? false : true);
    bool exclusive2 = coinflip();

    char romx1[30], romy1[30], romx2[30], romy2[30];

    int which_room = 0;

    for (i = 0; i < roomsss; i++)
    {
        romx1[which_room] = 10 + random2(50);
        romy1[which_room] = 10 + random2(40);
        romx2[which_room] = romx1[which_room] + 2 + random2(8);
        romy2[which_room] = romy1[which_room] + 2 + random2(8);

        if (exclusive && count_antifeature_in_box(romx1[which_room] - 1,
                                                  romy1[which_room] - 1,
                                                  romx2[which_room] + 1,
                                                  romy2[which_room] + 1,
                                                  DNGN_ROCK_WALL))
            continue;

        replace_area(romx1[which_room], romy1[which_room], romx2[which_room],
                     romy2[which_room], DNGN_ROCK_WALL, DNGN_FLOOR);

        if (which_room > 0 && !exclusive2)
        {
            const int rx1 = romx1[which_room];
            const int rx2 = romx2[which_room];
            const int prev_rx1 = romx1[which_room - 1];
            const int prev_rx2 = romx2[which_room - 1];

            const int ry1 = romy1[which_room];
            const int ry2 = romy2[which_room];
            const int prev_ry1 = romy1[which_room - 1];
            const int prev_ry2 = romy2[which_room - 1];

            join_the_dots( coord_def(rx1 + random2( rx2 - rx1 ),
                                     ry1 + random2( ry2 - ry1 )),
                           coord_def(prev_rx1 + random2(prev_rx2 - prev_rx1),
                                     prev_ry1 + random2(prev_ry2 - prev_ry1)),
                           MMT_VAULT );
        }

        which_room++;

        if (which_room >= 29)
            break;

    }

    if (exclusive2)
    {
        for (i = 0; i < which_room; i++)
        {
            if (i > 0)
            {
                const int rx1 = romx1[i];
                const int rx2 = romx2[i];
                const int prev_rx1 = romx1[i - 1];
                const int prev_rx2 = romx2[i - 1];

                const int ry1 = romy1[i];
                const int ry2 = romy2[i];
                const int prev_ry1 = romy1[i - 1];
                const int prev_ry2 = romy2[i - 1];

                join_the_dots( coord_def( rx1 + random2( rx2 - rx1 ),
                                          ry1 + random2( ry2 - ry1 ) ),
                               coord_def(
                                   prev_rx1 + random2( prev_rx2 - prev_rx1 ),
                                   prev_ry1 + random2( prev_ry2 - prev_ry1 ) ),
                               MMT_VAULT );
            }
        }
    }

    return 2;
}                               // end plan_3()

static char plan_4(char forbid_x1, char forbid_y1, char forbid_x2,
                   char forbid_y2, dungeon_feature_type force_wall)
{
    // a more chaotic version of city level
    int temp_rand;              // req'd for probability checking

    int number_boxes = 5000;
    dungeon_feature_type drawing = DNGN_ROCK_WALL;
    char b1x, b1y, b2x, b2y;
    int i;

    temp_rand = random2(81);

    number_boxes = ((temp_rand > 48) ? 4000 :   // odds: 32 in 81 {dlb}
                    (temp_rand > 24) ? 3000 :   // odds: 24 in 81 {dlb}
                    (temp_rand >  8) ? 5000 :   // odds: 16 in 81 {dlb}
                    (temp_rand >  0) ? 2000     // odds:  8 in 81 {dlb}
                                     : 1000);   // odds:  1 in 81 {dlb}

    if (force_wall != NUM_FEATURES)
        drawing = force_wall;
    else
    {
        temp_rand = random2(18);

        drawing = ((temp_rand > 7) ? DNGN_ROCK_WALL :   // odds: 10 in 18 {dlb}
                   (temp_rand > 2) ? DNGN_STONE_WALL    // odds:  5 in 18 {dlb}
                                   : DNGN_METAL_WALL);  // odds:  3 in 18 {dlb}
    }

    replace_area(10, 10, (GXM - 10), (GYM - 10), DNGN_ROCK_WALL, DNGN_FLOOR);

    // replace_area can also be used to fill in:
    for (i = 0; i < number_boxes; i++)
    {

        b1x = 11 + random2(45);
        b1y = 11 + random2(35);

        b2x = b1x + 3 + random2(7) + random2(5);
        b2y = b1y + 3 + random2(7) + random2(5);

        if (forbid_x1 != 0 || forbid_x2 != 0)
        {
            if (b1x <= forbid_x2 && b1x >= forbid_x1
                && b1y <= forbid_y2 && b1y >= forbid_y1)
                continue;

            if (b2x <= forbid_x2 && b2x >= forbid_x1
                && b2y <= forbid_y2 && b2y >= forbid_y1)
                continue;
        }

        if (count_antifeature_in_box(b1x-1, b1y-1, b2x+1, b2y+1, DNGN_FLOOR))
            continue;

        if (force_wall == NUM_FEATURES)
        {
            // NB: comparison reversal here - combined
            temp_rand = random2(1200);

            // probabilities *not meant* to sum to one! {dlb}
            if (temp_rand < 417)        // odds: 261 in 1200 {dlb}
                drawing = DNGN_ROCK_WALL;
            else if (temp_rand < 156)   // odds: 116 in 1200 {dlb}
                drawing = DNGN_STONE_WALL;
            else if (temp_rand < 40)    // odds:  40 in 1200 {dlb}
                drawing = DNGN_METAL_WALL;
        }

        temp_rand = random2(210);

        if (temp_rand > 71)     // odds: 138 in 210 {dlb}
            replace_area(b1x, b1y, b2x, b2y, DNGN_FLOOR, drawing);
        else                    // odds:  72 in 210 {dlb}
            box_room(b1x, b2x - 1, b1y, b2y - 1, drawing);
    }

    if (forbid_x1 == 0 && one_chance_in(4))     // a market square
    {
        spec_room sr;
        sr.x1 = 25;
        sr.y1 = 25;
        sr.x2 = 55;
        sr.y2 = 45;

        int oblique_max = 0;
        if (!one_chance_in(4))
            oblique_max = 5 + random2(20);      // used elsewhere {dlb}

        dungeon_feature_type feature = DNGN_FLOOR;
        if (one_chance_in(10))
            feature = coinflip()? DNGN_DEEP_WATER : DNGN_LAVA;

        octa_room(sr, oblique_max, feature);
    }

    return 2;
}                               // end plan_4()

static char plan_5()
{
    unsigned char imax = 5 + random2(20);       // value range of [5,24] {dlb}

    for (unsigned char i = 0; i < imax; i++)
    {
        join_the_dots(
            coord_def( random2(GXM - 20) + 10, random2(GYM - 20) + 10 ),
            coord_def( random2(GXM - 20) + 10, random2(GYM - 20) + 10 ),
            MMT_VAULT );
    }

    if (!one_chance_in(4))
        spotty_level(true, 100, coinflip());

    return 2;
}                               // end plan_5()

static char plan_6(int level_number)
{
    spec_room sr;

    // circle of standing stones (well, kind of)
    sr.x1 = 10;
    sr.x2 = (GXM - 10);
    sr.y1 = 10;
    sr.y2 = (GYM - 10);

    octa_room(sr, 14, DNGN_FLOOR);

    replace_area(23, 23, 26, 26, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(23, 47, 26, 50, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(55, 23, 58, 26, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(55, 47, 58, 50, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(39, 20, 43, 23, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(39, 50, 43, 53, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(20, 30, 23, 33, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(20, 40, 23, 43, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(58, 30, 61, 33, DNGN_FLOOR, DNGN_STONE_WALL);
    replace_area(58, 40, 61, 43, DNGN_FLOOR, DNGN_STONE_WALL);

    grd[35][32] = DNGN_STONE_WALL;
    grd[46][32] = DNGN_STONE_WALL;
    grd[35][40] = DNGN_STONE_WALL;
    grd[46][40] = DNGN_STONE_WALL;

    grd[69][34] = DNGN_STONE_STAIRS_DOWN_I;
    grd[69][35] = DNGN_STONE_STAIRS_DOWN_II;
    grd[69][36] = DNGN_STONE_STAIRS_DOWN_III;

    grd[10][34] = DNGN_STONE_STAIRS_UP_I;
    grd[10][35] = DNGN_STONE_STAIRS_UP_II;
    grd[10][36] = DNGN_STONE_STAIRS_UP_III;

    // This "back door" is often one of the easier ways to get out of 
    // pandemonium... the easiest is to use the banish spell.  
    //
    // Note, that although "level_number > 20" will work for most 
    // trips to pandemonium (through regular portals), it won't work 
    // for demonspawn who gate themselves there. -- bwr
    if (((player_in_branch( BRANCH_MAIN_DUNGEON ) && level_number > 20)
            || you.level_type == LEVEL_PANDEMONIUM)
        && (coinflip() || you.mutation[ MUT_PANDEMONIUM ]))
    {
        grd[40][36] = DNGN_ENTER_ABYSS;
        grd[41][36] = DNGN_ENTER_ABYSS;
    }

    return 0;
}                               // end plan_6()

static bool octa_room(spec_room &sr, int oblique_max,
                      dungeon_feature_type type_floor)
{
    int x,y;

    // hack - avoid lava in the crypt {gdl}
    if ((player_in_branch( BRANCH_CRYPT ) || player_in_branch( BRANCH_TOMB ))
         && type_floor == DNGN_LAVA)
    {
        type_floor = DNGN_SHALLOW_WATER;
    }

    int oblique = oblique_max;

    // check octagonal room for special; avoid if exists
    for (x = sr.x1; x < sr.x2; x++)
    {
        for (y = sr.y1 + oblique; y < sr.y2 - oblique; y++)
        {
            if (grd[x][y] == DNGN_BUILDER_SPECIAL_WALL)
                return false;
        }

        if (oblique > 0)
            oblique--;

        if (x > sr.x2 - oblique_max)
            oblique += 2;
    }

    oblique = oblique_max;


    for (x = sr.x1; x < sr.x2; x++)
    {
        for (y = sr.y1 + oblique; y < sr.y2 - oblique; y++)
        {
            if (grd[x][y] == DNGN_ROCK_WALL)
                grd[x][y] = type_floor;

            if (grd[x][y] == DNGN_FLOOR && type_floor == DNGN_SHALLOW_WATER)
                grd[x][y] = DNGN_SHALLOW_WATER;

            if (grd[x][y] == DNGN_CLOSED_DOOR && !grid_is_solid(type_floor))
                grd[x][y] = DNGN_FLOOR;       // ick
        }

        if (oblique > 0)
            oblique--;

        if (x > sr.x2 - oblique_max)
            oblique += 2;
    }

    return true;
}                               // end octa_room()

static void find_maze_neighbours(const coord_def &c,
                                 const dgn_region &region,
                                 coord_list &ns)
{
    std::vector<coord_def> coords;

    for (int yi = -2; yi <= 2; yi += 2)
    {
        for (int xi = -2; xi <= 2; xi += 2)
        {
            if (!!xi == !!yi)
                continue;
            
            const coord_def cp(c.x + xi, c.y + yi);
            if (region.contains(cp))
                coords.push_back(cp);
        }
    }

    while (!coords.empty())
    {
        const int n = random2(coords.size());
        ns.push_back(coords[n]);
        coords.erase( coords.begin() + n );
    }
}

static void labyrinth_maze_recurse(const coord_def &c, const dgn_region &where)
{
    coord_list neighbours;
    find_maze_neighbours(c, where, neighbours);

    coord_list deferred;
    for (coord_list::iterator i = neighbours.begin();
         i != neighbours.end(); ++i)
    {
        const coord_def &nc = *i;
        if (grd(nc) != DNGN_ROCK_WALL)
            continue;
        
        grd(nc) = DNGN_FLOOR;
        grd(c + (nc - c) / 2) = DNGN_FLOOR;
        
        if (!one_chance_in(5))
            labyrinth_maze_recurse(nc, where);
        else
            deferred.push_back(nc);
    }

    for (coord_list::iterator i = deferred.begin(); i != deferred.end(); ++i)
        labyrinth_maze_recurse(*i, where);
}

static void labyrinth_build_maze(coord_def &e, const dgn_region &lab)
{
    labyrinth_maze_recurse(lab.random_point(), lab);

    do
        e = lab.random_point();
    while (grd(e) != DNGN_FLOOR);
}

static void labyrinth_place_items(const coord_def &end)
{
    int num_items = 8 + random2avg(9, 2);
    for (int i = 0; i < num_items; i++)
    {
        int temp_rand = random2(11);

        const object_class_type
            glopop = ((temp_rand == 0 || temp_rand == 9)  ? OBJ_WEAPONS :
                  (temp_rand == 1 || temp_rand == 10) ? OBJ_ARMOUR :
                  (temp_rand == 2)                    ? OBJ_MISSILES :
                  (temp_rand == 3)                    ? OBJ_WANDS :
                  (temp_rand == 4)                    ? OBJ_MISCELLANY :
                  (temp_rand == 5)                    ? OBJ_SCROLLS :
                  (temp_rand == 6)                    ? OBJ_JEWELLERY :
                  (temp_rand == 7)                    ? OBJ_BOOKS
                  /* (temp_rand == 8) */              : OBJ_STAVES);

        const int treasure_item =
            items( 1, glopop, OBJ_RANDOM, true, 
                   you.your_level * 3, MAKE_ITEM_RANDOM_RACE );

        if (treasure_item != NON_ITEM)
        {
            mitm[treasure_item].x = end.x;
            mitm[treasure_item].y = end.y;
        }
    }
}

static void labyrinth_place_exit(const coord_def &end)
{
    labyrinth_place_items(end);
    mons_place( MONS_MINOTAUR, BEH_SLEEP, MHITNOT, true, end.x, end.y );
    grd(end) = DNGN_ROCK_STAIRS_UP;
}

static void init_minivault_placement(int vault, vault_placement &place)
{
    map_type vgrid;
    vault_main(vgrid, place, vault);
}

static void change_walls_from_centre(const dgn_region &region,
                                     const coord_def &centre,
                                     bool  rectangular,
                                     unsigned mmask,
                                     dungeon_feature_type wall,
                                     const std::vector<dist_feat> &ldist)
{
    if (ldist.empty())
        return;

    const coord_def &end = region.pos + region.size;
    for (int y = region.pos.y; y < end.y; ++y)
    {
        for (int x = region.pos.x; x < end.x; ++x)
        {
            const coord_def c(x, y);
            if (grd(c) != wall || !unforbidden(c, mmask))
                continue;

            const int distance =
                rectangular? (c - centre).rdist() : (c - centre).abs();

            for (int i = 0, size = ldist.size(); i < size; ++i)
            {
                if (distance <= ldist[i].dist)
                {
                    grd(c) = ldist[i].feat;
                    break;
                }
            }
        }
    }
}

// Called as:
// change_walls_from_centre( region_affected, centre, rectangular, wall,
//                           dist1, feat1, dist2, feat2, ..., 0 )
// What it does:
// Examines each square in region_affected, calculates its distance from
// "centre" (the centre need not be in region_affected). If the distance is
// less than or equal to dist1, and the feature == wall, then it is replaced
// by feat1. Otherwise, if the distance <= dist2 and feature == wall, it is
// replaced by feat2, and so on. A distance of 0 indicates the end of the
// list of distances.
//
static void change_walls_from_centre(const dgn_region &region,
                                     const coord_def &c,
                                     bool  rectangular,
                                     dungeon_feature_type wall,
                                     ...)
{
    std::vector<dist_feat> ldist;
    
    va_list args;
    va_start(args, wall);

    while (true)
    {
        const int dist = va_arg(args, int);
        if (!dist)
            break;
        const dungeon_feature_type feat =
            static_cast<dungeon_feature_type>( va_arg(args, int) );
        ldist.push_back(dist_feat(dist, feat));
    }

    change_walls_from_centre(region, c, rectangular, MMT_VAULT, wall, ldist);
}

static void place_extra_lab_minivaults(int level_number)
{
    std::set<int> vaults_used;
    while (true)
    {
        const int vault = random_map_for_tag("lab", true, false);
        if (vault == -1 || vaults_used.find(vault) != vaults_used.end())
            break;

        vaults_used.insert(vault);
        if (!build_minivaults(level_number, vault))
            break;
    }
}

// Checks if there is a square with the given mask within radius of pos.
static bool has_vault_in_radius(const coord_def &pos, int radius,
                                unsigned mask)
{
    for (int yi = -radius; yi <= radius; ++yi)
    {
        for (int xi = -radius; xi <= radius; ++xi)
        {
            const coord_def p = pos + coord_def(xi, yi);
            if (!in_bounds(p))
                continue;
            if (!unforbidden(p, mask))
                return (true);
        }
    }
    return (false);
}

// Find an entry point that's:
// * At least 25 squares away from the exit.
// * At least 4 squares away from the nearest vault.
// * Floor (well, obviously).
static coord_def labyrinth_find_entry_point(const dgn_region &reg,
                                            const coord_def &end)
{
    const int min_distance = 20 * 20;
    // Try many times.
    for (int i = 0; i < 2000; ++i)
    {
        const coord_def place = reg.random_point();
        if (grd(place) != DNGN_FLOOR)
            continue;

        if ((place - end).abs() < min_distance)
            continue;

        if (has_vault_in_radius(place, 4, MMT_VAULT))
            continue;

        return (place);
    }
    coord_def unfound;
    return (unfound);
}

static void labyrinth_place_entry_point(const dgn_region &region,
                                        const coord_def &pos)
{
    const coord_def p = labyrinth_find_entry_point(region, pos);
    if (in_bounds(p))
        env.markers.add(new map_feature_marker(p, DNGN_ENTER_LABYRINTH));
}

static void labyrinth_level(int level_number)
{
    dgn_region lab =
        dgn_region::absolute( LABYRINTH_BORDER,
                              LABYRINTH_BORDER,
                              GXM - LABYRINTH_BORDER - 1,
                              GYM - LABYRINTH_BORDER - 1 );
    
    // First decide if we're going to use a Lab minivault.
    int vault = random_map_for_tag("minotaur", true, false);
    vault_placement place;
    if (vault != -1)
        init_minivault_placement(vault, place);
    
    coord_def end;
    labyrinth_build_maze(end, lab);

    if (vault == -1 || !build_minivaults(level_number, vault))
    {
        vault = -1;
        labyrinth_place_exit(end);
    }
    else
    {
        const vault_placement &rplace = *(level_vaults.end() - 1);
        if (rplace.map.has_tag("generate_loot"))
        {
            for (int y = rplace.pos.y;
                 y <= rplace.pos.y + rplace.size.y - 1; ++y)
            {
                for (int x = rplace.pos.x;
                     x <= rplace.pos.x + rplace.size.x - 1; ++x)
                {
                    if (grd[x][y] == DNGN_ROCK_STAIRS_UP)
                    {
                        labyrinth_place_items(coord_def(x, y));
                        break;
                    }
                }
            }
        }
        place.pos  = rplace.pos;
        place.size = rplace.size;
    }
    
    if (vault != -1)
        end = place.pos + place.size / 2;

    place_extra_lab_minivaults(level_number);

    change_walls_from_centre(lab, end, false,
                             DNGN_ROCK_WALL,
                             15 * 15, DNGN_METAL_WALL,
                             34 * 34, DNGN_STONE_WALL,
                             0);

    labyrinth_place_entry_point(lab, end);
    
    // turn rock walls into undiggable stone or metal:
    // dungeon_feature_type wall_xform =
    //    ((random2(50) > 10) ? DNGN_STONE_WALL   // 78.0%
    //     : DNGN_METAL_WALL); // 22.0%
    //replace_area(0, 0, GXM - 1, GYM - 1, DNGN_ROCK_WALL, wall_xform, vaults);
     
    link_items();
}                               // end labyrinth_level()

static bool is_wall(int x, int y)
{
    unsigned char feat = grd[x][y];

    switch (feat)
    {
        case DNGN_ROCK_WALL:
        case DNGN_STONE_WALL:
        case DNGN_METAL_WALL:
        case DNGN_GREEN_CRYSTAL_WALL:
        case DNGN_WAX_WALL:
        case DNGN_CLEAR_ROCK_WALL:
        case DNGN_CLEAR_STONE_WALL:
        case DNGN_CLEAR_PERMAROCK_WALL:
            return true;
        default:
            return false;
    }
}


static int box_room_door_spot(int x, int y)
{
    // if there is a door near us embedded in rock, we have to be a door too.
    if ((grd[x-1][y] == DNGN_CLOSED_DOOR && is_wall(x-1,y-1) && is_wall(x-1,y+1))
     || (grd[x+1][y] == DNGN_CLOSED_DOOR && is_wall(x+1,y-1) && is_wall(x+1,y+1))
     || (grd[x][y-1] == DNGN_CLOSED_DOOR && is_wall(x-1,y-1) && is_wall(x+1,y-1))
     || (grd[x][y+1] == DNGN_CLOSED_DOOR && is_wall(x-1,y+1) && is_wall(x+1,y+1)))
    {
        grd[x][y] = DNGN_CLOSED_DOOR;
        return 2;
    }

    // to be a good spot for a door, we need non-wall on two sides and
    // wall on two sides.
    bool nor = is_wall(x, y-1);
    bool sou = is_wall(x, y+1);
    bool eas = is_wall(x-1, y);
    bool wes = is_wall(x+1, y);

    if (nor == sou && eas == wes && nor != eas)
        return 1;

    return 0;
}

static int box_room_doors( int bx1, int bx2, int by1, int by2, int new_doors)
{
    int good_doors[200];        // 1 == good spot, 2 == door placed!
    int spot;
    int i,j;
    int doors_placed = new_doors;

    // sanity
    if ( 2 * ( (bx2 - bx1) + (by2-by1) ) > 200)
        return 0;

    // go through, building list of good door spots, and replacing wall
    // with door if we're about to block off another door.
    int spot_count = 0;

    // top & bottom
    for(i=bx1+1; i<bx2; i++)
    {
        good_doors[spot_count ++] = box_room_door_spot(i, by1);
        good_doors[spot_count ++] = box_room_door_spot(i, by2);
    }
    // left & right
    for(i=by1+1; i<by2; i++)
    {
        good_doors[spot_count ++] = box_room_door_spot(bx1, i);
        good_doors[spot_count ++] = box_room_door_spot(bx2, i);
    }

    if (new_doors == 0)
    {
        // count # of doors we HAD to place
        for(i=0; i<spot_count; i++)
            if (good_doors[i] == 2)
                doors_placed++;

        return doors_placed;
    }

    // Avoid an infinite loop if there are not enough good spots. --KON
    j = 0;
    for (i=0; i<spot_count; i++)
        if (good_doors[i] == 1)
            j++;
    if (new_doors > j)
        new_doors = j;

    while(new_doors > 0 && spot_count > 0)
    {
        spot = random2(spot_count);
        if (good_doors[spot] != 1)
            continue;

        j = 0;
        for(i=bx1+1; i<bx2; i++)
        {
            if (spot == j++)
            {
                grd[i][by1] = DNGN_CLOSED_DOOR;
                break;
            }
            if (spot == j++)
            {
                grd[i][by2] = DNGN_CLOSED_DOOR;
                break;
            }
        }

        for(i=by1+1; i<by2; i++)
        {
            if (spot == j++)
            {
                grd[bx1][i] = DNGN_CLOSED_DOOR;
                break;
            }
            if (spot == j++)
            {
                grd[bx2][i] = DNGN_CLOSED_DOOR;
                break;
            }
        }

        // try not to put a door in the same place twice
        good_doors[spot] = 2;
        new_doors --;
    }

    return doors_placed;
}


static void box_room(int bx1, int bx2, int by1, int by2,
                     dungeon_feature_type wall_type)
{
    // hack -- avoid lava in the crypt. {gdl}
    if ((player_in_branch( BRANCH_CRYPT ) || player_in_branch( BRANCH_TOMB ))
         && wall_type == DNGN_LAVA)
    {
        wall_type = DNGN_SHALLOW_WATER;
    }

    int temp_rand, new_doors, doors_placed;

    // do top & bottom walls
    replace_area(bx1,by1,bx2,by1,DNGN_FLOOR,wall_type);
    replace_area(bx1,by2,bx2,by2,DNGN_FLOOR,wall_type);

    // do left & right walls
    replace_area(bx1,by1+1,bx1,by2-1,DNGN_FLOOR,wall_type);
    replace_area(bx2,by1+1,bx2,by2-1,DNGN_FLOOR,wall_type);

    // sometimes we have to place doors, or else we shut in other
    // buildings' doors
    doors_placed = box_room_doors(bx1, bx2, by1, by2, 0);

    temp_rand = random2(100);
    new_doors = (temp_rand > 45) ? 2 :
                ((temp_rand > 22) ? 1 : 3);

    // small rooms don't have as many doors
    if ((bx2-bx1)*(by2-by1) < 36 && new_doors > 1)
        new_doors--;

    new_doors -= doors_placed;
    if (new_doors > 0)
        box_room_doors(bx1, bx2, by1, by2, new_doors);
}

static void city_level(int level_number)
{
    int temp_rand;          // probability determination {dlb}
    // remember, can have many wall types in one level
    dungeon_feature_type wall_type;
    // simplifies logic of innermost loop {dlb}
    dungeon_feature_type wall_type_room;

    int xs = 0, ys = 0;
    int x1 = 0, x2 = 0;
    int y1 = 0, y2 = 0;
    int i,j;

    temp_rand = random2(8);

    wall_type = ((temp_rand > 4) ? DNGN_ROCK_WALL :     // 37.5% {dlb}
                 (temp_rand > 1) ? DNGN_STONE_WALL      // 37.5% {dlb}
                                 : DNGN_METAL_WALL);    // 25.0% {dlb}

    if (one_chance_in(100))
        wall_type = DNGN_GREEN_CRYSTAL_WALL;

    make_box( 7, 7, GXM-7, GYM-7, DNGN_FLOOR );

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 4; j++)
        {
            xs = 8 + (i * 13);
            ys = 8 + (j * 14);
            x1 = xs + random2avg(5, 2);
            y1 = ys + random2avg(5, 2);
            x2 = xs + 11 - random2avg(5, 2);
            y2 = ys + 11 - random2avg(5, 2);

            temp_rand = random2(280);

            if (temp_rand > 39) // 85.7% draw room(s) {dlb}
            {
                wall_type_room = ((temp_rand > 63) ? wall_type :       // 77.1%
                                  (temp_rand > 54) ? DNGN_STONE_WALL : //  3.2%
                                  (temp_rand > 45) ? DNGN_ROCK_WALL    //  3.2%
                                                   : DNGN_METAL_WALL); //  2.1%

                if (one_chance_in(250))
                    wall_type_room = DNGN_GREEN_CRYSTAL_WALL;

                box_room(x1, x2, y1, y2, wall_type_room);

                // inner room - neat.
                if (x2 - x1 > 5 && y2 - y1 > 5 && one_chance_in(8))
                {
                    box_room(x1 + 2, x2 - 2, y1 + 2, y2 - 2, wall_type);

                    // treasure area.. neat.
                    if (one_chance_in(3))
                        treasure_area(level_number, x1 + 3, x2 - 3,
                                      y1 + 3, y2 - 3);
                }
            }
        }
    }

    int stair_count = coinflip() ? 2 : 1;

    for (j = 0; j < stair_count; j++)
    {
        for (i = 0; i < 2; i++)
        {
            place_specific_stair(
                static_cast<dungeon_feature_type>(
                    j + ((i==0) ? DNGN_STONE_STAIRS_DOWN_I
                         : DNGN_STONE_STAIRS_UP_I) ) );
        }
    }

}                               // end city_level()

static bool treasure_area(int level_number, unsigned char ta1_x,
                          unsigned char ta2_x, unsigned char ta1_y,
                          unsigned char ta2_y)
{
    int x_count = 0;
    int y_count = 0;
    int item_made = 0;

    ta2_x++;
    ta2_y++;

    if (ta2_x <= ta1_x || ta2_y <= ta1_y)
        return false;

    if ((ta2_x - ta1_x) * (ta2_y - ta1_y) >= 40)
        return false;

    for (x_count = ta1_x; x_count < ta2_x; x_count++)
    {
        for (y_count = ta1_y; y_count < ta2_y; y_count++)
        {
            if (grd[x_count][y_count] != DNGN_FLOOR || coinflip())
                continue;

            item_made = items( 1, OBJ_RANDOM, OBJ_RANDOM, true,
                               random2( level_number * 2 ), MAKE_ITEM_RANDOM_RACE );

            if (item_made != NON_ITEM)
            {
                mitm[item_made].x = x_count;
                mitm[item_made].y = y_count;
            }
        }
    }

    return true;
}                               // end treasure_area()

static void diamond_rooms(int level_number)
{
    char numb_diam = 1 + random2(10);
    dungeon_feature_type type_floor = DNGN_DEEP_WATER;
    int runthru = 0;
    int i, oblique_max;

    // I guess no diamond rooms in either of these places {dlb}:
    if (player_in_branch( BRANCH_DIS ) || player_in_branch( BRANCH_TARTARUS ))
        return;

    if (level_number > 5 + random2(5) && coinflip())
        type_floor = DNGN_SHALLOW_WATER;

    if (level_number > 10 + random2(5) && coinflip())
        type_floor = DNGN_DEEP_WATER;

    if (level_number > 17 && coinflip())
        type_floor = DNGN_LAVA;

    if (level_number > 10 && one_chance_in(15))
        type_floor = (coinflip()? DNGN_STONE_WALL : DNGN_ROCK_WALL);

    if (level_number > 12 && one_chance_in(20))
        type_floor = DNGN_METAL_WALL;

    if (player_in_branch( BRANCH_GEHENNA ))
        type_floor = DNGN_LAVA;
    else if (player_in_branch( BRANCH_COCYTUS ))
        type_floor = DNGN_DEEP_WATER;

    for (i = 0; i < numb_diam; i++)
    {
        spec_room sr;

        sr.x1 = 8 + random2(43);
        sr.y1 = 8 + random2(35);
        sr.x2 = sr.x1 + 5 + random2(15);
        sr.y2 = sr.y1 + 5 + random2(10);

        oblique_max = (sr.x2 - sr.x1) / 2;      //random2(20) + 5;

        if (!octa_room(sr, oblique_max, type_floor))
        {
            runthru++;
            if (runthru > 9)
            {
                runthru = 0;
            }
            else
            {
                i--;
                continue;
            }
        }
    }                           // end "for(bk...)"
}                               // end diamond_rooms()

static void big_room(int level_number)
{
    dungeon_feature_type type_floor = DNGN_FLOOR;
    dungeon_feature_type type_2 = DNGN_FLOOR;
    int i, j, k, l;

    spec_room sr;
    int oblique;

    if (one_chance_in(4))
    {
        oblique = 5 + random2(20);

        sr.x1 = 8 + random2(30);
        sr.y1 = 8 + random2(22);
        sr.x2 = sr.x1 + 20 + random2(10);
        sr.y2 = sr.y1 + 20 + random2(8);

        // usually floor, except at higher levels
        if (!one_chance_in(5) || level_number < 8 + random2(8))
        {
            octa_room(sr, oblique, DNGN_FLOOR);
            return;
        }

        // default is lava.
        type_floor = DNGN_LAVA;

        if (level_number > 7)
        {
            type_floor = ((random2(level_number) < 14) ? DNGN_DEEP_WATER
                                                       : DNGN_LAVA);
        }

        octa_room(sr, oblique, type_floor);
    }

    // what now?
    sr.x1 = 8 + random2(30);
    sr.y1 = 8 + random2(22);
    sr.x2 = sr.x1 + 20 + random2(10);
    sr.y2 = sr.y1 + 20 + random2(8);

    // check for previous special
    if (find_in_area(sr.x1, sr.y1, sr.x2, sr.y2, DNGN_BUILDER_SPECIAL_WALL))
        return;

    if (level_number > 7 && one_chance_in(4))
    {
        type_floor = ((random2(level_number) < 14) ? DNGN_DEEP_WATER
                                                   : DNGN_LAVA);
    }

    // make the big room.
    replace_area(sr.x1, sr.y1, sr.x2, sr.y2, DNGN_ROCK_WALL, type_floor);
    replace_area(sr.x1, sr.y1, sr.x2, sr.y2, DNGN_CLOSED_DOOR, type_floor);

    if (type_floor == DNGN_FLOOR)
    {
        const int minwall = DNGN_RNDWALL_MIN;
        const int range   = DNGN_RNDWALL_MAX - DNGN_RNDWALL_MIN + 1;
        type_2 = static_cast<dungeon_feature_type>(minwall + random2(range));
    }

    // no lava in the Crypt or Tomb, thanks!
    if (player_in_branch( BRANCH_CRYPT ) || player_in_branch( BRANCH_TOMB ))
    {
        if (type_floor == DNGN_LAVA)
            type_floor = DNGN_SHALLOW_WATER;

        if (type_2 == DNGN_LAVA)
            type_2 = DNGN_SHALLOW_WATER;
    }

    // sometimes make it a chequerboard
    if (one_chance_in(4))
    {
        chequerboard( sr, type_floor, type_floor, type_2 );
    }
    // sometimes make an inside room w/ stone wall.
    else if (one_chance_in(6))
    {
        i = sr.x1;
        j = sr.y1;
        k = sr.x2;
        l = sr.y2;

        do
        {
            i += 2 + random2(3);
            j += 2 + random2(3);
            k -= 2 + random2(3);
            l -= 2 + random2(3);
            // check for too small
            if (i >= k - 3)
                break;
            if (j >= l - 3)
                break;

            box_room(i, k, j, l, DNGN_STONE_WALL);

        }
        while (level_number < 1500);       // ie forever
    }
}                               // end big_room()

// helper function for chequerboard rooms
// note that box boundaries are INclusive
static void chequerboard( spec_room &sr, dungeon_feature_type target,
                          dungeon_feature_type floor1,
                          dungeon_feature_type floor2 )
{
    int i, j;

    if (sr.x2 < sr.x1 || sr.y2 < sr.y1)
        return;

    for (i = sr.x1; i <= sr.x2; i++)
    {
        for (j = sr.y1; j <= sr.y2; j++)
        {
            if (grd[i][j] == target)
                grd[i][j] = (((i + j) % 2) ? floor2 : floor1);
        }
    }
}                               // end chequerboard()

static void roguey_level(int level_number, spec_room &sr)
{
    int bcount_x, bcount_y;
    int cn = 0;
    int i;

    FixedVector < unsigned char, 30 > rox1;
    FixedVector < unsigned char, 30 > rox2;
    FixedVector < unsigned char, 30 > roy1;
    FixedVector < unsigned char, 30 > roy2;

    for (bcount_y = 0; bcount_y < 5; bcount_y++)
    {
        for (bcount_x = 0; bcount_x < 5; bcount_x++)
        {
            // rooms:
            rox1[cn] = bcount_x * 13 + 8 + random2(4);
            roy1[cn] = bcount_y * 11 + 8 + random2(4);

            rox2[cn] = rox1[cn] + 3 + random2(8);
            roy2[cn] = roy1[cn] + 3 + random2(6);

            // bounds
            if (rox2[cn] > GXM-8)
                rox2[cn] = GXM-8;

            cn++;
        }
    }

    cn = 0;

    for (i = 0; i < 25; i++)
    {
        replace_area( rox1[i], roy1[i], rox2[i], roy2[i], 
                      DNGN_ROCK_WALL, DNGN_FLOOR );

        // inner room?
        if (rox2[i] - rox1[i] > 5 && roy2[i] - roy1[i] > 5)
        {
            if (random2(100 - level_number) < 3)
            {
                if (!one_chance_in(4))
                {
                    box_room( rox1[i] + 2, rox2[i] - 2, roy1[i] + 2,
                              roy2[i] - 2, (coinflip() ? DNGN_STONE_WALL
                                                       : DNGN_ROCK_WALL) );
                }
                else
                {
                    box_room( rox1[i] + 2, rox2[i] - 2, roy1[i] + 2,
                                 roy2[i] - 2, DNGN_METAL_WALL );
                }

                if (coinflip())
                {
                    treasure_area( level_number, rox1[i] + 3, rox2[i] - 3,
                                                 roy1[i] + 3, roy2[i] - 3 );
                }
            }
        }
    }                           // end "for i"

    // Now, join them together:
    FixedVector < char, 2 > pos;
    FixedVector < char, 2 > jpos;

    char doing = 0;

    char last_room = 0;
    int bp;

    for (bp = 0; bp < 2; bp++)
    {
        for (i = 0; i < 25; i++)
        {
            if (bp == 0 && (!(i % 5) || i == 0))
                continue;

            if (bp == 1 && i < 5)
                continue;

            switch (bp)
            {
            case 0:
                last_room = i - 1;
                pos[0] = rox1[i];      // - 1;
                pos[1] = roy1[i] + random2(roy2[i] - roy1[i]);
                jpos[0] = rox2[last_room];      // + 1;
                jpos[1] = roy1[last_room]
                                + random2(roy2[last_room] - roy1[last_room]);
                break;

            case 1:
                last_room = i - 5;
                pos[1] = roy1[i];      // - 1;
                pos[0] = rox1[i] + random2(rox2[i] - rox1[i]);
                jpos[1] = roy2[last_room];      // + 1;
                jpos[0] = rox1[last_room]
                                + random2(rox2[last_room] - rox1[last_room]);
                break;
            }

            while (pos[0] != jpos[0] || pos[1] != jpos[1])
            {
                doing = (coinflip()? 1 : 0);

                if (pos[doing] < jpos[doing])
                    pos[doing]++;
                else if (pos[doing] > jpos[doing])
                    pos[doing]--;

                if (grd[pos[0]][pos[1]] == DNGN_ROCK_WALL)
                    grd[pos[0]][pos[1]] = DNGN_FLOOR;
            }

            if (grd[pos[0]][pos[1]] == DNGN_FLOOR)
            {
                if ((grd[pos[0] + 1][pos[1]] == DNGN_ROCK_WALL
                        && grd[pos[0] - 1][pos[1]] == DNGN_ROCK_WALL)
                    || (grd[pos[0]][pos[1] + 1] == DNGN_ROCK_WALL
                        && grd[pos[0]][pos[1] - 1] == DNGN_ROCK_WALL))
                {
                    grd[pos[0]][pos[1]] = DNGN_GRANITE_STATUE;
                }
            }
        }                       // end "for bp, for i"
    }

    // is one of them a special room?
    if (level_number > 8 && one_chance_in(10))
    {
        int spec_room_done = random2(25);

        sr.created = true;
        sr.hooked_up = true;
        sr.x1 = rox1[spec_room_done];
        sr.x2 = rox2[spec_room_done];
        sr.y1 = roy1[spec_room_done];
        sr.y2 = roy2[spec_room_done];
        special_room( level_number, sr );

        // make the room 'special' so it doesn't get overwritten
        // by something else (or put monsters in walls, etc..).

        // top
        replace_area(sr.x1-1, sr.y1-1, sr.x2+1,sr.y1-1, DNGN_ROCK_WALL, DNGN_BUILDER_SPECIAL_WALL);
        replace_area(sr.x1-1, sr.y1-1, sr.x2+1,sr.y1-1, DNGN_FLOOR, DNGN_BUILDER_SPECIAL_FLOOR);
        replace_area(sr.x1-1, sr.y1-1, sr.x2+1,sr.y1-1, DNGN_CLOSED_DOOR, DNGN_BUILDER_SPECIAL_FLOOR);

        // bottom
        replace_area(sr.x1-1, sr.y2+1, sr.x2+1,sr.y2+1, DNGN_ROCK_WALL, DNGN_BUILDER_SPECIAL_WALL);
        replace_area(sr.x1-1, sr.y2+1, sr.x2+1,sr.y2+1, DNGN_FLOOR, DNGN_BUILDER_SPECIAL_FLOOR);
        replace_area(sr.x1-1, sr.y2+1, sr.x2+1,sr.y2+1, DNGN_CLOSED_DOOR, DNGN_BUILDER_SPECIAL_FLOOR);

        // left
        replace_area(sr.x1-1, sr.y1-1, sr.x1-1, sr.y2+1, DNGN_ROCK_WALL, DNGN_BUILDER_SPECIAL_WALL);
        replace_area(sr.x1-1, sr.y1-1, sr.x1-1, sr.y2+1, DNGN_FLOOR, DNGN_BUILDER_SPECIAL_FLOOR);
        replace_area(sr.x1-1, sr.y1-1, sr.x1-1, sr.y2+1, DNGN_CLOSED_DOOR, DNGN_BUILDER_SPECIAL_FLOOR);

        // right
        replace_area(sr.x2+1, sr.y1-1, sr.x2+1, sr.y2+1, DNGN_ROCK_WALL, DNGN_BUILDER_SPECIAL_WALL);
        replace_area(sr.x2+1, sr.y1-1, sr.x2+1, sr.y2+1, DNGN_FLOOR, DNGN_BUILDER_SPECIAL_FLOOR);
        replace_area(sr.x2+1, sr.y1-1, sr.x2+1, sr.y2+1, DNGN_CLOSED_DOOR, DNGN_BUILDER_SPECIAL_FLOOR);
    }

    int stair_count = coinflip() ? 2 : 1;

    for (int j = 0; j < stair_count; j++)
    {
        for (i = 0; i < 2; i++)
        {
            place_specific_stair(
                static_cast<dungeon_feature_type>(
                    j + ((i==0) ? DNGN_STONE_STAIRS_DOWN_I
                         : DNGN_STONE_STAIRS_UP_I)));
        }
    }
}                               // end roguey_level()

static void morgue(spec_room &sr)
{
    int temp_rand = 0;          // probability determination {dlb}
    int x,y;

    for (x = sr.x1; x <= sr.x2; x++)
    {
        for (y = sr.y1; y <= sr.y2; y++)
        {
            if (grd[x][y] == DNGN_FLOOR || grd[x][y] == DNGN_BUILDER_SPECIAL_FLOOR)
            {
                int mon_type;
                temp_rand = random2(24);

                mon_type =  ((temp_rand > 11) ? MONS_ZOMBIE_SMALL :  // 50.0%
                             (temp_rand >  7) ? MONS_WIGHT :         // 16.7%
                             (temp_rand >  3) ? MONS_NECROPHAGE :    // 16.7%
                             (temp_rand >  0) ? MONS_WRAITH          // 12.5%
                                              : MONS_VAMPIRE);       //  4.2%

                mons_place( mon_type, BEH_SLEEP, MHITNOT, true, x, y );
            }
        }
    }
}                               // end morgue()

static void jelly_pit(int level_number, spec_room &sr)
{
    FixedVector< pit_mons_def, MAX_PIT_MONSTERS >  pit_list;
    const int lordx = sr.x1 + random2(sr.x2 - sr.x1);
    const int lordy = sr.y1 + random2(sr.y2 - sr.y1);

    for (int i = 0; i < MAX_PIT_MONSTERS; i++)
    {
        pit_list[i].type = MONS_PROGRAM_BUG;
        pit_list[i].rare = 0;
    }
    
#if DEBUG_DIAGNOSTICS
    mprf( MSGCH_DIAGNOSTICS, "Build: Jelly Pit" );
#endif
    pit_list[0].type = MONS_OOZE;
    pit_list[0].rare = 27 - level_number / 5;

    pit_list[1].type = MONS_JELLY;
    pit_list[1].rare = 20;

    pit_list[2].type = MONS_BROWN_OOZE;
    pit_list[2].rare = 3 + level_number;

    pit_list[3].type = MONS_DEATH_OOZE;
    pit_list[3].rare = 2 + (2 * level_number) / 3;

    if (level_number >= 12)
    {
        pit_list[4].type = MONS_AZURE_JELLY;
        pit_list[4].rare = 1 + (level_number - 12) / 3;
    }

    if (level_number >= 15)
    {
        pit_list[5].type = MONS_ACID_BLOB;
        pit_list[5].rare = 1 + (level_number - 15) / 4;
    }

    fill_monster_pit( sr, pit_list, 90, MONS_PROGRAM_BUG, lordx, lordy );
}

bool place_specific_trap(int spec_x, int spec_y,
                         trap_type spec_type)
{
    if (spec_type == TRAP_RANDOM || spec_type == TRAP_NONTELEPORT)
    {
        trap_type forbidden1 = NUM_TRAPS;
        trap_type forbidden2 = NUM_TRAPS;

        if (spec_type == TRAP_NONTELEPORT)
        {
            forbidden1 = TRAP_SHAFT;
            forbidden2 = TRAP_TELEPORT;
        }
        else if (!is_valid_shaft_level())
            forbidden1 = TRAP_SHAFT;

        do
        {
            spec_type = static_cast<trap_type>( random2(NUM_TRAPS) );
        } while (spec_type == forbidden1 || spec_type == forbidden2);
    }

    for (int tcount = 0; tcount < MAX_TRAPS; tcount++)
    {
        if (env.trap[tcount].type == TRAP_UNASSIGNED)
        {
            env.trap[tcount].type = spec_type;
            env.trap[tcount].x = spec_x;
            env.trap[tcount].y = spec_y;
            grd[spec_x][spec_y] = DNGN_UNDISCOVERED_TRAP;
            return true;
        }
    }

    return false;
}                               // end place_specific_trap()

void define_zombie( int mid, int ztype, int cs, int power )
{
    int mons_sec2 = 0;
    int zombie_size = 0;
    bool ignore_rarity = false;
    int test, cls;

    if (power > 27)
        power = 27;

    // set size based on zombie class (cs)
    switch(cs)
    {
        case MONS_ZOMBIE_SMALL:
        case MONS_SIMULACRUM_SMALL:
        case MONS_SKELETON_SMALL:
            zombie_size = Z_SMALL;
            break;

        case MONS_ZOMBIE_LARGE:
        case MONS_SIMULACRUM_LARGE:
        case MONS_SKELETON_LARGE:
            zombie_size = Z_BIG;
            break;

        case MONS_SPECTRAL_THING:
            zombie_size = -1;
            break;

        default:
            // this should NEVER happen.
            perror("\ncreate_zombie() got passed incorrect zombie type!\n");
            end(0);
            break;
    }

    // that is, random creature from which to fashion undead
    if (ztype == 250)
    {
        // how OOD this zombie can be.
        int relax = 5;

        // pick an appropriate creature to make a zombie out of,
        // levelwise.  The old code was generating absolutely
        // incredible OOD zombies.
        while(true)
        {
            // this limit can be updated if mons->number goes >8 bits..
            test = random2(182);            // not guaranteed to be valid, so..
            cls = mons_species(test);
            if (cls == MONS_PROGRAM_BUG)
                continue;

            // on certain branches, zombie creation will fail if we use
            // the mons_rarity() functions, because (for example) there
            // are NO zombifiable "native" abyss creatures. Other branches
            // where this is a problem are hell levels and the crypt.
            // we have to watch for summoned zombies on other levels, too,
            // such as the Temple, HoB, and Slime Pits.
            if (you.level_type != LEVEL_DUNGEON
                || player_in_hell()
                || player_in_branch( BRANCH_HALL_OF_ZOT )
                || player_in_branch( BRANCH_VESTIBULE_OF_HELL )
                || player_in_branch( BRANCH_ECUMENICAL_TEMPLE )
                || player_in_branch( BRANCH_CRYPT )
                || player_in_branch( BRANCH_TOMB )
                || player_in_branch( BRANCH_HALL_OF_BLADES )
                || player_in_branch( BRANCH_SNAKE_PIT )
                || player_in_branch( BRANCH_SLIME_PITS )
                || one_chance_in(1000))
            {
                ignore_rarity = true;
            }

            // don't make out-of-rarity zombies when we don't have to
            if (!ignore_rarity && mons_rarity(cls) == 0)
                continue;

            // monster class must be zombifiable
            if (!mons_zombie_size(cls))
                continue;

            // if skeleton, monster must have a skeleton
            if ((cs == MONS_SKELETON_SMALL || cs == MONS_SKELETON_LARGE)
                && !mons_skeleton(cls))
            {
                continue;
            }

            // size must match, but you can make a spectral thing out of anything.
            if (mons_zombie_size(cls) != zombie_size && zombie_size != -1)
                continue;

            // hack -- non-dungeon zombies are always made out of nastier
            // monsters
            if (you.level_type != LEVEL_DUNGEON && mons_power(cls) > 8)
                break;

            // check for rarity.. and OOD - identical to mons_place()
            int level, diff, chance;

            level  = mons_level( cls ) - 4;
            diff   = level - power;

            chance = (ignore_rarity) ? 100
                                     : mons_rarity(cls) - (diff * diff) / 2;

            if (power > level - relax && power < level + relax
                && random2avg(100, 2) <= chance)
            {
                break;
            }

            // every so often, we'll relax the OOD restrictions.  Avoids
            // infinite loops (if we don't do this, things like creating
            // a large skeleton on level 1 may hang the game!)
            if (one_chance_in(5))
                relax++;
        }

        // set type and secondary appropriately
        menv[mid].number = cls;
        mons_sec2 = cls;
    }
    else
    {
        menv[mid].number = mons_species(ztype);
        mons_sec2 = menv[mid].number;
    }

    menv[mid].type = menv[mid].number;

    define_monster(mid);

    menv[mid].hit_points = hit_points( menv[mid].hit_dice, 6, 5 );
    menv[mid].max_hit_points = menv[mid].hit_points;

    menv[mid].ac -= 2;

    if (menv[mid].ac < 0)
        menv[mid].ac = 0;

    menv[mid].ev -= 5;

    if (menv[mid].ev < 0)
        menv[mid].ev = 0;

    menv[mid].speed -= 2;

    if (menv[mid].speed < 3)
        menv[mid].speed = 3;

    menv[mid].speed_increment = 70;

    if (cs == MONS_ZOMBIE_SMALL || cs == MONS_ZOMBIE_LARGE)
    {
        menv[mid].type = ((mons_zombie_size(menv[mid].number) == Z_BIG)
                                    ? MONS_ZOMBIE_LARGE : MONS_ZOMBIE_SMALL);
    }
    else if (cs == MONS_SKELETON_SMALL || cs == MONS_SKELETON_LARGE)
    {
        menv[mid].hit_points = hit_points( menv[mid].hit_dice, 5, 4 );
        menv[mid].max_hit_points = menv[mid].hit_points;

        menv[mid].ac -= 4;

        if (menv[mid].ac < 0)
            menv[mid].ac = 0;

        menv[mid].ev -= 2;

        if (menv[mid].ev < 0)
            menv[mid].ev = 0;

        menv[mid].type = ((mons_zombie_size( menv[mid].number ) == Z_BIG)
                            ? MONS_SKELETON_LARGE : MONS_SKELETON_SMALL);
    }
    else if (cs == MONS_SIMULACRUM_SMALL || cs == MONS_SIMULACRUM_LARGE)
    {
        // Simulacrum aren't tough, but you can create piles of them. -- bwr
        menv[mid].hit_points = hit_points( menv[mid].hit_dice, 1, 4 );
        menv[mid].max_hit_points = menv[mid].hit_points;
        menv[mid].type = ((mons_zombie_size( menv[mid].number ) == Z_BIG)
                            ? MONS_SIMULACRUM_LARGE : MONS_SIMULACRUM_SMALL);
    }
    else if (cs == MONS_SPECTRAL_THING)
    {
        menv[mid].hit_points = hit_points( menv[mid].hit_dice, 4, 4 );
        menv[mid].max_hit_points = menv[mid].hit_points;
        menv[mid].ac += 4;
        menv[mid].type = MONS_SPECTRAL_THING;
    }

    menv[mid].number = mons_sec2;
    menv[mid].colour = mons_class_colour(cs);
}                               // end define_zombie()

static void build_river( dungeon_feature_type river_type ) //mv
{
    int i,j;
    int y, width;

    if (player_in_branch( BRANCH_CRYPT ) || player_in_branch( BRANCH_TOMB ))
        return;

    // if (one_chance_in(10)) 
    //     build_river(river_type); 

    // Made rivers less wide... min width five rivers were too annoying. -- bwr
    width = 3 + random2(4);
    y = 10 - width + random2avg( GYM-10, 3 );

    for (i = 5; i < (GXM - 5); i++)
    {
        if (one_chance_in(3))   y++;
        if (one_chance_in(3))   y--;
        if (coinflip())         width++;
        if (coinflip())         width--;

        if (width < 2) width = 2;
        if (width > 6) width = 6;

        for (j = y; j < y+width ; j++)
        {
            if (j >= 5 && j <= GYM - 5)
            {
                // Note that vaults might have been created in this area!
                // So we'll avoid the silliness of orcs/royal jelly on
                // lava and deep water grids. -- bwr
                if (!one_chance_in(200)
                    && (grd[i][j] < DNGN_ENTER_SHOP
                        || grd[i][j] > DNGN_EXIT_PORTAL_VAULT)
                    && grd[i][j] != DNGN_EXIT_HELL // just to be safe
                    && mgrd[i][j] == NON_MONSTER
                    && igrd[i][j] == NON_ITEM)
                {
                    if (width == 2 && river_type == DNGN_DEEP_WATER
                        && coinflip())
                    {
                        grd[i][j] = DNGN_SHALLOW_WATER;
                    }
                    else
                        grd[i][j] = river_type;
                }
            }
        }
    }
}                               // end build_river()

static void build_lake(dungeon_feature_type lake_type) //mv
{
    int i, j;
    int x1, y1, x2, y2;

    if (player_in_branch( BRANCH_CRYPT ) || player_in_branch( BRANCH_TOMB ))
        return;

    // if (one_chance_in (10))
    //      build_lake(lake_type); 

    x1 = 5 + random2(GXM - 30);
    y1 = 5 + random2(GYM - 30);
    x2 = x1 + 4 + random2(16);
    y2 = y1 + 8 + random2(12);
    // mpr("lake");

    for (j = y1; j < y2; j++)
    {
        if (coinflip())  x1 += random2(3);
        if (coinflip())  x1 -= random2(3);
        if (coinflip())  x2 += random2(3);
        if (coinflip())  x2 -= random2(3);

    //  mv: this does much more worse effects
    //    if (coinflip()) x1 = x1 -2 + random2(5);
    //    if (coinflip()) x2 = x2 -2 + random2(5);

        if ((j-y1) < ((y2-y1) / 2))
        {
            x2 += random2(3);
            x1 -= random2(3);
        }
        else
        {
            x2 -= random2(3);
            x1 += random2(3);
        }

        for (i = x1; i < x2 ; i++)
        {
            if ((j >= 5 && j <= GYM - 5) && (i >= 5 && i <= GXM - 5))
            {
                // Note that vaults might have been created in this area!
                // So we'll avoid the silliness of monsters and items 
                // on lava and deep water grids. -- bwr
                if (!one_chance_in(200)
                    && (grd[i][j] < DNGN_ENTER_SHOP
                        || grd[i][j] > DNGN_EXIT_PORTAL_VAULT)
                    && grd[i][j] != DNGN_EXIT_HELL // just to be safe
                    && mgrd[i][j] == NON_MONSTER
                    && igrd[i][j] == NON_ITEM)
                {
                    grd[i][j] = lake_type;
                }
            }
        }
    }
}                               // end lake()

struct nearest_point
{
    coord_def target;
    coord_def nearest;
    int       distance;

    nearest_point(const coord_def &t) : target(t), nearest(), distance(-1)
    {
    }
    void operator () (const coord_def &c)
    {
        if (grd(c) == DNGN_FLOOR)
        {
            const int ndist = (c - target).abs();
            if (distance == -1 || ndist < distance)
            {
                distance = ndist;
                nearest  = c;
            }
        }
    }
};

inline static bool dgn_square_travel_ok(const coord_def &c)
{
    const dungeon_feature_type feat = grd(c);
    return (is_traversable(feat) || grid_is_trap(feat)
            || feat == DNGN_SECRET_DOOR);
}

// Fill travel_point_distance out from all stone stairs on the level.
static coord_def dgn_find_closest_to_stone_stairs(coord_def base_pos)
{
    memset(travel_point_distance, 0, sizeof(travel_distance_grid_t));
    init_travel_terrain_check(false);
    nearest_point np(base_pos);
    for (int y = 0; y < GYM; ++y)
        for (int x = 0; x < GXM; ++x)
            if (!travel_point_distance[x][y] && grid_is_stone_stair(grd[x][y]))
                dgn_fill_zone(coord_def(x, y), 1, np, dgn_square_travel_ok);
    return (np.nearest);
}

static coord_def dgn_find_feature_marker(dungeon_feature_type feat)
{
    std::vector<map_marker*> markers = env.markers.get_all();
    for (int i = 0, size = markers.size(); i < size; ++i)
    {
        map_marker *mark = markers[i];
        if (mark->get_type() == MAT_FEATURE
            && dynamic_cast<map_feature_marker*>(mark)->feat == feat)
        {
            return (mark->pos);
        }
    }
    coord_def unfound;
    return (unfound);    
}

static coord_def dgn_find_labyrinth_entry_point()
{
    return (dgn_find_feature_marker(DNGN_ENTER_LABYRINTH));
}

coord_def dgn_find_nearby_stair(dungeon_feature_type stair_to_find,
                                coord_def base_pos, bool find_closest)
{
#ifdef DEBUG_DIAGNOSTICS
    mprf(MSGCH_DIAGNOSTICS,
         "Level entry point on %sstair: %d (%s)",
         find_closest? "closest " : "",
         stair_to_find,
         dungeon_feature_name(stair_to_find));
#endif
    
    if (stair_to_find == DNGN_ROCK_STAIRS_UP
        || stair_to_find == DNGN_ROCK_STAIRS_DOWN)
    {
        const coord_def pos(dgn_find_closest_to_stone_stairs(base_pos));
        if (in_bounds(pos))
            return (pos);
    }

    if (stair_to_find == DNGN_STONE_ARCH)
    {
        const coord_def pos(dgn_find_feature_marker(stair_to_find));
        if (in_bounds(pos) && grd(pos) == stair_to_find)
            return (pos);
    }

    if (stair_to_find == DNGN_ENTER_LABYRINTH)
    {
        const coord_def pos(dgn_find_labyrinth_entry_point());
        if (in_bounds(pos))
            return (pos);

        // Couldn't find a good place, warn, and use old behaviour.
#ifdef DEBUG_DIAGNOSTICS
        mpr("Oops, couldn't find labyrinth entry marker.", MSGCH_DIAGNOSTICS);
#endif
        stair_to_find = DNGN_FLOOR;
    }
    
    if (stair_to_find == your_branch().exit_stairs)
    {
        const coord_def pos(dgn_find_feature_marker(DNGN_STONE_STAIRS_UP_I));
        if (in_bounds(pos) && grd(pos) == stair_to_find)
            return (pos);
    }

    // scan around the player's position first
    int basex = base_pos.x;
    int basey = base_pos.y;

    // check for illegal starting point
    if ( !in_bounds(basex, basey) )
    {
        basex = 0;
        basey = 0;
    }

    coord_def result;

    int found = 0;
    int best_dist = 1 + GXM*GXM + GYM*GYM;

    // XXX These passes should be rewritten to use an iterator of STL
    // algorithm of some kind.

    // First pass: look for an exact match
    for (int xcode = 0; xcode < GXM; ++xcode )
    {
        const int xsign = ((xcode % 2) ? 1 : -1);
        const int xdiff = xsign * (xcode + 1)/2;
        const int xpos  = (basex + xdiff + GXM) % GXM;

        for (int ycode = 0; ycode < GYM; ++ycode)
        {
            const int ysign = ((ycode % 2) ? 1 : -1);
            const int ydiff = ysign * (ycode + 1)/2;
            const int ypos  = (basey + ydiff + GYM) % GYM;

            // note that due to the wrapping above, we can't just use
            // xdiff*xdiff + ydiff*ydiff
            const int dist = (xpos-basex)*(xpos-basex) +
                (ypos-basey)*(ypos-basey);

            if (grd[xpos][ypos] == stair_to_find)
            {
                found++;
                if (find_closest)
                {
                    if (dist < best_dist)
                    {
                        best_dist = dist;
                        result.x = xpos;
                        result.y = ypos;
                    }
                }
                else if (one_chance_in( found ))
                {
                    result.x = xpos;
                    result.y = ypos;
                }
            }
        }
    }

    if ( found )
        return result;

    best_dist = 1 + GXM*GXM + GYM*GYM;

    // Second pass: find a staircase in the proper direction
    for (int xcode = 0; xcode < GXM; ++xcode )
    {
        const int xsign = ((xcode % 2) ? 1 : -1);
        const int xdiff = xsign * (xcode + 1)/2;
        const int xpos  = (basex + xdiff + GXM) % GXM;

        for (int ycode = 0; ycode < GYM; ++ycode)
        {
            const int ysign = ((ycode % 2) ? 1 : -1);
            const int ydiff = ysign * (ycode + 1)/2;
            const int ypos  = (basey + ydiff + GYM) % GYM;

            bool good_stair;
            const int looking_at = grd[xpos][ypos];

            if (stair_to_find <= DNGN_ROCK_STAIRS_DOWN )
                good_stair =
                    (looking_at >= DNGN_STONE_STAIRS_DOWN_I) &&
                    (looking_at <= DNGN_ROCK_STAIRS_DOWN);
            else
                good_stair =
                    (looking_at >= DNGN_STONE_STAIRS_UP_I) &&
                    (looking_at <= DNGN_ROCK_STAIRS_UP);

            const int dist = (xpos-basex)*(xpos-basex) +
                (ypos-basey)*(ypos-basey);

            if ( good_stair )
            {
                found++;
                if (find_closest && dist < best_dist)
                {
                    best_dist = dist;
                    result.x = xpos;
                    result.y = ypos;
                }
                else if (one_chance_in( found ))
                {
                    result.x = xpos;
                    result.y = ypos;
                }
            }
        }
    }

    if ( found )
        return result;

    // Third pass: look for any clear terrain and abandon the idea of
    // looking nearby now. This is used when taking transit Pandemonium gates,
    // or landing in Labyrinths. Never land the PC inside a Pan or Lab vault.
    // We can't check vaults for other levels because vault information is
    // not saved, and the player can re-enter other levels.
    for (int xpos = 0; xpos < GXM; xpos++)
    {
        for (int ypos = 0; ypos < GYM; ypos++)
        {
            if (grd[xpos][ypos] >= DNGN_FLOOR
                && (you.level_type == LEVEL_DUNGEON
                    || unforbidden(coord_def(xpos, ypos), MMT_VAULT)))
            {
                found++;
                if (one_chance_in( found ))
                {
                    result.x = xpos;
                    result.y = ypos;
                }
            }
        }
    }

    ASSERT( found );
    return result;
}

void dgn_set_lt_callback(std::string level_type_name,
                         std::string callback_name)
{
    ASSERT(level_type_name != "");
    ASSERT(callback_name   != "");

    level_type_post_callbacks[level_type_name] = callback_name;
}

////////////////////////////////////////////////////////////////////
// dgn_region

bool dgn_region::overlaps(const dgn_region &other) const
{
    // The old overlap check checked only two corners - top-left and
    // bottom-right. I'm hoping nothing actually *relied* on that stupid bug.
    
    return (between(pos.x, other.pos.x, other.pos.x + other.size.x - 1)
            || between(pos.x + size.x - 1, other.pos.x,
                       other.pos.x + other.size.x - 1))
        && (between(pos.y, other.pos.y, other.pos.y + other.size.y - 1)
            || between(pos.y + size.y - 1, other.pos.y,
                       other.pos.y + other.size.y - 1));
}

bool dgn_region::overlaps_any(const dgn_region_list &regions) const
{
    for (dgn_region_list::const_iterator i = regions.begin();
         i != regions.end(); ++i)
    {
        if (overlaps(*i))
            return (true);
    }
    return (false);
}

bool dgn_region::overlaps(const dgn_region_list &regions,
                          const map_mask &mask) const
{
    return overlaps_any(regions) && overlaps(mask);
}

bool dgn_region::overlaps(const map_mask &mask) const
{
    const coord_def endp = pos + size;
    for (int y = pos.y; y < endp.y; ++y)
        for (int x = pos.x; x < endp.x; ++x)
            if (mask[x][y])
                return (true);
    return (false);
}

coord_def dgn_region::random_edge_point() const
{
    return random2(size.x + size.y) < size.x?
        coord_def( pos.x + random2(size.x),
                   coinflip()? pos.y : pos.y + size.y - 1 )
        :
        coord_def( coinflip()? pos.x : pos.x + size.x - 1,
                   pos.y + random2(size.y) );
}

coord_def dgn_region::random_point() const
{
    return coord_def( pos.x + random2(size.x), pos.y + random2(size.y) );
}
