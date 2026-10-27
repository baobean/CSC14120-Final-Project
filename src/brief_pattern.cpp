/*
 * brief_pattern.cpp - builds the 256 fixed BRIEF pixel pairs.
 * Authors: LCK 
 * External sources: none
 * AI assistance: "None" (The implementation is based on the comment given by NDB, which - from looking at the comment and speaking with NDB - is claude)
 * 
 *   Week 2 Day 13  LCK  update the author comment, cite any adapted code
 */
#include "brief_pattern.h"

const BriefPair* brief_pattern() {
    static BriefPair pairs[BRIEF_BITS] = {{0,0,0,0}};
    if (pairs[0].x1 == 0 && pairs[0].x2 == 0 && pairs[0].y1 == 0 && pairs[0].y2 == 0){
        std::mt19937 gen(BRIEF_SEED);
        for (int i = 0; i < BRIEF_BITS; i++){
            pairs[i].x1 = (int)(gen() % BRIEF_PATCH) - BRIEF_PATCH / 2;
            pairs[i].y1 = (int)(gen() % BRIEF_PATCH) - BRIEF_PATCH / 2;
            pairs[i].x2 = (int)(gen() % BRIEF_PATCH) - BRIEF_PATCH / 2;
            pairs[i].y2 = (int)(gen() % BRIEF_PATCH) - BRIEF_PATCH / 2;
            while (pairs[i].x1 == pairs[i].x2 && pairs[i].y1 == pairs[i].y2){
                pairs[i].x2 = (int)(gen() % BRIEF_PATCH) - BRIEF_PATCH / 2;
                pairs[i].y2 = (int)(gen() % BRIEF_PATCH) - BRIEF_PATCH / 2;
            }
        }
    }
    return pairs;
}
