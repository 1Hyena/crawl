/**
 * @file
 * @brief Functions for picking random entries from weighted, gradiated lists
**/

#ifndef RANDOMPICK_H
#define RANDOMPICK_H

#include "random.h"

enum distrib_type
{
    FLAT, // full chance throughout the range
    SEMI, // 50% chance at range ends, 100% in the middle
    PEAK, // 0% chance just outside range ends, 100% in the middle, range
          // ends typically get ~20% or more
    UP,   // linearly from near-zero to 100%, increasing with depth
    DOWN, // linearly from 100% at the start to near-zero
};

template <typename T>
struct random_pick_entry
{
    int minr;
    int maxr;
    int rarity; // 0..1000
    distrib_type distrib;
    T value;
};

template <typename T, int max>
class random_picker
{
public:
    T pick(const random_pick_entry<T> *weights, int level, T none);
    static int _rarity_at(const random_pick_entry<T> *pop,
                                         int depth);

protected:
    virtual bool veto(T val) { return false; }
};

template <typename T, int max>
T random_picker<T, max>::pick(const random_pick_entry<T> *weights, int level,
                              T none)
{
    struct { T value; int rarity; } valid[max];
    int nvalid = 0;
    int totalrar = 0;

    for (const random_pick_entry<T> *pop = weights; pop->value; pop++)
    {
        if (level < pop->minr || level > pop->maxr)
            continue;

        if (veto(pop->value))
            continue;

        int rar = _rarity_at(pop, level);
        ASSERT(rar > 0);

        valid[nvalid].value = pop->value;
        valid[nvalid].rarity = rar;
        totalrar += rar;
        nvalid++;
    }

    if (!nvalid)
        return none;

    totalrar = random2(totalrar); // the roll!

    for (int i = 0; i < nvalid; i++)
        if ((totalrar -= valid[i].rarity) < 0)
            return valid[i].value;

    die("random_pick roll out of range");
}

template <typename T, int max>
int random_picker<T, max>::_rarity_at(const random_pick_entry<T> *pop, int depth)
{
    int rar = pop->rarity;
    int len = pop->maxr - pop->minr;
    switch (pop->distrib)
    {
    case FLAT: // 100% everywhere
        return rar;

    case SEMI: // 100% in the middle, 50% at the edges
        if (len)
            len *= 2;
        else
            len += 2; // a single-level range
        return rar * (len - abs(pop->minr + pop->maxr - 2 * depth))
                   / len;

    case PEAK: // 100% in the middle, small at the edges, 0% outside
        len += 2; // we want it to zero outside the range, not at the edge
        return rar * (len - abs(pop->minr + pop->maxr - 2 * depth))
                   / len;

    case UP:
        return rar * (depth - pop->minr + 1) / (len + 1);

    case DOWN:
        return rar * (pop->maxr - depth + 1) / (len + 1);
    }

    die("bad distrib");
}

#endif
