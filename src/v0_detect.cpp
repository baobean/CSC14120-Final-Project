/*
 * v0_detect.cpp - V0 stage 2: Sobel gradients + Harris corners + NMS + top K.
 * Authors: NDB
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#include "v0.h"

void v0_sobel(const ImageF& in, ImageF& Ix, ImageF& Iy) {
    // TODO(NDB, Day 7)
    Ix.w = Iy.w = in.w;
    Ix.h = Iy.h = in.h;
    Ix.data.assign((size_t)in.w * in.h, 0.0f);
    Iy.data.assign((size_t)in.w * in.h, 0.0f);
}

void v0_detect(const ImageF& blurred, std::vector<Keypoint>& kps) {
    (void)blurred;
    // TODO(NDB, Day 8-9): Harris score, NMS 3x3, sort by score, keep top MAX_KEYPOINTS.
    kps.clear();
}
