/*
 * v0_homography.cpp - V0 stage 5: RANSAC + 4-point DLT.
 * Authors: DHDH
 * External sources: none
 *
 * STUB created by NDB so the pipeline builds. DHDH replaces the body.
 *
 * TODO summary (Stage 1):
 *   Week 1 Day 6   DHDH  solve H from 4 point pairs (8x8 Gaussian elimination), test it
 *   Week 1 Day 7   DHDH  RANSAC loop (sample list from ransac_samples.cpp)
 *   Week 2 Day 8   DHDH  test on the synthetic pair, check the H error is small
 *   Week 2 Day 12  DHDH  measure the H error on the synthetic pair for the notebook
 *   Week 2 Day 13  DHDH  update the author comment, cite any adapted code
 */
#include "ransac_samples.h"
#include "v0.h"

// TODO(DHDH, Week 1 Day 6): add
//   static bool solve_h4(const double xb[4], const double yb[4],
//                        const double xa[4], const double ya[4], Homography& H);
//   Each pair (xb, yb) -> (xa, ya) gives 2 rows of an 8x8 system A * h = b, with h33 = 1:
//     [xb yb 1  0  0 0 -xa*xb -xa*yb] h = xa
//     [ 0  0 0 xb yb 1 -ya*xb -ya*yb] h = ya
//   Solve with Gaussian elimination + partial pivoting. Return false if a pivot is
//   below ~1e-10 (the 4 points are degenerate, e.g. 3 on one line).
//   Test: build H from 4 exact points made with a known H and check you get it back.

bool v0_homography(const std::vector<Keypoint>& kpA, const std::vector<Keypoint>& kpB,
                   const std::vector<Match>& matches, Homography& H_BtoA, int& inliers) {
    (void)kpA;
    (void)kpB;
    H_BtoA = Homography{{1, 0, 0, 0, 1, 0, 0, 0, 1}};
    inliers = 0;

    // TODO(DHDH, Week 1 Day 7): RANSAC.
    //   std::vector<int> s; make_ransac_samples((int)matches.size(), s);
    //   If s is empty (fewer than 4 matches), return false.
    //   For it = 0..RANSAC_ITERS-1:
    //     take matches s[4*it..4*it+3]; point B = kpB[m.idxB], point A = kpA[m.idxA];
    //     solve_h4 -> H (skip the iteration if it fails);
    //     count inliers: for every match, map B with H (divide by w) and count it if the
    //     distance to A is < RANSAC_THRESH pixels;
    //     keep H if its count is strictly greater than the best so far (first one wins ties).
    //   Write the best H and its count; return true if a best H exists.
    (void)matches;
    return false;
}
