/*
 * v0_warp.cpp - V0 stage 6: inverse warp (bilinear) + linear blend.
 * Authors: DHDH
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. DHDH replaces the body.
 *
 * TODO summary (Stage 1):
 *   Week 2 Day 9   DHDH  output canvas size from the 4 warped corners of B
 *   Week 2 Day 10  DHDH  inverse warp with bilinear sampling
 *   Week 2 Day 11  DHDH  linear blend in the overlap
 *   Week 2 Day 13  DHDH  update the author comment, cite any adapted code
 *   Week 2 Day 14  DHDH  one panorama per site + the challenging pair (notebook)
 */
#include "v0.h"

void v0_warp_blend(const ImageRGB& A, const ImageRGB& B, const Homography& H_BtoA,
                   ImageRGB& out) {
    (void)B;
    (void)H_BtoA;
    out = A;

    // TODO(DHDH, Week 2 Day 9): canvas.
    //   Map the 4 corners of B, (0,0) (wB-1,0) (0,hB-1) (wB-1,hB-1), with H (divide by w).
    //   Canvas = bounding box of those 4 points and A's rectangle [0, wA) x [0, hA).
    //   Offset (tx, ty) = -min x, -min y (so A sits at (tx, ty) in the canvas).
    //   Safety: if the canvas is wider or taller than 4x A (bad H), print a warning and
    //   fall back to out = A.

    // TODO(DHDH, Week 2 Day 10): inverse warp.
    //   Compute Hinv = inverse of H (3x3, via the adjugate / determinant).
    //   For every canvas pixel (x, y): point in A's frame = (x - tx, y - ty);
    //   point in B = Hinv * that point (divide by w). If it is inside B, sample B with
    //   bilinear interpolation (4 neighbors, weights from the fractional parts).

    // TODO(DHDH, Week 2 Day 11): linear blend.
    //   A only -> A pixel; B only -> warped B pixel; neither -> black.
    //   Both -> alpha * A + (1 - alpha) * B, where alpha goes from 1 to 0 across the
    //   overlap in x (from the overlap's left edge to its right edge), so the seam fades.
}
