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
 *   Week 2 Day 12  DHDH  measure the H error on the synthetic pair for the notebook
 */
#include <cmath>
#include <utility>

#include "ransac_samples.h"
#include "v0.h"

// Maps (x, y) with H and divides by w. Returns false if w is ~0 (point at infinity).
static bool map_point(const Homography& H, double x, double y, double& ox, double& oy) {
    const double* h = H.h;
    double w = h[6] * x + h[7] * y + h[8];
    if (std::fabs(w) < 1e-12) return false;
    ox = (h[0] * x + h[1] * y + h[2]) / w;
    oy = (h[3] * x + h[4] * y + h[5]) / w;
    return true;
}

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
    H_BtoA = Homography{{1, 0, 0, 0, 1, 0, 0, 0, 1}};
    inliers = 0;

    // The same sample list is used by every version, so all of them test the same
    // hypotheses in the same order.
    std::vector<int> s;
    make_ransac_samples((int)matches.size(), s);
    if (s.empty()) return false;

    const double thresh2 = RANSAC_THRESH * RANSAC_THRESH;
    int best = -1;
    for (int it = 0; it < RANSAC_ITERS; it++) {
        double xb[4], yb[4], xa[4], ya[4];
        for (int k = 0; k < 4; k++) {
            const Match& m = matches[s[(size_t)it * 4 + k]];
            xb[k] = kpB[m.idxB].x;
            yb[k] = kpB[m.idxB].y;
            xa[k] = kpA[m.idxA].x;
            ya[k] = kpA[m.idxA].y;
        }
        Homography H;
        if (!solve_h4(xb, yb, xa, ya, H)) continue;

        // Inlier: B mapped by H lands within RANSAC_THRESH pixels of its match in A.
        int count = 0;
        for (const Match& m : matches) {
            double px, py;
            if (!map_point(H, kpB[m.idxB].x, kpB[m.idxB].y, px, py)) continue;
            double dx = px - kpA[m.idxA].x, dy = py - kpA[m.idxA].y;
            if (dx * dx + dy * dy < thresh2) count++;
        }
        // Strictly greater: on a tie the earlier iteration wins (same rule on the GPU).
        if (count > best) {
            best = count;
            H_BtoA = H;
        }
    }
    if (best < 0) return false;
    inliers = best;
    return true;
}
