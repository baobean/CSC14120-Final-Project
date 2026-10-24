/*
 * debug_draw.cpp - debug images for keypoints and matches (not timed, debug only).
 * Authors: NDB (draw_keypoints), LCK (draw_matches)
 * External sources: none
 *
 * STUBS created by NDB so the pipeline builds.
 *
 * TODO summary (Stage 1):
 *   Week 2 Day 9   NDB  draw_keypoints
 *   Week 2 Day 10  LCK  draw_matches
 */
#include "debug_draw.h"

void draw_keypoints(const ImageRGB& img, const std::vector<Keypoint>& kps, ImageRGB& out) {
    (void)kps;
    out = img;
    // TODO(NDB, Week 2 Day 9): for each keypoint, paint a 5x5 hollow square in red
    //   (255, 0, 0) centered on the rounded (x, y); skip pixels outside the image.
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
