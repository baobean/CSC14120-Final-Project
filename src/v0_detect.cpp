/*
 * v0_detect.cpp - V0 stage 2: Sobel gradients + Harris corners + NMS + top K.
 * Authors: NDB
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * TODO summary (Stage 1):
 *   Week 1 Day 7   NDB  Sobel Ix, Iy                                   (done)
 *   Week 2 Day 8   NDB  Harris score R for each pixel
 *   Week 2 Day 9   NDB  NMS 3x3, sort by score, keep top K; keypoint debug image (debug_draw.cpp)
 *   Week 2 Day 10  NDB  check keypoint count and spread on all sites
 */
#include "v0.h"

// Sobel kernels (x grows right, y grows down):
//        [-1 0 1]          [-1 -2 -1]
//   Ix = [-2 0 2]     Iy = [ 0  0  0]
//        [-1 0 1]          [ 1  2  1]
// Neighbors outside the image are clamped to the nearest edge pixel.
void v0_sobel(const ImageF& in, ImageF& Ix, ImageF& Iy) {
    const int w = in.w, h = in.h;
    Ix.w = Iy.w = w;
    Ix.h = Iy.h = h;
    Ix.data.resize((size_t)w * h);
    Iy.data.resize((size_t)w * h);
    const float* p = in.data.data();

    for (int y = 0; y < h; y++) {
        const float* up = p + (size_t)clampi(y - 1, 0, h - 1) * w;
        const float* mid = p + (size_t)y * w;
        const float* down = p + (size_t)clampi(y + 1, 0, h - 1) * w;
        for (int x = 0; x < w; x++) {
            int xl = clampi(x - 1, 0, w - 1);
            int xr = clampi(x + 1, 0, w - 1);
            float gx = (up[xr] + 2.0f * mid[xr] + down[xr]) - (up[xl] + 2.0f * mid[xl] + down[xl]);
            float gy = (down[xl] + 2.0f * down[x] + down[xr]) - (up[xl] + 2.0f * up[x] + up[xr]);
            Ix.data[(size_t)y * w + x] = gx;
            Iy.data[(size_t)y * w + x] = gy;
        }
    }
}

void v0_detect(const ImageF& blurred, std::vector<Keypoint>& kps) {
    ImageF Ix, Iy;
    v0_sobel(blurred, Ix, Iy);
    kps.clear();

    // TODO(NDB, Week 2 Day 8): Harris score.
    //   For each pixel, sum over a HARRIS_WIN x HARRIS_WIN window (clamped border):
    //     Sxx = sum Ix*Ix, Syy = sum Iy*Iy, Sxy = sum Ix*Iy
    //   R = (Sxx*Syy - Sxy*Sxy) - HARRIS_K * (Sxx + Syy)^2, stored in an ImageF.

    // TODO(NDB, Week 2 Day 9): NMS 3x3 + top K.
    //   Keep pixel (x, y) only if R > 0 and R is strictly greater than its 8 neighbors
    //   (clamped border). Sort by score, highest first; on equal score sort by y then x,
    //   so V1+ (atomics give a random order) can sort back to the same list.
    //   Keep the first MAX_KEYPOINTS.
}
