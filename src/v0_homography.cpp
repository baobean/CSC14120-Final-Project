/*
 * v0_homography.cpp - V0 stage 5: RANSAC + 4-point DLT.
 * Authors: DHDH
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. DHDH replaces the body.
 */
#include "v0.h"

bool v0_homography(const std::vector<Keypoint>& kpA, const std::vector<Keypoint>& kpB,
                   const std::vector<Match>& matches, Homography& H_BtoA, int& inliers) {
    (void)kpA;
    (void)kpB;
    (void)matches;
    // TODO(DHDH): RANSAC_ITERS iterations (seed RANSAC_SEED), 4 random matches,
    // solve 8x8 system (h33 = 1), count inliers within RANSAC_THRESH px, keep best.
    H_BtoA = Homography{{1, 0, 0, 0, 1, 0, 0, 0, 1}};
    inliers = 0;
    return false;
}
