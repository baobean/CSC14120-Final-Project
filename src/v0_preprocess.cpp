/*
 * v0_preprocess.cpp - V0 stage 1: grayscale + separable Gaussian blur.
 * Authors: NDB
 * External sources: none (standard formulas; luma weights from ITU-R BT.601)
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#include <cmath>

#include "v0.h"

void v0_grayscale(const ImageRGB& in, ImageF& gray) {
    size_t n = (size_t)in.w * in.h;
    gray.w = in.w;
    gray.h = in.h;
    gray.data.resize(n);
    const uint8_t* p = in.data.data();
    for (size_t i = 0; i < n; i++)
        gray.data[i] = 0.299f * p[3 * i] + 0.587f * p[3 * i + 1] + 0.114f * p[3 * i + 2];
}

// 1D Gaussian weights for offsets -BLUR_RADIUS..BLUR_RADIUS, normalized to sum 1.
static void gaussian_weights(float k[2 * BLUR_RADIUS + 1]) {
    float sum = 0.0f;
    for (int i = -BLUR_RADIUS; i <= BLUR_RADIUS; i++) {
        k[i + BLUR_RADIUS] = std::exp(-(float)(i * i) / (2.0f * BLUR_SIGMA * BLUR_SIGMA));
        sum += k[i + BLUR_RADIUS];
    }
    for (int i = 0; i < 2 * BLUR_RADIUS + 1; i++) k[i] /= sum;
}

// A 2D Gaussian is the product of two 1D Gaussians, so we blur the rows first and
// then the columns: 2 * 5 = 10 multiply-adds per pixel instead of 5 * 5 = 25.
void v0_gaussian_blur(const ImageF& in, ImageF& out) {
    const int w = in.w, h = in.h;
    float k[2 * BLUR_RADIUS + 1];
    gaussian_weights(k);

    // Row pass: in -> tmp
    std::vector<float> tmp((size_t)w * h);
    for (int y = 0; y < h; y++) {
        const float* row = &in.data[(size_t)y * w];
        for (int x = 0; x < w; x++) {
            float s = 0.0f;
            for (int i = -BLUR_RADIUS; i <= BLUR_RADIUS; i++)
                s += k[i + BLUR_RADIUS] * row[clampi(x + i, 0, w - 1)];
            tmp[(size_t)y * w + x] = s;
        }
    }

    // Column pass: tmp -> out
    out.w = w;
    out.h = h;
    out.data.resize((size_t)w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float s = 0.0f;
            for (int i = -BLUR_RADIUS; i <= BLUR_RADIUS; i++)
                s += k[i + BLUR_RADIUS] * tmp[(size_t)clampi(y + i, 0, h - 1) * w + x];
            out.data[(size_t)y * w + x] = s;
        }
    }
}
