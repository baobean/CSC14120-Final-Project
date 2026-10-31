/*
 * ransac_samples.cpp - builds the RANSAC sample list.
 * Authors: NDB (written for DHDH's stage; reviewed by DHDH)
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#include "ransac_samples.h"

#include <random>

void make_ransac_samples(int num_matches, std::vector<int>& samples) {
    samples.clear();
    if (num_matches < 4) return;

    // rng() % n, NOT std::uniform_int_distribution: mt19937's output is fixed by the
    // C++ standard, the distribution is not, so this list is the same on every compiler.
    std::mt19937 rng(RANSAC_SEED);
    samples.resize((size_t)RANSAC_ITERS * 4);
    for (int it = 0; it < RANSAC_ITERS; it++) {
        int* s = &samples[(size_t)it * 4];
        for (int k = 0; k < 4; k++) {
            bool repeated;
            do {
                s[k] = (int)(rng() % (unsigned)num_matches);
                repeated = false;
                for (int j = 0; j < k; j++) repeated = repeated || s[j] == s[k];
            } while (repeated);
        }
    }
}
