/*
 * v0_match.cpp - V0 stage 4: brute-force Hamming matching + Lowe ratio test.
 * Authors: LCK
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. LCK replaces the body.
 *
 * TODO summary (Stage 1):
 *   Week 2 Day 9   LCK  Hamming distance, best and second best for each A (this file)
 *   Week 2 Day 10  LCK  ratio test (this file); match debug image (debug_draw.cpp)
 *   Week 2 Day 12  LCK  try ratio 0.7 vs 0.8 on the challenging pair; keep 0.8 unless bad
 *   Week 2 Day 13  LCK  update the author comment, cite any adapted code
 */
#include "v0.h"

void v0_match(const std::vector<Descriptor>& descA, const std::vector<Descriptor>& descB,
              std::vector<Match>& matches) {
    (void)descA;
    (void)descB;
    matches.clear();

    // TODO(LCK, Week 2 Day 9): Hamming distance and the two nearest neighbors.
    //   dist(a, b) = sum over the 8 words of popcount(a.bits[k] ^ b.bits[k]), 0..256.
    //   Count bits with a plain loop or __builtin_popcount (no hand-written SIMD).
    //   For each i in descA, loop over every j in descB and keep:
    //     best (smallest dist, its j) and second best (second smallest dist).
    //   On equal distance keep the smaller j, so V1+ gives the same answer.

    // TODO(LCK, Week 2 Day 10): ratio test.
    //   Keep the match {i, best_j, best} only if best < RATIO_TEST * second.
    //   If descB has fewer than 2 descriptors, return no matches.
    //   `matches` is in order of i (image A index).
}
