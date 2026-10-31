/*
 * v0_homography.cpp - V0 stage 5: RANSAC + 4-point DLT.
 * Authors: NDB (written for DHDH's stage; reviewed by DHDH)
 * External sources: RANSAC from M. Fischler and R. Bolles, "Random Sample Consensus",
 *                   Comm. ACM 24(6), 1981. 4-point homography as in R. Szeliski,
 *                   "Computer Vision: Algorithms and Applications", 2nd ed., ch. 8.
 *                   Implemented from the formulas, no code copied.
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * TODO summary (Stage 1):
 *   Week 1 Day 7   DHDH  RANSAC loop (sample list from ransac_samples.cpp)
 *   Week 2 Day 12  DHDH  measure the H error on the synthetic pair for the notebook
 */
#include <cmath>
#include <utility>

#include "ransac_samples.h"
#include "v0.h"

// Solves H (h33 = 1) from 4 point pairs (xb, yb) -> (xa, ya).
// Each pair gives 2 rows of an 8x8 system M * h = b:
//   [xb yb 1  0  0 0 -xa*xb -xa*yb] h = xa
//   [ 0  0 0 xb yb 1 -ya*xb -ya*yb] h = ya
// Gaussian elimination with partial pivoting, then back substitution.
// Returns false if a pivot is below 1e-10 (degenerate points, e.g. 3 on one line).
static bool solve_h4(const double xb[4], const double yb[4],
                     const double xa[4], const double ya[4], Homography& H) {
    double M[8][9];  // augmented matrix [M | b]
    for (int i = 0; i < 4; i++) {
        double* r0 = M[2 * i];
        double* r1 = M[2 * i + 1];
        r0[0] = xb[i]; r0[1] = yb[i]; r0[2] = 1; r0[3] = 0;     r0[4] = 0;     r0[5] = 0;
        r0[6] = -xa[i] * xb[i]; r0[7] = -xa[i] * yb[i]; r0[8] = xa[i];
        r1[0] = 0;     r1[1] = 0;     r1[2] = 0; r1[3] = xb[i]; r1[4] = yb[i]; r1[5] = 1;
        r1[6] = -ya[i] * xb[i]; r1[7] = -ya[i] * yb[i]; r1[8] = ya[i];
    }

    for (int col = 0; col < 8; col++) {
        // Partial pivoting: bring the row with the largest |value| in this column up.
        int piv = col;
        for (int r = col + 1; r < 8; r++)
            if (std::fabs(M[r][col]) > std::fabs(M[piv][col])) piv = r;
        if (std::fabs(M[piv][col]) < 1e-10) return false;
        if (piv != col)
            for (int c = 0; c < 9; c++) std::swap(M[col][c], M[piv][c]);
        // Remove this column from the rows below.
        for (int r = col + 1; r < 8; r++) {
            double f = M[r][col] / M[col][col];
            for (int c = col; c < 9; c++) M[r][c] -= f * M[col][c];
        }
    }

    double h[8];
    for (int r = 7; r >= 0; r--) {
        double s = M[r][8];
        for (int c = r + 1; c < 8; c++) s -= M[r][c] * h[c];
        h[r] = s / M[r][r];
    }
    for (int i = 0; i < 8; i++) H.h[i] = h[i];
    H.h[8] = 1.0;
    return true;
}

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
