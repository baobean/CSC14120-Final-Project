/*
 * v0_describe.cpp - V0 stage 3: BRIEF descriptors (256 bits, no rotation).
 * Authors: LCK
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. LCK replaces the body.
 *
 * TODO summary (Stage 1):
 *   Week 1 Day 6   LCK  256 fixed pixel pairs -> brief_pattern.cpp
 *   Week 1 Day 7   LCK  256-bit descriptor for each keypoint (this file)
 *   Week 2 Day 8   LCK  skip keypoints closer than 16 px to the border (this file)
 *   Week 2 Day 13  LCK  update the author comment, cite any adapted code
 */
#include "brief_pattern.h"
#include "v0.h"

void v0_describe(const ImageF& blurred, const std::vector<Keypoint>& kps,
                 std::vector<Keypoint>& kept, std::vector<Descriptor>& desc) {
    (void)blurred;
    (void)kps;
    kept.clear();
    desc.clear();

    // TODO(LCK, Week 2 Day 8): border skip.
    //   Round each keypoint to integer (cx, cy). Skip it if cx < 16 || cy < 16 ||
    //   cx >= w - 16 || cy >= h - 16, so the whole 31x31 patch is inside the image.
    //   Keep the input order (it is sorted by score) so V1+ gives the same order.

    // TODO(LCK, Week 1 Day 7): descriptor for each kept keypoint.
    //   const BriefPair* pat = brief_pattern();
    //   For i = 0..BRIEF_BITS-1:
    //     a = blurred(cx + pat[i].x1, cy + pat[i].y1)
    //     b = blurred(cx + pat[i].x2, cy + pat[i].y2)
    //     bit i = 1 if a > b (first pixel brighter), else 0
    //   Store bit i in desc.bits[i / 32] at bit position (i % 32). Zero the words first.
    //   Push the keypoint to `kept` and the descriptor to `desc` (same index).
}
