/*
 * brief_pattern.h - the 256 fixed BRIEF pixel pairs, shared by V0 and all GPU versions.
 * Authors: LCK
 * External sources: none
 *
 * Header created by NDB (proposal from the Week 1 check-in). LCK owns the implementation.
 */
#pragma once

#include <cstdint>
#include "common.h"

// One test of the descriptor: compare pixel (x1, y1) with pixel (x2, y2).
// Offsets are relative to the keypoint, in [-BRIEF_PATCH/2, BRIEF_PATCH/2] = [-15, 15].
struct BriefPair {
    int8_t x1, y1, x2, y2;
};

// Returns BRIEF_BITS pairs. The table is built once (seed BRIEF_SEED) and never changes,
// so V1+ can copy it to constant memory and get the same descriptors as V0.
const BriefPair* brief_pattern();
