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
#include <vector>

#include "v0.h"
#include "brief_pattern.h"

void v0_describe(const ImageF& blurred, const std::vector<Keypoint>& kps,
                 std::vector<Keypoint>& kept, std::vector<Descriptor>& desc) {

    const float epsilon = 0.01, grown_size = 1.5f, half_patch_width = BRIEF_PATCH / 2;
    float maxR = 0;
    kept.clear();
    // Filter out poinst that is to near to border
    for (int i = 0; i < kps.size(); i++){
        if (
            (kps[i].x - half_patch_width >= 0 && kps[i].x + half_patch_width < blurred.w) &&
            (kps[i].y - half_patch_width >= 0 && kps[i].y + half_patch_width < blurred.h)
        )
        {
            if (kept.size() + 1 > kept.capacity()) kept.reserve(kept.size() * grown_size);
            kept.push_back(kps[i]);
            maxR = (maxR > kps[i].score)?maxR:kps[i].score;
        }
    }
    
    // Filter out flat section
    sort(kept.begin(), kept.end(),[](const Keypoint& a, const Keypoint& b) {return a.score > b.score;}); 
    auto remain_size = lower_bound(kept.begin(), kept.end(), epsilon * maxR, [](const Keypoint& a, double b) {return a.score > b;});
    if (remain_size - kept.begin() != kept.size())
        kept.erase(remain_size + 1, kept.end());
    
    // Select random point to compare
    const BriefPair* pairs = brief_pattern();
    
    // Creating descriptor for each key point
    desc.clear();
    desc.resize(kept.size());
    for (int i = 0; i < kept.size();i++){
        memset(desc[i].bits, 0, BRIEF_BITS); // memset to be safe

        int x = kept[i].x, y = kept[i].y;
        for (int bit = 0; bit < BRIEF_BITS; bit++){ 
            // Calculating the position to read from
            int aIndex =  (x + pairs[bit].x1) * blurred.w + y + pairs[bit].y1 , 
                bIndex =  (x + pairs[bit].x2) * blurred.w + y + pairs[bit].y2;
            if (blurred.data[aIndex] > blurred.data[bIndex])
                desc[i].bits[bit / 32] = (1 << (bit%32)) | desc[i].bits[bit / 32];
        }
    }
}
