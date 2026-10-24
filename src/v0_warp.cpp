/*
 * v0_warp.cpp - V0 stage 6: inverse warp (bilinear) + linear blend.
 * Authors: DHDH
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. DHDH replaces the body.
 */
#include "v0.h"

void v0_warp_blend(const ImageRGB& A, const ImageRGB& B, const Homography& H_BtoA,
                   ImageRGB& out) {
    (void)B;
    (void)H_BtoA;
    // TODO(DHDH): canvas from the 4 warped corners of B, inverse warp with H^-1 and
    // bilinear sampling, linear blend weight 1 -> 0 across the overlap.
    out = A;
}
