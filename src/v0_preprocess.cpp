/*
 * v0_preprocess.cpp - V0 stage 1: grayscale + separable Gaussian blur.
 * Authors: NDB
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#include "v0.h"

void v0_grayscale(const ImageRGB& in, ImageF& gray) {
    // TODO(NDB, Day 5)
    gray.w = in.w;
    gray.h = in.h;
    gray.data.assign((size_t)in.w * in.h, 0.0f);
}

void v0_gaussian_blur(const ImageF& in, ImageF& out) {
    // TODO(NDB, Day 6)
    out = in;
}
