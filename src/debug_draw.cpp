/*
 * debug_draw.cpp - debug images for keypoints and matches (not timed, debug only).
 * Authors: NDB (draw_keypoints), LCK (draw_matches)
 * External sources: none
 * AI assistance (draw_keypoints): drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * TODO summary (Stage 1):
 *   Week 2 Day 10  LCK  draw_matches
 */
#include "debug_draw.h"

#include <cmath>

void draw_keypoints(const ImageRGB& img, const std::vector<Keypoint>& kps, ImageRGB& out) {
    out = img;
    // 5x5 hollow red square centered on each keypoint; pixels outside the image are skipped.
    for (const Keypoint& kp : kps) {
        int cx = (int)std::lround(kp.x), cy = (int)std::lround(kp.y);
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx != -2 && dx != 2 && dy != -2 && dy != 2) continue;  // inside: hollow
                int x = cx + dx, y = cy + dy;
                if (x < 0 || x >= out.w || y < 0 || y >= out.h) continue;
                uint8_t* p = &out.data[((size_t)y * out.w + x) * 3];
                p[0] = 255;
                p[1] = 0;
                p[2] = 0;
            }
        }
    }
}

void draw_matches(const ImageRGB& A, const std::vector<Keypoint>& kpA,
                  const ImageRGB& B, const std::vector<Keypoint>& kpB,
                  const std::vector<Match>& matches, ImageRGB& out) {
    (void)kpA;
    (void)kpB;
    (void)matches;
    (void)B;
    out = A;
    // TODO(LCK, Week 2 Day 10): canvas of size (A.w + B.w) x max(A.h, B.h), copy A to the
    //   left and B to the right. For each match draw a straight line (simple DDA: step
    //   max(|dx|, |dy|) times) from kpA[idxA] to kpB[idxB] + (A.w, 0), in a color that
    //   changes with the match index so neighboring lines are easy to tell apart.
}
