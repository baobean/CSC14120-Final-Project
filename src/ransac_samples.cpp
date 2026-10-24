/*
 * ransac_samples.cpp - builds the RANSAC sample list.
 * Authors: DHDH
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. DHDH replaces the body.
 *
 * TODO summary (Stage 1):
 *   Week 1 Day 7   DHDH  generate RANSAC_ITERS x 4 distinct indexes from RANSAC_SEED
 *   Week 2 Day 13  DHDH  update the author comment, cite any adapted code
 */
#include "ransac_samples.h"

void make_ransac_samples(int num_matches, std::vector<int>& samples) {
    samples.clear();
    if (num_matches < 4) return;

    // TODO(DHDH, Week 1 Day 7): fill RANSAC_ITERS * 4 indexes.
    //   std::mt19937 rng(RANSAC_SEED); index = rng() % num_matches.
    //   Use rng() % n, NOT std::uniform_int_distribution (same reason as brief_pattern.cpp:
    //   mt19937 output is identical on every compiler, the distribution is not).
    //   Inside one iteration, if an index repeats, draw it again until all 4 are different.
}
