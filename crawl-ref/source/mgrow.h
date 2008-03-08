/*
 *  File:       mgrow.cc
 *  Summary:    Monster level-up code.
 *  Written by: dshaligram on Fri Oct 26 08:33:37 2007 UTC
 *
 *  Modified for Crawl Reference by $Author: dshaligram $ on $Date: 2007-10-25T09:49:39.320031Z $
 */

#ifndef __MGROW_H__
#define __MGROW_H__

#include "AppHdr.h"
#include "FixVec.h"

// Monster level-up data.

struct monster_level_up
{
    monster_type before, after;
    int chance;     // Chance in 1000 of the monster growing up,
                    // defaults to 1000.
    
    bool adjust_hp; // If hp post growing up is less than minimum, adjust it.

    monster_level_up(monster_type _before, monster_type _after,
                     int _chance = 1000, bool _adjust = true)
        : before(_before), after(_after), chance(_chance), adjust_hp(_adjust)
    {
    }
};

const int MAX_MONS_HD = 27;
class mons_experience_levels
{
public:
    mons_experience_levels();
    unsigned operator [] (int xl) const
    {
        return mexp[xl];
    }
private:
    FixedVector<unsigned, MAX_MONS_HD + 1> mexp;
};

#endif
