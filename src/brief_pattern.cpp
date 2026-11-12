/*
 * brief_pattern.cpp - builds the 256 fixed BRIEF pixel pairs.
 * Authors: Chí Khải 
 * External sources: 
 *  - BRIEF descriptor: https://gilscvblog.com/2013/09/19/a-tutorial-on-binary-descriptors-part-2-the-brief-descriptor/
 * 
 */
#include "brief_pattern.h"

static bool in(BriefPair* arr, int x, int y){
    for (int i = 0; i < BRIEF_BITS;i++){
        if ((arr[i].x1 == x && arr[i].y1 == y) || (arr[i].x2 == x&& arr[i].y2 == y)) return true;
    }
    return false;
}

const BriefPair* brief_pattern() {
    static BriefPair pairs[BRIEF_BITS] = {{0,0,0,0}};
    if (pairs[0].x1 == 0 && pairs[0].x2 == 0 && pairs[0].y1 == 0 && pairs[0].y2 == 0){
        std::mt19937 gen(BRIEF_SEED);
        
        std::uniform_int_distribution uni(-15, 15);
        for (int i = 0, x, y; i < BRIEF_BITS; i++){
            x = uni(gen);
            y = uni(gen);
            while (in(pairs, x, y)){
                x = uni(gen);
                y = uni(gen);
            }
            pairs[i].x1 = x;
            pairs[i].y1 = y;
        }
        for (int i = 0, x, y; i < BRIEF_BITS; i++){
            x = uni(gen);
            y = uni(gen);
            while (in(pairs, x, y)){
                x = uni(gen);
                y = uni(gen);
            }
            pairs[i].x2 = x;
            pairs[i].y2 = y;
        }
    }
    return pairs;
}
