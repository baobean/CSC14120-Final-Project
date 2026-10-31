/*
 * v0_detect.cpp - V0 stage 2: Sobel gradients + Harris corners + NMS + top K.
 * Authors: NDB
 * External sources: Harris corner response from C. Harris and M. Stephens,
 *                   "A Combined Corner and Edge Detector", Alvey Vision Conference, 1988.
 *                   Implemented from the formula, no code copied.
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
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

// Sum of `in` over a HARRIS_WIN x HARRIS_WIN window around each pixel (clamped border).
// The box is separable like the blur: row sums first, then column sums of those,
// so 2 * 5 = 10 adds per pixel instead of 25.
static void box_sum(const std::vector<float>& in, int w, int h, std::vector<float>& out) {
    const int r = HARRIS_WIN / 2;
    std::vector<float> tmp((size_t)w * h);
    for (int y = 0; y < h; y++) {
        const float* row = &in[(size_t)y * w];
        for (int x = 0; x < w; x++) {
            float s = 0.0f;
            for (int i = -r; i <= r; i++) s += row[clampi(x + i, 0, w - 1)];
            tmp[(size_t)y * w + x] = s;
        }
    }
    out.resize((size_t)w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float s = 0.0f;
            for (int i = -r; i <= r; i++) s += tmp[(size_t)clampi(y + i, 0, h - 1) * w + x];
            out[(size_t)y * w + x] = s;
        }
    }
}

// M = [Sxx Sxy; Sxy Syy] is the structure tensor summed over the window.
// Corner: both eigenvalues large -> R > 0. Edge: one large -> R < 0. Flat: R ~ 0.
// R = det(M) - k * trace(M)^2 gives this without computing the eigenvalues.
void v0_harris(const ImageF& Ix, const ImageF& Iy, ImageF& R) {
    const int w = Ix.w, h = Ix.h;
    const size_t n = (size_t)w * h;
    std::vector<float> xx(n), yy(n), xy(n);
    for (size_t i = 0; i < n; i++) {
        float gx = Ix.data[i], gy = Iy.data[i];
        xx[i] = gx * gx;
        yy[i] = gy * gy;
        xy[i] = gx * gy;
    }
    std::vector<float> sxx, syy, sxy;
    box_sum(xx, w, h, sxx);
    box_sum(yy, w, h, syy);
    box_sum(xy, w, h, sxy);

    R.w = w;
    R.h = h;
    R.data.resize(n);
    for (size_t i = 0; i < n; i++) {
        float det = sxx[i] * syy[i] - sxy[i] * sxy[i];
        float tr = sxx[i] + syy[i];
        R.data[i] = det - HARRIS_K * tr * tr;
    }
}

void v0_detect(const ImageF& blurred, std::vector<Keypoint>& kps) {
    ImageF Ix, Iy, R;
    v0_sobel(blurred, Ix, Iy);
    v0_harris(Ix, Iy, R);
    kps.clear();

    // TODO(NDB, Week 2 Day 9): NMS 3x3 + top K.
    //   Keep pixel (x, y) only if R > 0 and R is strictly greater than its 8 neighbors
    //   (clamped border). Sort by score, highest first; on equal score sort by y then x,
    //   so V1+ (atomics give a random order) can sort back to the same list.
    //   Keep the first MAX_KEYPOINTS.
}
