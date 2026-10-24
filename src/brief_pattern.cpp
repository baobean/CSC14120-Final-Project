/*
 * brief_pattern.cpp - builds the 256 fixed BRIEF pixel pairs.
 * Authors: LCK
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. LCK replaces the body.
 *
 * TODO summary (Stage 1):
 *   Week 1 Day 6   LCK  generate the 256 pairs from BRIEF_SEED
 *   Week 2 Day 13  LCK  update the author comment, cite any adapted code
 */
#include "brief_pattern.h"

const BriefPair* brief_pattern() {
    static BriefPair pairs[BRIEF_BITS] = {};

    // TODO(LCK, Week 1 Day 6): fill `pairs` once (use a static bool `built` flag).
    //   Use std::mt19937 rng(BRIEF_SEED). Take each coordinate as
    //   (int)(rng() % BRIEF_PATCH) - BRIEF_PATCH / 2, which gives -15..15.
    //   Use rng() % n, NOT std::uniform_int_distribution: the mt19937 output is the same
    //   on every compiler, the distribution is not, and V0 and V1+ must get the same table.
    //   If both points of a pair are equal, draw that pair again.

    return pairs;
}
