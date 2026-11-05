/*
 * v0_describe.cpp - V0 stage 3: BRIEF descriptors (256 bits, no rotation).
 * Authors: LCK
 * External sources: https://www.youtube.com/watch?v=KgsHoJYJ4S8 , 
 * 
 * 
 *
 * TODO summary (Stage 1):
 *   
 *   Week 2 Day 13  LCK  update the author comment, cite any adapted code
 */
#include <algorithm>
#include <cstring>
#include <vector>
#include "v0.h"
#include "brief_pattern.h"

void v0_describe(const ImageF& blurred, const std::vector<Keypoint>& kps,
                 std::vector<Keypoint>& kept, std::vector<Descriptor>& desc) {
    
    const float grown_size = 2.0f, half_patch_width = BRIEF_PATCH / 2;
    
    kept.clear();
    // Filter out poinst that is to near to border
    for (int i = 0; i < kps.size(); i++){
        
        if (
            (kps[i].x - half_patch_width >= 0 && kps[i].x + half_patch_width < blurred.w) &&
            (kps[i].y - half_patch_width >= 0 && kps[i].y + half_patch_width < blurred.h)
        )
        {
            int a = kept.size() + 1;
            if (a > kept.capacity()){
                size_t new_size = (a) *  grown_size;

                kept.reserve(new_size);
            }
           
            kept.push_back(kps[i]);
        }
    }

    // Select random point to compare
    const BriefPair* pairs = brief_pattern();
    // Creating descriptor for each key point
    desc.clear();
    desc.resize(kept.size());
    for (int i = 0; i < kept.size();i++){
        memset(desc[i].bits , 0, sizeof(desc[i].bits)); // memset to be safe
        int x = kept[i].x, y = kept[i].y;
        for (int bit = 0; bit < BRIEF_BITS; bit++){ 
            int aIndex =  (y + pairs[bit].y1) * blurred.w + x + pairs[bit].x1, 
            bIndex =  (y + pairs[bit].y2) * blurred.w + x + pairs[bit].x2;
            if (blurred.data[aIndex] > blurred.data[bIndex])
            desc[i].bits[bit / 32] = (1 << (bit%32)) | desc[i].bits[bit / 32];
        }
    }
    
}
