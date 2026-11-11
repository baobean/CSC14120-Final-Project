/*
 * v0_match.cpp - V0 stage 4: brute-force Hamming matching + Lowe ratio test.
 * Authors: LCK
 * External sources: 
 *  - Fast bit counting: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
 *  - Lowe's radios: https://docs.opencv.org/3.4.20/d5/d6f/tutorial_feature_flann_matcher.html 
 *                   https://sites.cc.gatech.edu/classes/AY2016/cs4476_fall/results/proj2/html/sshah426/index.html
 * AI assistance: NDB's Claude is uses to checks for errors
 *
 */
#include <cstdlib>
#include <vector>
#include <utility>
#include "v0.h"

static const unsigned char BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};
// Keep for reference
// static int count_bit(const Descriptor& value){
//     int total = 0;
//     uint32_t v;
//     for (int i = 0; i < BRIEF_BITS / 32;i++){
//         v = value.bits[i];
//         total += BitsSetTable256[v & 0xff] + 
//         BitsSetTable256[(v >> 8) & 0xff] + 
//         BitsSetTable256[(v >> 16) & 0xff] + 
//         BitsSetTable256[v >> 24]; 
//     }
//     return total;
// }

int count_bit(uint32_t v){
    #ifdef _MSC_VER
        return BitsSetTable256[v & 0xff] + 
                BitsSetTable256[(v >> 8) & 0xff] + 
                BitsSetTable256[(v >> 16) & 0xff] + 
                BitsSetTable256[v >> 24]; 
    #else
        return __builtin_popcountll(v);
    #endif
}

int hamming_distance(const Descriptor& A, const Descriptor& B){
    int total = 0;
    uint32_t v;
    // Fast bit counting from: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
    for (int i = 0; i < BRIEF_BITS / 32 ;i++){
        v = A.bits[i] ^ B.bits[i];
        total += count_bit(v);
    }
    return total;
}


void v0_match(const std::vector<Descriptor>& descA, const std::vector<Descriptor>& descB,std::vector<Match>& matches) {
    matches.clear();
    if (descB.size() < 2) {
        return;
    }

    struct cmp_result{
        int r = 257, index = -1;
    };
    std::vector<std::pair<cmp_result,cmp_result>> results(descA.size());
    matches.reserve(descA.size());
    cmp_result ref1; // A copied refference to allow better access
    for (int i = 0; i < descA.size(); i++){
        for (int j = 0; j < descB.size(); j++){
            int dis = hamming_distance(descA[i], descB[j]);
            ref1 = results[i].first;
            if (dis < ref1.r){
                results[i].second = ref1;
                results[i].first = {dis, j};
            }
            else if (dis < results[i].second.r){
                results[i].second = {dis, j};
            }
        }
    }

    for (int i = 0; i < descA.size(); i++){
        if (results[i].first.index != -1 && results[i].first.r < RATIO_TEST * results[i].second.r)
            matches.push_back({i, results[i].first.index, results[i].first.r});
    }
}
