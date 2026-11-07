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
#include <cstdlib>
#include <vector>
#include <utility>
#include "v0.h"

static int count_bit(const Descriptor& v){
    int total = 0;
    for (int i = 0; i < BRIEF_BITS;i++){
        if ((v.bits[i/32] & (1ll << (i % 32))) != 0){
            total += 1;
        }
    }
    return total;
}

static int hamming_distance(const Descriptor& A, const Descriptor& B){
    Descriptor result;
    for (int i = 0; i < BRIEF_BITS / 32 ;i++){
        result.bits[i] = A.bits[i] ^ B.bits[i];
    }
    return count_bit(result);
}


void v0_match(const std::vector<Descriptor>& descA, const std::vector<Descriptor>& descB,std::vector<Match>& matches) {
    if (descB.size() < 2) {
        matches.clear();
        return;
    }

    struct cmp_result{
        int r = 257, index = -1;
    };
    std::vector<std::pair<cmp_result,cmp_result>> results(descA.size());

    cmp_result ref1; // A copied refference to allow better access
    for (int i = 0; i < descA.size(); i++){
        for (int j = 0; j < descB.size(); j++){
            int dis = hamming_distance(descA[i], descB[j]);
            ref1 = results[i].first;
            if (dis < ref1.r){
                results[i].second = results[i].first;
                results[i].first = {dis, j};
            }
            else if (dis != ref1.r && dis < results[i].second.r){
                results[i].second = {dis, j};
            }
        }
        
    }

    for (int i = 0; i < descA.size(); i++){
        if (results[i].first.index != -1 && results[i].first.r < RATIO_TEST * results[i].second.r)
            matches.push_back({i, results[i].first.index, results[i].first.r});
    }

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
