/*
 *  File:       direct.cc
 *  Summary:    Functions used when picking squares.
 *  Written by: Linley Henzell
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 * <5>  01/08/01       GDL   complete rewrite of direction()
 * <4>  11/23/99       LRH   Looking at monsters now
 *                           displays more info
 * <3>  5/12/99        BWR   changes to allow for space selection of target.
 *                           CR, ESC, and 't' in targeting.
 * <2>  5/09/99        JDJ   look_around no longer prints a prompt.
 * <1>  -/--/--        LRH   Created
 */

#include "AppHdr.h"
#include "directn.h"
#include "format.h"

#include <cstdarg>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>

#ifdef DOS
 #include <conio.h>
#endif

#include "externs.h"

#include "cio.h"
#include "command.h"
#include "debug.h"
#include "describe.h"
#include "dungeon.h"
#include "itemname.h"
#include "mapmark.h"
#include "message.h"
#include "misc.h"
#include "menu.h"
#include "monstuff.h"
#include "mon-util.h"
#include "player.h"
#include "shopping.h"
#include "state.h"
#include "stuff.h"
#include "spells4.h"
#include "stash.h"
#include "tiles.h"
#include "terrain.h"
#include "traps.h"
#include "travel.h"
#include "tutorial.h"
#include "view.h"

#include "macro.h"

enum LOSSelect
{
    LOS_ANY      = 0x00,

    // Check only visible squares
    LOS_VISIBLE  = 0x01,

    // Check only hidden squares
    LOS_HIDDEN   = 0x02,

    LOS_VISMASK  = 0x03,

    // Flip from visible to hidden when going forward,
    // or hidden to visible when going backwards.
    LOS_FLIPVH   = 0x20,

    // Flip from hidden to visible when going forward,
    // or visible to hidden when going backwards.
    LOS_FLIPHV   = 0x40,

    LOS_NONE     = 0xFFFF
};

static void describe_feature(int mx, int my, bool oos);
static void describe_cell(int mx, int my);

static bool find_object(  int x, int y, int mode, bool need_path, int range );
static bool find_monster( int x, int y, int mode, bool need_path, int range );
static bool find_feature( int x, int y, int mode, bool need_path, int range );

static char find_square_wrapper( int tx, int ty,
                                 FixedVector<char, 2> &mfp, char direction,
                                 bool (*targ)(int, int, int, bool, int),
                                 bool need_path = false, int mode = TARG_ANY,
                                 int range = -1, bool wrap = false,
                                 int los = LOS_ANY);

static char find_square( int xps, int yps,
                         FixedVector<char, 2> &mfp, int direction,
                         bool (*targ)(int, int, int, bool, int),
                         bool need_path, int mode = TARG_ANY, int range = -1,
                         bool wrap = false, int los = LOS_ANY);

static int targeting_cmd_to_compass( command_type command );
static void describe_oos_square(int x, int y);
static void extend_move_to_edge(dist &moves);

void direction_choose_compass( dist& moves, targeting_behaviour *beh)
{
    moves.isValid       = true;
    moves.isTarget      = false;
    moves.isMe          = false;
    moves.isCancel      = false;
    moves.dx = moves.dy = 0;

#ifdef USE_TILE
    mouse_control mc(MOUSE_MODE_TARGET_DIR);
#endif

    beh->compass        = true;

    do
    {
        const command_type key_command = beh->get_command();

        if (key_command == CMD_TARGET_SELECT)
        {
            moves.dx = moves.dy = 0;
            moves.isMe = true;
            break;
        }

        const int i = targeting_cmd_to_compass(key_command);
        if ( i != -1 )
        {
            moves.dx = Compass[i].x;
            moves.dy = Compass[i].y;
        }
        else if ( key_command == CMD_TARGET_CANCEL )
        {
            moves.isCancel = true;
            moves.isValid = false;
        }
    }
    while ( !moves.isCancel && moves.dx == 0 && moves.dy == 0 );

    return;
}

static int targeting_cmd_to_compass( command_type command )
{
    switch ( command )
    {
    case CMD_TARGET_UP:         case CMD_TARGET_DIR_UP:
        return 0;
    case CMD_TARGET_UP_RIGHT:   case CMD_TARGET_DIR_UP_RIGHT:
        return 1;
    case CMD_TARGET_RIGHT:      case CMD_TARGET_DIR_RIGHT:
        return 2;
    case CMD_TARGET_DOWN_RIGHT: case CMD_TARGET_DIR_DOWN_RIGHT:
        return 3;
    case CMD_TARGET_DOWN:       case CMD_TARGET_DIR_DOWN:
        return 4;
    case CMD_TARGET_DOWN_LEFT:  case CMD_TARGET_DIR_DOWN_LEFT:
        return 5;
    case CMD_TARGET_LEFT:       case CMD_TARGET_DIR_LEFT:
        return 6;
    case CMD_TARGET_UP_LEFT:    case CMD_TARGET_DIR_UP_LEFT:
        return 7;
    default:
        return -1;
    }
}

static int targeting_cmd_to_feature( command_type command )
{
    switch ( command )
    {
    case CMD_TARGET_FIND_TRAP:
        return '^';
    case CMD_TARGET_FIND_PORTAL:
        return '\\';
    case CMD_TARGET_FIND_ALTAR:
        return  '_';
    case CMD_TARGET_FIND_UPSTAIR:
        return '<';
    case CMD_TARGET_FIND_DOWNSTAIR:
        return '>';
    default:
        return 0;
    }
}

static command_type shift_direction(command_type cmd)
{
    switch (cmd)
    {
    case CMD_TARGET_DOWN_LEFT:  return CMD_TARGET_DIR_DOWN_LEFT;
    case CMD_TARGET_LEFT:       return CMD_TARGET_DIR_LEFT;
    case CMD_TARGET_DOWN:       return CMD_TARGET_DIR_DOWN;
    case CMD_TARGET_UP:         return CMD_TARGET_DIR_UP;
    case CMD_TARGET_RIGHT:      return CMD_TARGET_DIR_RIGHT;
    case CMD_TARGET_DOWN_RIGHT: return CMD_TARGET_DIR_DOWN_RIGHT;
    case CMD_TARGET_UP_RIGHT:   return CMD_TARGET_DIR_UP_RIGHT;
    case CMD_TARGET_UP_LEFT:    return CMD_TARGET_DIR_UP_LEFT;
    default: return (cmd);
    }
}

static const char *target_mode_help_text(int mode)
{
    switch (mode)
    {
    case DIR_NONE:
        return Options.target_unshifted_dirs? "? - help" :
            "? - help, Shift-Dir - shoot in a straight line";
    case DIR_TARGET:
        return "? - help, Dir - move target cursor";
    default:
        return "? - help";
    }
}

static void draw_ray_glyph(const coord_def &pos, int colour,
                           int glych, int mcol)
{
#ifdef USE_TILE
    tile_place_ray(pos);
#else
    int mid = mgrd(pos);
    if (mid != NON_MONSTER)
    {
        const monsters *mons = &menv[mid];
        if (mons->alive() && player_monster_visible(mons))
        {
            glych  = get_screen_glyph(pos.x, pos.y);
            colour = mcol;
        }
    }
    const coord_def vp = grid2view(pos);
    cgotoxy(vp.x, vp.y, GOTO_DNGN);
    textcolor( real_colour(colour) );
    putch(glych);
#endif
}

// Unseen monsters in shallow water show a "strange disturbance".
// (Unless flying!)
static bool _mon_submerged_in_water(const monsters *mon)
{
    if (!mon)
        return false;

    return (grd[mon->x][mon->y] == DNGN_SHALLOW_WATER
            && see_grid(mon->x, mon->y)
            && !player_monster_visible(mon)
            && !mons_flies(mon));
}

static bool _is_target_in_range(int x, int y, int range)
{
    // range doesn't matter
    if (range == -1)
        return true;

    return (grid_distance(you.x_pos, you.y_pos, x, y) <= range);
}

// We handle targeting for repeating commands and re-doing the
// previous command differently (i.e., not just letting the keys
// stuffed into the macro buffer replay as-is) because if the player
// targeted a monster using the movement keys and the monster then
// moved between repititions, then simply replaying the keys in the
// buffer will target an empty square.
static void direction_again(dist& moves, targeting_type restricts,
                            targ_mode_type mode, int range, bool just_looking,
                            const char *prompt, targeting_behaviour *beh)
{
    moves.isValid       = false;
    moves.isTarget      = false;
    moves.isMe          = false;
    moves.isCancel      = false;
    moves.isEndpoint    = false;
    moves.choseRay      = false;

    if (you.prev_targ == MHITNOT && you.prev_grd_targ == coord_def(0, 0))
    {
        moves.isCancel = true;
        crawl_state.cancel_cmd_repeat();
        return;
    }

    int targ_types = 0;
    if (you.prev_targ != MHITNOT && you.prev_targ != MHITYOU)
        targ_types++;
    if (you.prev_targ == MHITYOU)
        targ_types++;
    if (you.prev_grd_targ != coord_def(0, 0))
        targ_types++;
    ASSERT(targ_types == 1);

    // Discard keys until we get to a set-target command
    command_type key_command = CMD_NO_CMD;

    while (crawl_state.is_replaying_keys())
    {
        key_command = beh->get_command();

        if (key_command == CMD_TARGET_PREV_TARGET
            || key_command == CMD_TARGET_SELECT_ENDPOINT
            || key_command == CMD_TARGET_SELECT
            || key_command == CMD_TARGET_MAYBE_PREV_TARGET)
        {
            break;
        }
    }

    if (!crawl_state.is_replaying_keys())
    {
        moves.isCancel = true;

        mpr("Ran out of keys.");

        return;
    }

    if (key_command == CMD_TARGET_SELECT_ENDPOINT)
        moves.isEndpoint = true;

    if (you.prev_grd_targ != coord_def(0, 0))
    {
        if (!see_grid(you.prev_grd_targ))
        {
            moves.isCancel = true;

            crawl_state.cancel_cmd_repeat("You can no longer see the dungeon "
                                          "square you previously targeted.");
            return;
        }
        else if (you.prev_grd_targ.x == you.x_pos
                 && you.prev_grd_targ.y == you.y_pos)
        {
            moves.isCancel = true;

            crawl_state.cancel_cmd_repeat("You are now standing on your "
                                          "previously targeted dungeon "
                                          "square.");
            return;
        }
        else if (!_is_target_in_range(you.prev_grd_targ.x, you.prev_grd_targ.y,
                                      range))
        {
            moves.isCancel = true;

            crawl_state.cancel_cmd_repeat("Your previous target is now out of "
                                          "range.");
            return;
        }

        moves.tx = you.prev_grd_targ.x;
        moves.ty = you.prev_grd_targ.y;

        ray_def ray;
        find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty, true, ray,
                 0, true);
        moves.ray = ray;
    }
    else if (you.prev_targ == MHITYOU)
    {
        moves.isMe = true;
        moves.tx   = you.x_pos;
        moves.ty   = you.y_pos;

        // Discard 'Y' player gave to yesno()
        if (mode == TARG_ENEMY)
            getchm();
    }
    else
    {
        const monsters *montarget = &menv[you.prev_targ];

        if (!mons_near(montarget)
            || !player_monster_visible( montarget ))
        {
            moves.isCancel = true;

            crawl_state.cancel_cmd_repeat("Your target is gone.");

            return;
        }
        else if (!_is_target_in_range(you.prev_grd_targ.x, you.prev_grd_targ.y,
                                      range))
        {
            moves.isCancel = true;

            crawl_state.cancel_cmd_repeat("Your previous target is now out of "
                                          "range.");
            return;
        }

        moves.tx = montarget->x;
        moves.ty = montarget->y;

        ray_def ray;
        find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty, true, ray,
                 0, true);
        moves.ray = ray;
    }

    moves.isValid  = true;
    moves.isTarget = true;

    return;
}

//---------------------------------------------------------------
//
// direction
//
// use restrict == DIR_DIR to allow only a compass direction;
//              == DIR_TARGET to allow only choosing a square;
//              == DIR_NONE to allow either.
//
// outputs: dist structure:
//
//           isValid        a valid target or direction was chosen
//           isCancel       player hit 'escape'
//           isTarget       targetting was used
//           choseRay       player wants a specific ray
//           ray            ...this one
//           isEndpoint     player wants the ray to stop on the dime
//           tx,ty          target x,y
//           dx,dy          direction delta for DIR_DIR
//
//---------------------------------------------------------------
void direction(dist& moves, targeting_type restricts,
               targ_mode_type mode, int range, bool just_looking,
               bool needs_path, const char *prompt,
               targeting_behaviour *beh)
{
    static targeting_behaviour stock_behaviour;
    if (!beh)
        beh = &stock_behaviour;

    beh->just_looking = just_looking;

    if (crawl_state.is_replaying_keys() && restricts != DIR_DIR)
    {
        direction_again(moves, restricts, mode, range, just_looking,
                        prompt, beh);
        return;
    }

    // NOTE: Even if just_looking is set, moves is still interesting,
    // because we can travel there!

    if (restricts == DIR_DIR)
    {
        direction_choose_compass( moves, beh );
        return;
    }

    cursor_control con(!Options.use_fake_cursor);

    int dir = 0;
    bool show_beam = Options.show_beam && !just_looking && needs_path;
    ray_def ray;

    FixedVector < char, 2 > objfind_pos;
    FixedVector < char, 2 > monsfind_pos;

    // init
    moves.dx = moves.dy = 0;
    moves.tx = you.x_pos;
    moves.ty = you.y_pos;

    // If we show the beam on startup, we have to initialise it.
    if (show_beam)
        find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty, true, ray);

    bool skip_iter = false;
    bool found_autotarget = false;
    bool target_unshifted = Options.target_unshifted_dirs;

    // Find a default target
    if (Options.default_target && mode == TARG_ENEMY)
    {
        skip_iter = true;   // skip first iteration...XXX mega-hack
        if (you.prev_targ != MHITNOT && you.prev_targ != MHITYOU)
        {
            const monsters *montarget = &menv[you.prev_targ];
            if (mons_near(montarget) && player_monster_visible(montarget)
                && !mons_friendly(montarget) // not made friendly since then
                && _is_target_in_range(montarget->x, montarget->y, range))
            {
                found_autotarget = true;
                moves.tx = montarget->x;
                moves.ty = montarget->y;
            }
        }
    }

    bool show_prompt = true;
    while (true)
    {
        // Prompts might get scrolled off if you have too few lines available.
        // We'll live with that.
        if ( !just_looking && (show_prompt || beh->should_redraw()) )
        {
            mprf(MSGCH_PROMPT, "%s (%s)", prompt? prompt : "Aim",
                 target_mode_help_text(restricts));

            if ((mode == TARG_ANY || mode == TARG_FRIEND)
                 && moves.tx == you.x_pos && moves.ty == you.y_pos)
            {
                terse_describe_square(moves.target());
            }

            show_prompt = false;
        }

        // Reinit...this needs to be done every loop iteration
        // because moves is more persistent than loop_done.
        moves.isValid       = false;
        moves.isTarget      = false;
        moves.isMe          = false;
        moves.isCancel      = false;
        moves.isEndpoint    = false;
        moves.choseRay      = false;

        cursorxy( grid2viewX(moves.tx), grid2viewY(moves.ty) );

        command_type key_command;

        if ( skip_iter )
        {
            if ( found_autotarget )
                key_command = CMD_NO_CMD;
            else
                key_command = CMD_TARGET_CYCLE_FORWARD; // find closest enemy
        }
        else
        {
#ifdef USE_TILE
            mouse_control mc(MOUSE_MODE_TARGET);
#endif
            key_command = beh->get_command();
        }

#ifdef USE_TILE
        // if a mouse command, update location to mouse position...
        if ( key_command == CMD_TARGET_MOUSE_MOVE
             || key_command == CMD_TARGET_MOUSE_SELECT )
        {
            coord_def gc;
            if (gui_get_mouse_grid_pos(gc))
            {
                moves.tx = gc.x;
                moves.ty = gc.y;

                if (key_command == CMD_TARGET_MOUSE_SELECT)
                    key_command = CMD_TARGET_SELECT;
            }
            else
            {
                key_command = CMD_NO_CMD;
            }
        }
#endif

        if (target_unshifted && moves.tx == you.x_pos && moves.ty == you.y_pos
            && restricts != DIR_TARGET)
        {
            key_command = shift_direction(key_command);
        }

        if (target_unshifted
            && (key_command == CMD_TARGET_CYCLE_FORWARD
                || key_command == CMD_TARGET_CYCLE_BACK
                || key_command == CMD_TARGET_OBJ_CYCLE_FORWARD
                || key_command == CMD_TARGET_OBJ_CYCLE_BACK))
        {
            target_unshifted = false;
        }

        if ( key_command == CMD_TARGET_MAYBE_PREV_TARGET )
        {
            if ( moves.tx == you.x_pos && moves.ty == you.y_pos )
                key_command = CMD_TARGET_PREV_TARGET;
            else
                key_command = CMD_TARGET_SELECT;
        }

        bool need_beam_redraw = false;
        bool force_redraw = false;
        bool loop_done = false;

        const int old_tx = moves.tx + (skip_iter ? 500 : 0); // hmmm...hack
        const int old_ty = moves.ty;

        int i, mid;

        switch ( key_command )
        {
            // standard movement
        case CMD_TARGET_DOWN_LEFT:
        case CMD_TARGET_DOWN:
        case CMD_TARGET_DOWN_RIGHT:
        case CMD_TARGET_LEFT:
        case CMD_TARGET_RIGHT:
        case CMD_TARGET_UP_LEFT:
        case CMD_TARGET_UP:
        case CMD_TARGET_UP_RIGHT:
            i = targeting_cmd_to_compass(key_command);
            moves.tx += Compass[i].x;
            moves.ty += Compass[i].y;
            break;

        case CMD_TARGET_DIR_DOWN_LEFT:
        case CMD_TARGET_DIR_DOWN:
        case CMD_TARGET_DIR_DOWN_RIGHT:
        case CMD_TARGET_DIR_LEFT:
        case CMD_TARGET_DIR_RIGHT:
        case CMD_TARGET_DIR_UP_LEFT:
        case CMD_TARGET_DIR_UP:
        case CMD_TARGET_DIR_UP_RIGHT:
            i = targeting_cmd_to_compass(key_command);

            if (restricts != DIR_TARGET)
            {
                // A direction is allowed, and we've selected it.
                moves.dx = Compass[i].x;
                moves.dy = Compass[i].y;
                // Needed for now...eventually shouldn't be necessary
                moves.tx = you.x_pos + moves.dx;
                moves.ty = you.y_pos + moves.dy;
                moves.isValid = true;
                moves.isTarget = false;
                show_beam = false;
                moves.choseRay = false;
                loop_done = true;
            }
            else
            {
                // Direction not allowed, so just move in that direction.
                // Maybe make this a bigger jump?
                if (restricts == DIR_TARGET)
                {
                    moves.tx += Compass[i].x * 3;
                    moves.ty += Compass[i].y * 3;
                }
                else
                {
                    moves.tx += Compass[i].x;
                    moves.ty += Compass[i].y;
                }
            }
            break;

#ifdef WIZARD
        case CMD_TARGET_CYCLE_BEAM:
            show_beam = find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty,
                                 true, ray, (show_beam ? 1 : 0));
            need_beam_redraw = true;
            break;
#endif

        case CMD_TARGET_HIDE_BEAM:
            if (show_beam)
            {
                show_beam = false;
                need_beam_redraw = true;
            }
            else
            {
                if (!needs_path)
                {
                    mprf(MSGCH_EXAMINE_FILTER,
                         "This spell doesn't need a beam path.");
                    break;
                }

                show_beam = find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty,
                                     true, ray, 0, true);
                need_beam_redraw = show_beam;
            }
            break;

        case CMD_TARGET_FIND_YOU:
            moves.tx = you.x_pos;
            moves.ty = you.y_pos;
            moves.dx = 0;
            moves.dy = 0;
            break;

        case CMD_TARGET_FIND_TRAP:
        case CMD_TARGET_FIND_PORTAL:
        case CMD_TARGET_FIND_ALTAR:
        case CMD_TARGET_FIND_UPSTAIR:
        case CMD_TARGET_FIND_DOWNSTAIR:
        {
            const int thing_to_find = targeting_cmd_to_feature(key_command);
            if (find_square_wrapper(moves.tx, moves.ty, objfind_pos, 1,
                                    find_feature, needs_path, thing_to_find,
                                    range, true, Options.target_los_first ?
                                        LOS_FLIPVH : LOS_ANY))
            {
                moves.tx = objfind_pos[0];
                moves.ty = objfind_pos[1];
            }
            else
            {
                if (!skip_iter)
                    flush_input_buffer(FLUSH_ON_FAILURE);
            }
            break;
        }

        case CMD_TARGET_CYCLE_TARGET_MODE:
            mode = static_cast<targ_mode_type>((mode + 1) % TARG_NUM_MODES);
            mprf( "Targeting mode is now: %s",
                  (mode == TARG_ANY)   ? "any" :
                  (mode == TARG_ENEMY) ? "enemies"
                                       : "friends" );
            break;

        case CMD_TARGET_PREV_TARGET:
            // Do we have a previous target?
            if (you.prev_targ == MHITNOT || you.prev_targ == MHITYOU)
            {
                mpr("You haven't got a previous target.", MSGCH_EXAMINE_FILTER);
                break;
            }

            // we have a valid previous target (maybe)
            {
                const monsters *montarget = &menv[you.prev_targ];

                if (!mons_near(montarget)
                    || !player_monster_visible( montarget ))
                {
                    mpr("You can't see that creature any more.",
                        MSGCH_EXAMINE_FILTER);
                }
                else
                {
                    // We have all the information we need
                    moves.isValid = true;
                    moves.isTarget = true;
                    moves.tx = montarget->x;
                    moves.ty = montarget->y;
                    if ( !just_looking )
                    {
                        // We have to turn off show_beam, because
                        // when jumping to a previous target we don't
                        // care about the beam; otherwise Bad Things
                        // will happen because the ray is invalid,
                        // and we don't get a chance to update it before
                        // breaking from the loop.
                        show_beam = false;
                        loop_done = true;
                    }
                }
                break;
            }

        case CMD_TARGET_SELECT_ENDPOINT:
            moves.isEndpoint = true;
            // intentional fall-through
        case CMD_TARGET_SELECT: // finalize current choice
            if (!moves.isEndpoint
                && mgrd[moves.tx][moves.ty] != NON_MONSTER
                && _mon_submerged_in_water(&menv[mgrd[moves.tx][moves.ty]]))
            {
                moves.isEndpoint = true;
            }
            moves.isValid  = true;
            moves.isTarget = true;
            loop_done = true;

            you.prev_grd_targ = coord_def(0, 0);

            // maybe we should except just_looking here?
            mid = mgrd[moves.tx][moves.ty];

            if ( mid != NON_MONSTER )
                you.prev_targ = mid;
            else if (moves.tx == you.x_pos && moves.ty == you.y_pos)
                you.prev_targ = MHITYOU;
            else
                you.prev_grd_targ = coord_def(moves.tx, moves.ty);
            break;

        case CMD_TARGET_OBJ_CYCLE_BACK:
        case CMD_TARGET_OBJ_CYCLE_FORWARD:
            dir = (key_command == CMD_TARGET_OBJ_CYCLE_BACK) ? -1 : 1;
            if (find_square_wrapper( moves.tx, moves.ty, objfind_pos, dir,
                                     find_object, needs_path, TARG_ANY, range,
                                     true, Options.target_los_first ?
                                         (dir == 1? LOS_FLIPVH : LOS_FLIPHV)
                                          : LOS_ANY))
            {
                moves.tx = objfind_pos[0];
                moves.ty = objfind_pos[1];
            }
            else if (!skip_iter)
                flush_input_buffer(FLUSH_ON_FAILURE);

            break;

        case CMD_TARGET_CYCLE_FORWARD:
        case CMD_TARGET_CYCLE_BACK:
            dir = (key_command == CMD_TARGET_CYCLE_BACK) ? -1 : 1;
            if (find_square_wrapper( moves.tx, moves.ty, monsfind_pos, dir,
                                     find_monster, needs_path, mode, range,
                                     Options.target_wrap))
            {
                moves.tx = monsfind_pos[0];
                moves.ty = monsfind_pos[1];
            }
            else if (!skip_iter)
                flush_input_buffer(FLUSH_ON_FAILURE);

            break;

        case CMD_TARGET_CANCEL:
            loop_done = true;
            moves.isCancel = true;
            break;

#ifdef WIZARD
        case CMD_TARGET_WIZARD_MAKE_FRIENDLY:
            // Maybe we can skip this check...but it can't hurt
            if (!you.wizard || !in_bounds(moves.tx, moves.ty))
                break;
            mid = mgrd[moves.tx][moves.ty];
            if (mid == NON_MONSTER) // can put in terrain description here
                break;

            {
                monsters &m = menv[mid];
                switch (m.attitude)
                {
                 case ATT_FRIENDLY:
                     m.attitude = ATT_GOOD_NEUTRAL;
                     m.flags &= ~MF_CREATED_FRIENDLY;
                     m.flags |= MF_WAS_NEUTRAL;
                     break;
                 case ATT_GOOD_NEUTRAL:
                     m.attitude = ATT_NEUTRAL;
                     break;
                 case ATT_NEUTRAL:
                     m.attitude = ATT_HOSTILE;
                     m.flags &= ~MF_WAS_NEUTRAL;
                     break;
                 case ATT_HOSTILE:
                     m.attitude = ATT_FRIENDLY;
                     m.flags |= MF_CREATED_FRIENDLY;
                     break;
                 default:
                     break;
                }

                // To update visual branding of friendlies.  Only
                // seems capabable of adding bolding, not removing it,
                // though.
                viewwindow(true, false);
            }
            break;

        case CMD_TARGET_WIZARD_BLESS_MONSTER:
            if (!you.wizard || !in_bounds(moves.tx, moves.ty))
                break;
            mid = mgrd[moves.tx][moves.ty];
            if (mid == NON_MONSTER) // can put in terrain description here
                break;

            debug_apply_monster_blessing(&menv[mid]);
            break;

        case CMD_TARGET_WIZARD_MAKE_SHOUT:
            // Maybe we can skip this check...but it can't hurt
            if (!you.wizard || !in_bounds(moves.tx, moves.ty))
                break;
            mid = mgrd[moves.tx][moves.ty];
            if (mid == NON_MONSTER) // can put in terrain description here
                break;

            debug_make_monster_shout(&menv[mid]);
            break;

        case CMD_TARGET_WIZARD_GIVE_ITEM:
            if (!you.wizard || !in_bounds(moves.tx, moves.ty))
                break;
            mid = mgrd[moves.tx][moves.ty];
            if (mid == NON_MONSTER) // can put in terrain description here
                break;

            wizard_give_monster_item(&menv[mid]);
            break;
#endif

        case CMD_TARGET_DESCRIBE:
            full_describe_square(moves.target());
            force_redraw = true;
            break;

        case CMD_TARGET_HELP:
            show_targeting_help();
            force_redraw = true;
            redraw_screen();
            mesclr(true);
            show_prompt = true;
            break;

        default:
            break;
        }

        if ( loop_done == true )
        {
            // This is where we either finalize everything, or else
            // decide that we're not really done and continue looping.

            if ( just_looking ) // easy out
                break;

            // A bunch of confirmation tests; if we survive them all,
            // then break out.

            // Confirm self-targeting on TARG_ENEMY.
            // Conceivably we might want to confirm on TARG_ANY too.
            if ( moves.isTarget
                 && moves.tx == you.x_pos && moves.ty == you.y_pos
                 && mode == TARG_ENEMY
                 && !yesno("Really target yourself?", false, 'n'))
            {
                mesclr();
                show_prompt = true;
            }
            else if ( moves.isTarget && !see_grid(moves.tx, moves.ty) )
            {
                mpr("Sorry, you can't target what you can't see.",
                    MSGCH_EXAMINE_FILTER);
            }
            // Ask for confirmation if we're quitting for some odd reason
            else if ( moves.isValid || moves.isCancel
                      || yesno("Are you sure you want to fizzle?", false, 'n') )
            {
                // Finalize whatever is inside the loop
                // (moves-internal finalizations can be done later)
                moves.choseRay = show_beam;
                moves.ray = ray;
                break;
            }
        }

        // We'll go on looping. Redraw whatever is necessary.

        if ( !in_viewport_bounds(grid2viewX(moves.tx), grid2viewY(moves.ty)) )
        {
            // Tried to step out of bounds
            moves.tx = old_tx;
            moves.ty = old_ty;
        }

        bool have_moved = false;

        if ( old_tx != moves.tx || old_ty != moves.ty )
        {
            have_moved = true;
            show_beam = show_beam &&
                find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty, true, ray,
                         0, true);
        }

        if ( force_redraw )
            have_moved = true;

        if ( have_moved )
        {
            // If the target x,y has changed, the beam must have changed.
            if ( show_beam )
                need_beam_redraw = true;

            if ( !skip_iter )   // don't clear before we get a chance to see
                mesclr(true);   // maybe not completely necessary

            terse_describe_square(moves.target());
        }

#ifdef USE_TILE
        // tiles always need a beam redraw if show_beam is true (and if valid...)
        if ( need_beam_redraw
             || show_beam && find_ray(you.x_pos, you.y_pos, moves.tx, moves.ty,
                                      true, ray, 0, true) )
        {
#else
        if ( need_beam_redraw )
        {
            viewwindow(true, false);
#endif
            if ( show_beam
                 && in_vlos(grid2viewX(moves.tx), grid2viewY(moves.ty))
                 && moves.target() != you.pos() )
            {
                // Draw the new ray with magenta '*'s, not including
                // your square or the target square.
                ray_def raycopy = ray; // temporary copy to work with
                while ( raycopy.pos() != moves.target() )
                {
                    if ( raycopy.pos() != you.pos() )
                    {
                        // Sanity: don't loop forever if the ray is problematic
                        if ( !in_los(raycopy.x(), raycopy.y()) )
                            break;
                        draw_ray_glyph(raycopy.pos(), MAGENTA, '*',
                                       MAGENTA | COLFLAG_REVERSE);
                    }
                    raycopy.advance_through(moves.target());
                }
                textcolor(LIGHTGREY);
#ifdef USE_TILE
                draw_ray_glyph(moves.target(), MAGENTA, '*',
                               MAGENTA | COLFLAG_REVERSE);
            }
            viewwindow(true, false);
#else
            }
#endif

        }
        skip_iter = false;      // only skip one iteration at most
    }
    moves.isMe = (moves.tx == you.x_pos && moves.ty == you.y_pos);
    mesclr();

    // We need this for directional explosions, otherwise they'll explode one
    // square away from the player.
    extend_move_to_edge(moves);
}

void terse_describe_square(const coord_def &c)
{
    if (!see_grid(c.x, c.y))
        describe_oos_square(c.x, c.y);
    else if (in_bounds(c) )
        describe_cell(c.x, c.y);
}

void full_describe_square(const coord_def &c)
{
    // Don't give out information for things outside LOS
    if (!see_grid(c.x, c.y))
        return;

    const int mid = mgrd(c);
    const int oid = igrd(c);

    if (mid != NON_MONSTER && player_monster_visible(&menv[mid]))
    {
        // First priority: monsters
        describe_monsters(menv[mid]);
    }
    else if (oid != NON_ITEM)
    {
        // Second priority: objects
        describe_item( mitm[oid] );
    }
    else
    {
        // Third priority: features
        describe_feature_wide(c.x, c.y);
    }

    redraw_screen();
    mesclr(true);
}

static void extend_move_to_edge(dist &moves)
{
    if (!moves.dx && !moves.dy)
        return;

    // now the tricky bit - extend the target x,y out to map edge.
    int mx = 0, my = 0;

    if (moves.dx > 0)
        mx = (GXM - 1) - you.x_pos;
    if (moves.dx < 0)
        mx = you.x_pos;

    if (moves.dy > 0)
        my = (GYM - 1) - you.y_pos;
    if (moves.dy < 0)
        my = you.y_pos;

    if (!(mx == 0 || my == 0))
    {
        if (mx < my)
            my = mx;
        else
            mx = my;
    }
    moves.tx = you.x_pos + moves.dx * mx;
    moves.ty = you.y_pos + moves.dy * my;
}

// Attempts to describe a square that's not in line-of-sight. If
// there's a stash on the square, announces the top item and number
// of items, otherwise, if there's a stair that's in the travel
// cache and noted in the Dungeon (O)verview, names the stair.
static void describe_oos_square(int x, int y)
{
    mpr("You can't see that place.", MSGCH_EXAMINE_FILTER);

    if (!in_bounds(x, y) || !is_terrain_seen(x, y))
        return;

    describe_stash(x, y);
    describe_feature(x, y, true);
}

bool in_vlos(int x, int y)
{
    return (in_los_bounds(x, y)
            && (env.show(view2show(coord_def(x, y)))
                || coord_def(x, y) == grid2view(you.pos())));
}

bool in_vlos(const coord_def &pos)
{
    return (in_vlos(pos.x, pos.y));
}

bool in_los(int x, int y)
{
    return (in_vlos(grid2view(coord_def(x, y))));
}

static bool find_monster( int x, int y, int mode, bool need_path,
                          int range = -1)
{
    // target the player for friendly and general spells
    if ((mode == TARG_FRIEND || mode == TARG_ANY)
         && x == you.x_pos && y == you.y_pos)
    {
        return (true);
    }

    // don't target out of range
    if (!_is_target_in_range(x, y, range))
        return (false);

    const int targ_mon = mgrd[ x ][ y ];

    // No monster or outside LOS.
    if (targ_mon == NON_MONSTER || !in_los(x,y))
        return (false);

    // Monster in LOS but only via glass walls, so no direct path.
    if (need_path && !see_grid_no_trans(x,y))
        return (false);

    monsters *mon = &menv[targ_mon];

    // Unknown mimics don't count as monsters, either.
    if (mons_is_mimic(mon->type)
        && !(mon->flags & MF_KNOWN_MIMIC))
    {
        return (false);
    }

    // Don't usually target unseen monsters...
    if (!player_monster_visible(mon))
    {
        // ... unless it creates a "disturbance in the water".
        // Since you can't see the monster, assume it's not a friend.
        // Also don't target submerged monsters if there are other targets
        // in sight. (This might be too restrictive.)
        return (mode != TARG_FRIEND
                && _mon_submerged_in_water(mon)
                && i_feel_safe(false, false, true, range));
    }

    // Now compare target modes.
    if (mode == TARG_ANY)
        return true;

    if (mode == TARG_FRIEND)
        return (mons_friendly(&menv[targ_mon] ));

    ASSERT(mode == TARG_ENEMY);
    if (mons_friendly(&menv[targ_mon]))
        return false;

    // Don't target zero xp monsters, unless target_zero_exp is set.
    return (Options.target_zero_exp
            || !mons_class_flag( menv[targ_mon].type, M_NO_EXP_GAIN ));
}

static bool find_feature( int x, int y, int mode,
                          bool /* need_path */, int /* range */)
{
    // The stair need not be in LOS if the square is mapped.
    if (!in_los(x, y) && (!Options.target_oos || !is_terrain_seen(x, y)))
        return (false);

    return is_feature(mode, x, y);
}

static bool find_object(int x, int y, int mode,
                        bool /* need_path */, int /* range */)
{
    // First, check for mimics.
    bool is_mimic = false;
    const int mons = mgrd[ x ][ y ];
    if (mons != NON_MONSTER
        && player_monster_visible( &(menv[mons]) )
        && mons_is_mimic(menv[mons].type)
        && !(menv[mons].flags & MF_KNOWN_MIMIC))
    {
        is_mimic = true;
    }

    const int item = igrd[x][y];
    if (item == NON_ITEM && !is_mimic)
        return false;

    return (in_los(x, y) || Options.target_oos && is_terrain_seen(x, y)
                            && (is_stash(x, y) || is_mimic));
}

static int next_los(int dir, int los, bool wrap)
{
    if (los == LOS_ANY)
        return (wrap? los : LOS_NONE);

    bool vis    = los & LOS_VISIBLE;
    bool hidden = los & LOS_HIDDEN;
    bool flipvh = los & LOS_FLIPVH;
    bool fliphv = los & LOS_FLIPHV;

    if (!vis && !hidden)
        vis = true;

    if (wrap)
    {
        if (!flipvh && !fliphv)
            return (los);

        // We have to invert flipvh and fliphv if we're wrapping. Here's
        // why:
        //
        //  * Say the cursor is on the last item in LOS, there are no
        //    items outside LOS, and wrap == true. flipvh is true.
        //  * We set wrap false and flip from visible to hidden, but there
        //    are no hidden items. So now we need to flip back to visible
        //    so we can go back to the first item in LOS. Unless we set
        //    fliphv, we can't flip from hidden to visible.
        //
        los = flipvh? LOS_FLIPHV : LOS_FLIPVH;
    }
    else
    {
        if (!flipvh && !fliphv)
            return (LOS_NONE);

        if (flipvh && vis != (dir == 1))
            return (LOS_NONE);

        if (fliphv && vis == (dir == 1))
            return (LOS_NONE);
    }

    los = (los & ~LOS_VISMASK) | (vis? LOS_HIDDEN : LOS_VISIBLE);
    return (los);
}

bool in_viewport_bounds(int x, int y)
{
    return crawl_view.in_view_viewport(coord_def(x, y));
}

bool in_los_bounds(int x, int y)
{
    return crawl_view.in_view_los(coord_def(x, y));
}

//---------------------------------------------------------------
//
// find_square
//
// Finds the next monster/object/whatever (moving in a spiral
// outwards from the player, so closer targets are chosen first;
// starts to player's left) and puts its coordinates in mfp.
// Returns 1 if it found something, zero otherwise. If direction
// is -1, goes backwards.
//
// If the game option target_zero_exp is true, zero experience
// monsters will be targeted.
//
//---------------------------------------------------------------
static char find_square( int xps, int yps,
                         FixedVector<char, 2> &mfp, int direction,
                         bool (*find_targ)( int x, int y, int mode,
                                            bool need_path, int range ),
                         bool need_path, int mode, int range, bool wrap,
                         int los )
{
    // the day will come when [unsigned] chars will be consigned to
    // the fires of Gehenna. Not quite yet, though.

    int temp_xps = xps;
    int temp_yps = yps;
    int x_change = 0;
    int y_change = 0;

    bool onlyVis = false, onlyHidden = false;

    int i, j;

    if (los == LOS_NONE)
        return (0);

    if (los == LOS_FLIPVH || los == LOS_FLIPHV)
    {
        if (in_los_bounds(xps, yps))
        {
            // We've been told to flip between visible/hidden, so we
            // need to find what we're currently on.
            const bool vis = (env.show(view2show(coord_def(xps, yps)))
                              || view2grid(coord_def(xps, yps)) == you.pos());

            if (wrap && (vis != (los == LOS_FLIPVH)) == (direction == 1))
            {
                // We've already flipped over into the other direction,
                // so correct the flip direction if we're wrapping.
                los = los == LOS_FLIPHV? LOS_FLIPVH : LOS_FLIPHV;
            }

            los = (los & ~LOS_VISMASK) | (vis? LOS_VISIBLE : LOS_HIDDEN);
        }
        else
        {
            if (wrap)
                los = LOS_HIDDEN | (direction == 1? LOS_FLIPHV : LOS_FLIPVH);
            else
                los |= LOS_HIDDEN;
        }
    }

    onlyVis     = (los & LOS_VISIBLE);
    onlyHidden  = (los & LOS_HIDDEN);

    int radius = 0;
    if (crawl_view.viewsz.x > crawl_view.viewsz.y)
        radius = crawl_view.viewsz.x - LOS_RADIUS - 1;
    else
        radius = crawl_view.viewsz.y - LOS_RADIUS - 1;

    const coord_def vyou = grid2view(you.pos());

    const int minx = vyou.x - radius, maxx = vyou.x + radius,
              miny = vyou.y - radius, maxy = vyou.y + radius,
              ctrx = vyou.x, ctry = vyou.y;

    while (temp_xps >= minx - 1 && temp_xps <= maxx
           && temp_yps <= maxy && temp_yps >= miny - 1)
    {
        if (direction == 1 && temp_xps == minx && temp_yps == maxy)
        {
            if (find_targ(you.x_pos, you.y_pos, mode, need_path, range))
            {
                mfp[0] = ctrx;
                mfp[1] = ctry;
                return (1);
            }
            return find_square(ctrx, ctry, mfp, direction, find_targ,
                               need_path, mode, range, false,
                               next_los(direction, los, wrap));
        }
        if (direction == -1 && temp_xps == ctrx && temp_yps == ctry)
        {
            return find_square(minx, maxy, mfp, direction, find_targ,
                               need_path, mode, range, false,
                               next_los(direction, los, wrap));
        }

        if (direction == 1)
        {
            if (temp_xps == minx - 1)
            {
                x_change = 0;
                y_change = -1;
            }
            else if (temp_xps == ctrx && temp_yps == ctry)
            {
                x_change = -1;
                y_change = 0;
            }
            else if (abs(temp_xps - ctrx) <= abs(temp_yps - ctry))
            {
                if (temp_xps - ctrx >= 0 && temp_yps - ctry <= 0)
                {
                    if (abs(temp_xps - ctrx) > abs(temp_yps - ctry + 1))
                    {
                        x_change = 0;
                        y_change = -1;
                        if (temp_xps - ctrx > 0)
                            y_change = 1;
                        goto finished_spiralling;
                    }
                }
                x_change = -1;
                if (temp_yps - ctry < 0)
                    x_change = 1;
                y_change = 0;
            }
            else
            {
                x_change = 0;
                y_change = -1;
                if (temp_xps - ctrx > 0)
                    y_change = 1;
            }
        }                       // end if (direction == 1)
        else
        {
            // This part checks all eight surrounding squares to find the
            // one that leads on to the present square.
            for (i = -1; i < 2; i++)
            {
                for (j = -1; j < 2; j++)
                {
                    if (i == 0 && j == 0)
                        continue;

                    if (temp_xps + i == minx - 1)
                    {
                        x_change = 0;
                        y_change = -1;
                    }
                    else if (temp_xps + i - ctrx == 0 && temp_yps + j - ctry == 0)
                    {
                        x_change = -1;
                        y_change = 0;
                    }
                    else if (abs(temp_xps + i - ctrx) <= abs(temp_yps + j - ctry))
                    {
                        const int xi = temp_xps + i - ctrx;
                        const int yj = temp_yps + j - ctry;

                        if (xi >= 0 && yj <= 0
                            && abs(xi) > abs(yj + 1))
                        {
                            x_change = 0;
                            y_change = -1;
                            if (xi > 0)
                                y_change = 1;
                            goto finished_spiralling;
                        }

                        x_change = -1;
                        if (yj < 0)
                            x_change = 1;
                        y_change = 0;
                    }
                    else
                    {
                        x_change = 0;
                        y_change = -1;
                        if (temp_xps + i - ctrx > 0)
                            y_change = 1;
                    }

                    if (temp_xps + i + x_change == temp_xps
                        && temp_yps + j + y_change == temp_yps)
                    {
                        goto finished_spiralling;
                    }
                }
            }
        }                       // end else


      finished_spiralling:
        x_change *= direction;
        y_change *= direction;

        temp_xps += x_change;
        if (temp_yps + y_change <= maxy)  // it can wrap, unfortunately
            temp_yps += y_change;

        const int targ_x = you.x_pos + temp_xps - ctrx;
        const int targ_y = you.y_pos + temp_yps - ctry;

        // We don't want to be looking outside the bounds of the arrays:
        //if (!in_los_bounds(temp_xps, temp_yps))
        //    continue;

        if (!crawl_view.in_grid_viewport(coord_def(targ_x, targ_y)))
            continue;

        if (!in_bounds(targ_x, targ_y))
            continue;

        if ((onlyVis || onlyHidden) && onlyVis != in_los(targ_x, targ_y))
            continue;

        if (find_targ(targ_x, targ_y, mode, need_path, range))
        {
            mfp[0] = temp_xps;
            mfp[1] = temp_yps;
            return (1);
        }
    }

    return (direction == 1?
        find_square(ctrx, ctry, mfp, direction, find_targ, need_path, mode,
                    range, false, next_los(direction, los, wrap))
      : find_square(minx, maxy, mfp, direction, find_targ, need_path, mode,
                    range, false, next_los(direction, los, wrap)));
}

// XXX Unbelievably hacky. And to think that my goal was to clean up the code.
// Identical to find_square, except that input (tx, ty) and output
// (mfp) are in grid coordinates rather than view coordinates.
static char find_square_wrapper( int tx, int ty,
                                 FixedVector<char, 2> &mfp, char direction,
                                 bool (*find_targ)( int x, int y, int mode,
                                                    bool need_path, int range ),
                                 bool need_path, int mode, int range, bool wrap,
                                 int los )
{
    const char r =  find_square(grid2viewX(tx), grid2viewY(ty), mfp,
                                direction, find_targ, need_path, mode, range,
                                wrap, los);
    mfp[0] = view2gridX(mfp[0]);
    mfp[1] = view2gridY(mfp[1]);
    return r;
}

static void describe_feature(int mx, int my, bool oos)
{
    if (oos && !is_terrain_seen(mx, my))
        return;

    dungeon_feature_type grid = grd[mx][my];
    if ( grid == DNGN_SECRET_DOOR )
        grid = grid_secret_door_appearance(mx, my);

    std::string desc = feature_description(grid);
    if (desc.length())
    {
        if (oos)
            desc = "[" + desc + "]";

        msg_channel_type channel = MSGCH_EXAMINE;
        if (oos || grid == DNGN_FLOOR)
            channel = MSGCH_EXAMINE_FILTER;

        mpr(desc.c_str(), channel);
    }
}

// Returns a vector of features matching the given pattern.
std::vector<dungeon_feature_type> features_by_desc(const base_pattern &pattern)
{
    std::vector<dungeon_feature_type> features;

    if (pattern.valid())
    {
        for (int i = 0; i < NUM_FEATURES; ++i)
        {
            std::string fdesc =
                feature_description(static_cast<dungeon_feature_type>(i));
            if (fdesc.empty())
                continue;

            if (pattern.matches( fdesc ))
                features.push_back( dungeon_feature_type(i) );
        }
    }
    return (features);
}

void describe_floor()
{
    const int grid = grd(you.pos());

    std::string prefix = "There is ";
    std::string feat;
    std::string suffix = " here.";
    switch (grid)
    {
    case DNGN_FLOOR:
    case DNGN_FLOOR_SPECIAL:
        return;

    case DNGN_ENTER_SHOP:
        prefix = "There is an entrance to ";
        break;

    default:
        break;
    }

    feat = feature_description(you.x_pos, you.y_pos,
                               is_bloodcovered(you.x_pos, you.y_pos),
                               DESC_NOCAP_A, false);
    if (feat.empty())
        return;

    msg_channel_type channel = MSGCH_EXAMINE;

    // water is not terribly important if you don't mind it
    if ((grd[you.x_pos][you.y_pos] == DNGN_DEEP_WATER
            || grd[you.x_pos][you.y_pos] == DNGN_SHALLOW_WATER)
        && player_likes_water())
    {
        channel = MSGCH_EXAMINE_FILTER;
    }
    mpr((prefix + feat + suffix).c_str(), channel);
    if (grid == DNGN_ENTER_LABYRINTH && you.is_undead != US_UNDEAD)
        mpr("Beware, for starvation awaits!", MSGCH_EXAMINE);
}

static std::string feature_do_grammar(description_level_type dtype,
                                      bool add_stop,
                                      bool force_article,
                                      std::string desc)
{
    if (add_stop)
        desc += ".";
    if (dtype == DESC_PLAIN || (!force_article && isupper(desc[0])))
    {
        if (isupper(desc[0]))
        {
            switch (dtype)
            {
            case DESC_PLAIN: case DESC_NOCAP_THE: case DESC_NOCAP_A:
                desc[0] = tolower(desc[0]);
                break;
            default:
                break;
            }
        }
        return (desc);
    }

    switch (dtype)
    {
    case DESC_CAP_THE:
        return "The " + desc;
    case DESC_NOCAP_THE:
        return "the " + desc;
    case DESC_CAP_A:
        return article_a(desc, false);
    case DESC_NOCAP_A:
        return article_a(desc, true);
    case DESC_NONE:
        return ("");
    default:
        return (desc);
    }
}

std::string feature_description(dungeon_feature_type grid,
                                trap_type trap, bool bloody,
                                description_level_type dtype,
                                bool add_stop)
{
    std::string desc = raw_feature_description(grid, trap);
    if (bloody)
        desc += ", spattered with blood";

    return feature_do_grammar(dtype, add_stop, grid_is_trap(grid), desc);
}

std::string raw_feature_description(dungeon_feature_type grid,
                                    trap_type trap)
{
    if (grid_is_trap(grid) && trap != NUM_TRAPS)
    {
        switch (trap)
        {
        case TRAP_DART:
            return ("dart trap");
        case TRAP_ARROW:
            return ("arrow trap");
        case TRAP_SPEAR:
            return ("spear trap");
        case TRAP_AXE:
            return ("axe trap");
        case TRAP_TELEPORT:
            return ("teleportation trap");
        case TRAP_ALARM:
            return ("alarm trap");
        case TRAP_BLADE:
            return ("blade trap");
        case TRAP_BOLT:
            return ("bolt trap");
        case TRAP_NET:
            return ("net trap");
        case TRAP_ZOT:
            return ("Zot trap");
        case TRAP_NEEDLE:
            return ("needle trap");
        case TRAP_SHAFT:
            return ("shaft");
        default:
            error_message_to_player();
            return ("undefined trap");
        }
    }

    switch (grid)
    {
    case DNGN_STONE_WALL:
        return ("stone wall");
    case DNGN_ROCK_WALL:
    case DNGN_SECRET_DOOR:
        if (you.level_type == LEVEL_PANDEMONIUM)
            return ("wall of the weird stuff which makes up Pandemonium");
        else
            return ("rock wall");
    case DNGN_PERMAROCK_WALL:
        return ("unnaturally hard rock wall");
    case DNGN_CLOSED_DOOR:
        return ("closed door");
    case DNGN_METAL_WALL:
        return ("metal wall");
    case DNGN_GREEN_CRYSTAL_WALL:
        return ("wall of green crystal");
    case DNGN_CLEAR_ROCK_WALL:
        return ("translucent rock wall");
    case DNGN_CLEAR_STONE_WALL:
        return ("translucent stone wall");
    case DNGN_CLEAR_PERMAROCK_WALL:
        return ("translucent unnaturally hard rock wall");
    case DNGN_ORCISH_IDOL:
        if (you.species == SP_HILL_ORC)
           return ("idol of Beogh");
        else
           return ("orcish idol");
    case DNGN_WAX_WALL:
        return ("wall of solid wax");
    case DNGN_GRANITE_STATUE:
        return ("granite statue");
    case DNGN_LAVA:
        return ("Some lava");
    case DNGN_DEEP_WATER:
        return ("Some deep water");
    case DNGN_SHALLOW_WATER:
        return ("Some shallow water");
    case DNGN_UNDISCOVERED_TRAP:
    case DNGN_FLOOR:
    case DNGN_FLOOR_SPECIAL:
        return ("Floor");
    case DNGN_OPEN_DOOR:
        return ("open door");
    case DNGN_ESCAPE_HATCH_DOWN:
        return ("escape hatch in the floor");
    case DNGN_ESCAPE_HATCH_UP:
        return ("escape hatch in the ceiling");
    case DNGN_STONE_STAIRS_DOWN_I:
    case DNGN_STONE_STAIRS_DOWN_II:
    case DNGN_STONE_STAIRS_DOWN_III:
        return ("stone staircase leading down");
    case DNGN_STONE_STAIRS_UP_I:
    case DNGN_STONE_STAIRS_UP_II:
    case DNGN_STONE_STAIRS_UP_III:
        return ("stone staircase leading up");
    case DNGN_ENTER_HELL:
        return ("gateway to Hell");
    case DNGN_TRAP_MECHANICAL:
        return ("mechanical trap");
    case DNGN_TRAP_MAGICAL:
        return ("magical trap");
    case DNGN_TRAP_NATURAL:
        return ("natural trap");
    case DNGN_ENTER_SHOP:
        return ("shop");
    case DNGN_ENTER_LABYRINTH:
        return ("labyrinth entrance");
    case DNGN_ENTER_DIS:
        return ("gateway to the Iron City of Dis");
    case DNGN_ENTER_GEHENNA:
        return ("gateway to Gehenna");
    case DNGN_ENTER_COCYTUS:
        return ("gateway to the freezing wastes of Cocytus");
    case DNGN_ENTER_TARTARUS:
        return ("gateway to the decaying netherworld of Tartarus");
    case DNGN_ENTER_ABYSS:
        return ("one-way gate to the infinite horrors of the Abyss");
    case DNGN_EXIT_ABYSS:
        return ("gateway leading out of the Abyss");
    case DNGN_STONE_ARCH:
        return ("empty arch of ancient stone");
    case DNGN_ENTER_PANDEMONIUM:
        return ("gate leading to the halls of Pandemonium");
    case DNGN_EXIT_PANDEMONIUM:
        return ("gate leading out of Pandemonium");
    case DNGN_TRANSIT_PANDEMONIUM:
        return ("gate leading to another region of Pandemonium");
    case DNGN_ENTER_ORCISH_MINES:
        return ("staircase to the Orcish Mines");
    case DNGN_ENTER_HIVE:
        return ("staircase to the Hive");
    case DNGN_ENTER_LAIR:
        return ("staircase to the Lair");
    case DNGN_ENTER_SLIME_PITS:
        return ("staircase to the Slime Pits");
    case DNGN_ENTER_VAULTS:
        return ("staircase to the Vaults");
    case DNGN_ENTER_CRYPT:
        return ("staircase to the Crypt");
    case DNGN_ENTER_HALL_OF_BLADES:
        return ("staircase to the Hall of Blades");
    case DNGN_ENTER_ZOT:
        return ("gate to the Realm of Zot");
    case DNGN_ENTER_TEMPLE:
        return ("staircase to the Ecumenical Temple");
    case DNGN_ENTER_SNAKE_PIT:
        return ("staircase to the Snake Pit");
    case DNGN_ENTER_ELVEN_HALLS:
        return ("staircase to the Elven Halls");
    case DNGN_ENTER_TOMB:
        return ("staircase to the Tomb");
    case DNGN_ENTER_SWAMP:
        return ("staircase to the Swamp");
    case DNGN_ENTER_SHOALS:
        return ("staircase to the Shoals");
    case DNGN_ENTER_PORTAL_VAULT:
        return ("gate leading to a distant place");
    case DNGN_EXIT_PORTAL_VAULT:
        return ("gate leading back to the Dungeon");
    case DNGN_RETURN_FROM_ORCISH_MINES:
    case DNGN_RETURN_FROM_HIVE:
    case DNGN_RETURN_FROM_LAIR:
    case DNGN_RETURN_FROM_VAULTS:
    case DNGN_RETURN_FROM_TEMPLE:
        return ("staircase back to the Dungeon");
    case DNGN_RETURN_FROM_SLIME_PITS:
    case DNGN_RETURN_FROM_SNAKE_PIT:
    case DNGN_RETURN_FROM_SWAMP:
    case DNGN_RETURN_FROM_SHOALS:
        return ("staircase back to the Lair");
    case DNGN_RETURN_FROM_CRYPT:
    case DNGN_RETURN_FROM_HALL_OF_BLADES:
        return ("staircase back to the Vaults");
    case DNGN_RETURN_FROM_ELVEN_HALLS:
        return ("staircase back to the Mines");
    case DNGN_RETURN_FROM_TOMB:
        return ("staircase back to the Crypt");
    case DNGN_RETURN_FROM_ZOT:
        return ("gate leading back out of this place");
    case DNGN_ALTAR_ZIN:
        return ("glowing white marble altar of Zin");
    case DNGN_ALTAR_SHINING_ONE:
        return ("glowing golden altar of the Shining One");
    case DNGN_ALTAR_KIKUBAAQUDGHA:
        return ("ancient bone altar of Kikubaaqudgha");
    case DNGN_ALTAR_YREDELEMNUL:
        return ("basalt altar of Yredelemnul");
    case DNGN_ALTAR_XOM:
        return ("shimmering altar of Xom");
    case DNGN_ALTAR_VEHUMET:
        return ("shining altar of Vehumet");
    case DNGN_ALTAR_OKAWARU:
        return ("iron altar of Okawaru");
    case DNGN_ALTAR_MAKHLEB:
        return ("burning altar of Makhleb");
    case DNGN_ALTAR_SIF_MUNA:
        return ("deep blue altar of Sif Muna");
    case DNGN_ALTAR_TROG:
        return ("bloodstained altar of Trog");
    case DNGN_ALTAR_NEMELEX_XOBEH:
        return ("sparkling altar of Nemelex Xobeh");
    case DNGN_ALTAR_ELYVILON:
        return ("silver altar of Elyvilon");
    case DNGN_ALTAR_LUGONU:
        return ("corrupted altar of Lugonu");
    case DNGN_ALTAR_BEOGH:
        return ("roughly hewn altar of Beogh");
    case DNGN_FOUNTAIN_BLUE:
        return ("fountain of clear blue water");
    case DNGN_FOUNTAIN_SPARKLING:
        return ("fountain of sparkling water");
    case DNGN_FOUNTAIN_BLOOD:
        return ("fountain of blood");
    case DNGN_DRY_FOUNTAIN_BLUE:
    case DNGN_DRY_FOUNTAIN_SPARKLING:
    case DNGN_DRY_FOUNTAIN_BLOOD:
    case DNGN_PERMADRY_FOUNTAIN:
        return ("dry fountain");
    default:
        return ("");
    }
}

static std::string marker_feature_description(const coord_def &p)
{
    std::vector<map_marker*> markers = env.markers.get_markers_at(p);
    for (int i = 0, size = markers.size(); i < size; ++i)
    {
        const std::string desc = markers[i]->feature_description();
        if (!desc.empty())
            return (desc);
    }
    return ("");
}

#ifndef DEBUG_DIAGNOSTICS
// Is a feature interesting enough to 'v'iew it, even if a player normally
// doesn't care about descriptions, i.e. does the description hold important
// information? (Yes, this is entirely subjective. JPEG)

static bool interesting_feature(dungeon_feature_type feat)
{
    return (get_feature_def(feat).flags & FFT_EXAMINE_HINT);
}
#endif

std::string feature_description(int mx, int my, bool bloody,
                                description_level_type dtype, bool add_stop)
{
    dungeon_feature_type grid = grd[mx][my];
    if ( grid == DNGN_SECRET_DOOR )
        grid = grid_secret_door_appearance(mx, my);

    if ( grid == DNGN_OPEN_DOOR || grid == DNGN_CLOSED_DOOR )
    {
        std::set<coord_def> all_door;
        find_connected_identical(coord_def(mx, my), grd[mx][my], all_door);
        const char *adj, *noun;
        get_door_description(all_door.size(), &adj, &noun);

        std::string desc = adj;
        desc += (grid == DNGN_OPEN_DOOR) ? "open " : "closed ";
        desc += noun;

        if (bloody)
            desc += ", spattered with blood";

        return feature_do_grammar(dtype, add_stop, false, desc);
    }

    switch (grid)
    {
    case DNGN_TRAP_MECHANICAL:
    case DNGN_TRAP_MAGICAL:
    case DNGN_TRAP_NATURAL:
        return (feature_description(grid, trap_type_at_xy(mx, my), bloody,
                                    dtype, add_stop));
    case DNGN_ENTER_SHOP:
        return (shop_name(mx, my, add_stop));

    case DNGN_ENTER_PORTAL_VAULT:
        return (feature_do_grammar(
                    dtype, add_stop, false,
                    marker_feature_description(coord_def(mx, my))));
    default:
        return (feature_description(grid, NUM_TRAPS, bloody, dtype, add_stop));
    }
}

static std::string describe_mons_enchantment(const monsters &mons,
                                             const mon_enchant &ench,
                                             bool paralysed)
{
    // Suppress silly-looking combinations, even if they're
    // internally valid.
    if (paralysed && (ench.ench == ENCH_SLOW || ench.ench == ENCH_HASTE))
        return "";

    if ((ench.ench == ENCH_HASTE || ench.ench == ENCH_BATTLE_FRENZY)
        && mons.has_ench(ENCH_BERSERK))
    {
        return "";
    }

    switch (ench.ench)
    {
    case ENCH_POISON:
        return "poisoned";
    case ENCH_SICK:
        return "sick";
    case ENCH_ROT:
        return "rotting away"; //jmf: "covered in sores"?
    case ENCH_BACKLIGHT:
        return "softly glowing";
    case ENCH_SLOW:
        return "moving slowly";
    case ENCH_BERSERK:
        return "berserk";
    case ENCH_BATTLE_FRENZY:
        return "consumed by blood-lust";
    case ENCH_HASTE:
        return "moving very quickly";
    case ENCH_CONFUSION:
        return "bewildered and confused";
    case ENCH_INVIS:
        return "slightly transparent";
    case ENCH_CHARM:
        return "in your thrall";
    case ENCH_STICKY_FLAME:
        return "covered in liquid flames";
    case ENCH_HELD:
        return "entangled in a net";
    default:
        return "";
    } // end switch
}

static std::string describe_monster_weapon(const monsters *mons)
{
    std::string desc = "";
    std::string name1, name2;
    const item_def *weap = mons->mslot_item(MSLOT_WEAPON);
    const item_def *alt  = mons->mslot_item(MSLOT_ALT_WEAPON);

    if (weap)
    {
        name1 = weap->name(DESC_NOCAP_A, false, false, true,
                           false, ISFLAG_KNOW_CURSE);
    }
    if (alt && (!weap || mons_wields_two_weapons(mons)))
    {
        name2 = alt->name(DESC_NOCAP_A, false, false, true,
                          false, ISFLAG_KNOW_CURSE);
    }

    if (name1.empty() && !name2.empty())
        name1.swap(name2);

    if (name1 == name2 && weap)
    {
        item_def dup = *weap;
        ++dup.quantity;
        name1 = dup.name(DESC_NOCAP_A, false, false, true, true,
                         ISFLAG_KNOW_CURSE);
        name2.clear();
    }

    if (name1.empty())
        return (desc);

    desc += " wielding ";
    desc += name1;

    if (!name2.empty())
    {
        desc += " and ";
        desc += name2;
    }

    return (desc);
}

#ifdef DEBUG_DIAGNOSTICS
static std::string stair_destination_description(const coord_def &pos)
{
    if (LevelInfo *linf = travel_cache.find_level_info(level_id::current()))
    {
        const stair_info *si = linf->get_stair(pos);
        if (si)
            return (" " + si->describe());
        else if (is_stair(grd(pos)))
            return (" (unknown stair)");
    }
    return ("");
}
#endif

static void describe_monster(const monsters *mon)
{
    // first print type and equipment
    std::string text = get_monster_desc(mon);
    text += ".";
    print_formatted_paragraph(text, get_number_of_cols());

    if (player_beheld_by(mon))
        mpr("You are beheld by her song.", MSGCH_EXAMINE);

    print_wounds(mon);

    if (!mons_is_mimic(mon->type) && mons_behaviour_perceptible(mon))
    {
        if (mon->behaviour == BEH_SLEEP)
        {
            mprf(MSGCH_EXAMINE, "%s appears to be resting.",
                 mon->pronoun(PRONOUN_CAP).c_str());
        }
        // Applies to both friendlies and hostiles
        else if (mon->behaviour == BEH_FLEE)
        {
            mprf(MSGCH_EXAMINE, "%s is retreating.",
                 mon->pronoun(PRONOUN_CAP).c_str());
        }
        // hostile with target != you
        else if (!mons_friendly(mon) && !mons_neutral(mon) && mon->foe != MHITYOU)
        {
            // special case: batty monsters get set to BEH_WANDER as
            // part of their special behaviour.
            if (!testbits(mon->flags, MF_BATTY))
            {
                mprf(MSGCH_EXAMINE, "%s doesn't appear to have noticed you.",
                     mon->pronoun(PRONOUN_CAP).c_str());
            }
        }
    }

    if (mon->attitude == ATT_FRIENDLY)
    {
        mprf(MSGCH_EXAMINE, "%s is friendly.",
             mon->pronoun(PRONOUN_CAP).c_str());
    }
    else if (mons_neutral(mon)) // don't differentiate between permanent or not
    {
        mprf(MSGCH_EXAMINE, "%s is indifferent to you.",
             mon->pronoun(PRONOUN_CAP).c_str());
    }

    if (mon->haloed())
    {
        mprf(MSGCH_EXAMINE, "%s is illuminated by a divine halo.",
             mon->pronoun(PRONOUN_CAP).c_str());
    }

    std::string desc = "";
    std::string last_desc = "";
    std::string tmp = "";

    const bool paralysed = mons_is_paralysed(mon);
    if (paralysed)
        last_desc += "paralysed";

    for (mon_enchant_list::const_iterator e = mon->enchantments.begin();
         e != mon->enchantments.end(); ++e)
    {
         tmp = describe_mons_enchantment(*mon, e->second, paralysed);
         if (!tmp.empty())
         {
             if (!desc.empty())
                 desc += ", ";
             desc += last_desc;
             last_desc = tmp;
         }
    }

    if (!last_desc.empty())
    {
        if (!desc.empty())
            desc += ", and ";
        desc += last_desc;
    }

    if (!desc.empty())
    {
        text = mon->pronoun(PRONOUN_CAP);
        text += " is ";
        text += desc;
        text += ".";
        print_formatted_paragraph(text, get_number_of_cols());
    }
}

// This method is called in two cases:
// a) Monsters coming into view: "An ogre comes into view. It is wielding ..."
// b) Monster description via 'x': "An ogre, wielding a club and wearing ..."
std::string get_monster_desc(const monsters *mon, bool full_desc,
                             description_level_type mondtype)
{
    std::string desc = "";
    if (mondtype != DESC_NONE)
    {
        desc = mon->name(mondtype);
        // For named monsters also mention the base type in the form of
        // "Morbeogh the orc priest".
        if (!(mon->mname).empty())
            desc += " " + mons_type_name(mon->type, DESC_NOCAP_THE);
    }
    std::string weap = "";

    if (mon->type != MONS_DANCING_WEAPON)
        weap = describe_monster_weapon(mon);

    if (!weap.empty())
    {
        if (full_desc)
            desc += ",";
        desc += weap;
    }

    // Print the rest of the equipment only for full descriptions.
    if (full_desc)
    {
        const int mon_arm = mon->inv[MSLOT_ARMOUR];
        const int mon_shd = mon->inv[MSLOT_SHIELD];
        const int mon_qvr = mon->inv[MSLOT_MISSILE];
        const int mon_alt = mon->inv[MSLOT_ALT_WEAPON];

        const bool need_quiver  = (mon_qvr != NON_ITEM && mons_friendly(mon));
        const bool need_alt_wpn = (mon_alt != NON_ITEM && mons_friendly(mon)
                                   && !mons_wields_two_weapons(mon));
              bool found_sth    = !weap.empty();


        if (mon_arm != NON_ITEM)
        {
            desc += ", ";
            if (found_sth && mon_shd == NON_ITEM && !need_quiver
                          && !need_alt_wpn)
            {
                desc += "and ";
            }
            desc += "wearing ";
            desc += mitm[mon_arm].name(DESC_NOCAP_A);
            if (!found_sth)
                found_sth = true;
        }

        if (mon_shd != NON_ITEM)
        {
            desc += ", ";
            if (found_sth && !need_quiver && !need_alt_wpn)
                desc += "and ";
            desc += "wearing ";
            desc += mitm[mon_shd].name(DESC_NOCAP_A);
            if (!found_sth)
                found_sth = true;
        }

        // For friendly monsters, also list quivered missiles
        // and alternate weapon.
        if (mons_friendly(mon))
        {
            if (mon_qvr != NON_ITEM)
            {
                desc += ", ";
                if (found_sth && !need_alt_wpn)
                    desc += "and ";
                desc += "quivering ";
                desc += mitm[mon_qvr].name(DESC_NOCAP_A);
                if (!found_sth)
                    found_sth = true;
            }

            if (need_alt_wpn)
            {
                desc += ", ";
                if (found_sth)
                    desc += "and ";
                desc += "carrying ";
                desc += mitm[mon_alt].name(DESC_NOCAP_A);
            }
        }
    }

    return desc;
}

static void describe_cell(int mx, int my)
{
    bool mimic_item = false;
    bool monster_described = false;
    bool cloud_described = false;
    bool item_described = false;

    if (mx == you.x_pos && my == you.y_pos)
        mpr("You.", MSGCH_EXAMINE_FILTER);

    if (mgrd[mx][my] != NON_MONSTER)
    {
        int i = mgrd[mx][my];

        if (_mon_submerged_in_water(&menv[i]))
        {
            mpr("There is a strange disturbance in the water here.",
                MSGCH_EXAMINE_FILTER);
        }

#if DEBUG_DIAGNOSTICS
        if (!player_monster_visible( &menv[i] ))
            mpr( "There is a non-visible monster here.", MSGCH_DIAGNOSTICS );
#else
        if (!player_monster_visible( &menv[i] ))
            goto look_clouds;
#endif

        describe_monster(&menv[i]);

        if (mons_is_mimic( menv[i].type ))
        {
            mimic_item = true;
            item_described = true;
        }
        else
            monster_described = true;

#if DEBUG_DIAGNOSTICS
        stethoscope(i);
#endif
        if (Options.tutorial_left && tutorial_monster_interesting(&menv[i]))
        {
            std::string msg = "(Press <w>v<lightgray> for more information.)";
            print_formatted_paragraph(msg, get_number_of_cols());
        }
    }

#if (!DEBUG_DIAGNOSTICS)
  // removing warning
  look_clouds:
#endif

    if (is_sanctuary(mx, my))
        mpr("This square lies inside a sanctuary.");

    if (env.cgrid[mx][my] != EMPTY_CLOUD)
    {
        const int cloud_inspected = env.cgrid[mx][my];
        const cloud_type ctype = (cloud_type) env.cloud[cloud_inspected].type;

        mprf(MSGCH_EXAMINE, "There is a cloud of %s here.",
             cloud_name(ctype).c_str());

        cloud_described = true;
    }

    int targ_item = igrd[ mx ][ my ];

    if (targ_item != NON_ITEM)
    {
        // If a mimic is on this square, we pretend it's the first item -- bwr
        if (mimic_item)
            mpr("There is something else lying underneath.", MSGCH_FLOOR_ITEMS);
        else
        {
            if (mitm[ targ_item ].base_type == OBJ_GOLD)
                mprf( MSGCH_FLOOR_ITEMS, "A pile of gold coins." );
            else
            {
                mprf( MSGCH_FLOOR_ITEMS, "You see %s here.",
                      mitm[targ_item].name(DESC_NOCAP_A).c_str());
            }

            if (mitm[ targ_item ].link != NON_ITEM)
            {
                mprf( MSGCH_FLOOR_ITEMS,
                      "There is something else lying underneath.");
            }
        }
        item_described = true;
    }

    bool bloody = false;
    if (is_bloodcovered(mx, my))
        bloody = true;

    std::string feature_desc = feature_description(mx, my, bloody);
#ifdef DEBUG_DIAGNOSTICS
    std::string marker;
    if (map_marker *mark = env.markers.find(coord_def(mx, my), MAT_ANY))
    {
        std::string desc = mark->debug_describe();
        if (desc.empty())
            desc = "?";
        marker = " (" + desc + ")";
    }
    const std::string traveldest =
        stair_destination_description(coord_def(mx, my));
    const dungeon_feature_type feat = grd[mx][my];
    mprf(MSGCH_DIAGNOSTICS, "(%d,%d): %s - %s (%d/%s)%s%s", mx, my,
         stringize_glyph(get_screen_glyph(mx, my)).c_str(),
         feature_desc.c_str(),
         feat,
         dungeon_feature_name(feat),
         marker.c_str(),
         traveldest.c_str());
#else
    if (Options.tutorial_left && tutorial_feat_interesting(grd[mx][my]))
    {
        feature_desc += " (Press <w>v<lightgray> for more information.)";

        print_formatted_paragraph(feature_desc, get_number_of_cols());
    }
    else
    {
        const dungeon_feature_type feat = grd[mx][my];

        if (interesting_feature(feat))
            feature_desc += " (Press 'v' for more information.)";

        // Suppress "Floor." if there's something on that square that we've
        // already described.
        if ((feat == DNGN_FLOOR || feat == DNGN_FLOOR_SPECIAL) && !bloody
            && (monster_described || item_described || cloud_described))
        {
            return;
        }

        msg_channel_type channel = MSGCH_EXAMINE;
        if (feat == DNGN_FLOOR
            || feat == DNGN_FLOOR_SPECIAL
            || feat == DNGN_SHALLOW_WATER
            || feat == DNGN_DEEP_WATER)
        {
            channel = MSGCH_EXAMINE_FILTER;
        }
        mpr(feature_desc.c_str(), channel);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////
// targeting_behaviour

targeting_behaviour::targeting_behaviour(bool look_around)
    : just_looking(look_around), compass(false)
{
}

targeting_behaviour::~targeting_behaviour()
{
}

int targeting_behaviour::get_key()
{
#ifdef USE_TILE
    mouse_control mc(MOUSE_MODE_TARGET_DIR);
#endif

    if (!crawl_state.is_replaying_keys())
        flush_input_buffer(FLUSH_BEFORE_COMMAND);

    return unmangle_direction_keys( getchm(KC_TARGETING), KC_TARGETING,
                                    false, false);
}

command_type targeting_behaviour::get_command(int key)
{
    if (key == -1)
        key = get_key();

    switch ( key )
    {
    case ESCAPE:
    case 'x':  return CMD_TARGET_CANCEL;

#ifdef USE_TILE
    case CK_MOUSE_MOVE:  return CMD_TARGET_MOUSE_MOVE;
    case CK_MOUSE_CLICK: return CMD_TARGET_MOUSE_SELECT;
#endif

#ifdef WIZARD
    case 'F':  return CMD_TARGET_WIZARD_MAKE_FRIENDLY;
    case 'P':  return CMD_TARGET_WIZARD_BLESS_MONSTER;
    case 's':  return CMD_TARGET_WIZARD_MAKE_SHOUT;
    case 'g':  return CMD_TARGET_WIZARD_GIVE_ITEM;
#endif
    case 'v':  return CMD_TARGET_DESCRIBE;
    case '?':  return CMD_TARGET_HELP;
    case ' ':  return just_looking? CMD_TARGET_CANCEL : CMD_TARGET_SELECT;
#ifdef WIZARD
    case CONTROL('C'): return CMD_TARGET_CYCLE_BEAM;
#endif
    case ':':  return CMD_TARGET_HIDE_BEAM;
    case '\r': return CMD_TARGET_SELECT;
    case '.':  return CMD_TARGET_SELECT;
    case '5':  return CMD_TARGET_SELECT;
    case '!':  return CMD_TARGET_SELECT_ENDPOINT;

    case '\\':
    case '\t': return CMD_TARGET_FIND_PORTAL;
    case '^':  return CMD_TARGET_FIND_TRAP;
    case '_':  return CMD_TARGET_FIND_ALTAR;
    case '<':  return CMD_TARGET_FIND_UPSTAIR;
    case '>':  return CMD_TARGET_FIND_DOWNSTAIR;

    case CONTROL('F'): return CMD_TARGET_CYCLE_TARGET_MODE;
    case 'p':  return CMD_TARGET_PREV_TARGET;
    case 'f':  return CMD_TARGET_MAYBE_PREV_TARGET;
    case 't':  return CMD_TARGET_MAYBE_PREV_TARGET; // for the 0.3.4 keys

    case '-':  return CMD_TARGET_CYCLE_BACK;
    case '+':
    case '=':  return CMD_TARGET_CYCLE_FORWARD;
    case ';':
    case '/':  return CMD_TARGET_OBJ_CYCLE_BACK;
    case '*':
    case '\'': return CMD_TARGET_OBJ_CYCLE_FORWARD;

    case 'b':  return CMD_TARGET_DOWN_LEFT;
    case 'h':  return CMD_TARGET_LEFT;
    case 'j':  return CMD_TARGET_DOWN;
    case 'k':  return CMD_TARGET_UP;
    case 'l':  return CMD_TARGET_RIGHT;
    case 'n':  return CMD_TARGET_DOWN_RIGHT;
    case 'u':  return CMD_TARGET_UP_RIGHT;
    case 'y':  return CMD_TARGET_UP_LEFT;

    case 'B':  return CMD_TARGET_DIR_DOWN_LEFT;
    case 'H':  return CMD_TARGET_DIR_LEFT;
    case 'J':  return CMD_TARGET_DIR_DOWN;
    case 'K':  return CMD_TARGET_DIR_UP;
    case 'L':  return CMD_TARGET_DIR_RIGHT;
    case 'N':  return CMD_TARGET_DIR_DOWN_RIGHT;
    case 'U':  return CMD_TARGET_DIR_UP_RIGHT;
    case 'Y':  return CMD_TARGET_DIR_UP_LEFT;

    default:   return CMD_NO_CMD;
    }
}

bool targeting_behaviour::should_redraw()
{
    return (false);
}
