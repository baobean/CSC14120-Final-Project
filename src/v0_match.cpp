/*
 * v0_match.cpp - V0 stage 4: brute-force Hamming matching + ratio test.
 * Authors: LCK
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. LCK replaces the body.
 */
#include "v0.h"

void v0_match(const std::vector<Descriptor>& descA, const std::vector<Descriptor>& descB,
              std::vector<Match>& matches) {
    (void)descA;
    (void)descB;
    // TODO(LCK): for each A, best and second best in B by Hamming distance;
    // keep if best < RATIO_TEST * second best.
    matches.clear();
}
