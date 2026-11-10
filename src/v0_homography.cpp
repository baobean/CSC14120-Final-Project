/*
 * v0_homography.cpp - V0 stage 5: RANSAC + 4-point DLT, then a least-squares refit
 * of H on all inliers.
 * Authors: NDB (written for DHDH's stage; reviewed by DHDH, except the refit added on 2026-11-10)
 * External sources: RANSAC from M. Fischler and R. Bolles, "Random Sample Consensus",
 *                   Comm. ACM 24(6), 1981. 4-point homography and the least-squares
 *                   refit as in R. Szeliski, "Computer Vision: Algorithms and
 *                   Applications", 2nd ed., ch. 8; point normalization from
 *                   R. Hartley, "In Defense of the Eight-Point Algorithm", IEEE PAMI
 *                   19(6), 1997. Implemented from the formulas, no code copied.
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
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

// Solves the 8x8 system in the augmented matrix [M | b] (8 rows, 9 columns).
// Gaussian elimination with partial pivoting, then back substitution.
// Returns false if a pivot is below 1e-10 (degenerate points, e.g. 3 on one line).
static bool solve8(double M[8][9], double h[8]) {
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
    for (int r = 7; r >= 0; r--) {
        double s = M[r][8];
        for (int c = r + 1; c < 8; c++) s -= M[r][c] * h[c];
        h[r] = s / M[r][r];
    }
    return true;
}

// The 2 equations that one point pair (xb, yb) -> (xa, ya) gives for h (h33 = 1):
//   [xb yb 1  0  0 0 -xa*xb -xa*yb] h = xa
//   [ 0  0 0 xb yb 1 -ya*xb -ya*yb] h = ya
// Each row is 8 coefficients followed by the right-hand side.
static void dlt_rows(double xb, double yb, double xa, double ya, double r0[9], double r1[9]) {
    r0[0] = xb; r0[1] = yb; r0[2] = 1; r0[3] = 0;  r0[4] = 0;  r0[5] = 0;
    r0[6] = -xa * xb; r0[7] = -xa * yb; r0[8] = xa;
    r1[0] = 0;  r1[1] = 0;  r1[2] = 0; r1[3] = xb; r1[4] = yb; r1[5] = 1;
    r1[6] = -ya * xb; r1[7] = -ya * yb; r1[8] = ya;
}

// Solves H exactly from 4 point pairs (xb, yb) -> (xa, ya): an 8x8 system.
static bool solve_h4(const double xb[4], const double yb[4],
                     const double xa[4], const double ya[4], Homography& H) {
    double M[8][9];  // augmented matrix [M | b]
    for (int i = 0; i < 4; i++) dlt_rows(xb[i], yb[i], xa[i], ya[i], M[2 * i], M[2 * i + 1]);
    double h[8];
    if (!solve8(M, h)) return false;
    for (int i = 0; i < 8; i++) H.h[i] = h[i];
    H.h[8] = 1.0;
    return true;
}

// Number of matches that H maps within RANSAC_THRESH pixels of their partner in A.
// If `idx` is given, it receives the indexes (into `matches`) of those inliers.
static int count_inliers(const Homography& H, const std::vector<Keypoint>& kpA,
                         const std::vector<Keypoint>& kpB, const std::vector<Match>& matches,
                         std::vector<int>* idx) {
    const double thresh2 = RANSAC_THRESH * RANSAC_THRESH;
    if (idx) idx->clear();
    int count = 0;
    for (size_t i = 0; i < matches.size(); i++) {
        const Match& m = matches[i];
        double px, py;
        if (!map_point(H, kpB[m.idxB].x, kpB[m.idxB].y, px, py)) continue;
        double dx = px - kpA[m.idxA].x, dy = py - kpA[m.idxA].y;
        if (dx * dx + dy * dy < thresh2) {
            count++;
            if (idx) idx->push_back((int)i);
        }
    }
    return count;
}

// Normalization (Hartley 1997) of a point set: move the centroid to (0, 0) and scale so
// the mean distance to it is sqrt(2). Gives (s, cx, cy): p' = s * (p - c).
static void normalization(const std::vector<double>& x, const std::vector<double>& y,
                          double& s, double& cx, double& cy) {
    const double n = (double)x.size();
    cx = cy = 0;
    for (size_t i = 0; i < x.size(); i++) { cx += x[i]; cy += y[i]; }
    cx /= n;
    cy /= n;
    double d = 0;
    for (size_t i = 0; i < x.size(); i++) d += std::sqrt((x[i] - cx) * (x[i] - cx) + (y[i] - cy) * (y[i] - cy));
    d /= n;
    s = d > 1e-12 ? std::sqrt(2.0) / d : 1.0;
}

// Least-squares H from all the inliers (n >= 4 point pairs).
// Every pair gives the same 2 rows as in solve_h4, so there are 2n equations for 8
// unknowns. The best h in the least-squares sense solves the normal equations
// (A^T A) h = A^T b, again an 8x8 system. The points are normalized first so this
// system stays well conditioned at 3840x2160, and H is mapped back afterwards:
//   H = Ta^-1 * Hn * Tb,  with Tb, Ta the normalizations of the B and A points.
static bool refit_h(const std::vector<Keypoint>& kpA, const std::vector<Keypoint>& kpB,
                    const std::vector<Match>& matches, const std::vector<int>& idx,
                    Homography& H) {
    const size_t n = idx.size();
    if (n < 4) return false;
    std::vector<double> xb(n), yb(n), xa(n), ya(n);
    for (size_t i = 0; i < n; i++) {
        const Match& m = matches[idx[i]];
        xb[i] = kpB[m.idxB].x;
        yb[i] = kpB[m.idxB].y;
        xa[i] = kpA[m.idxA].x;
        ya[i] = kpA[m.idxA].y;
    }
    double sb, cbx, cby, sa, cax, cay;
    normalization(xb, yb, sb, cbx, cby);
    normalization(xa, ya, sa, cax, cay);

    double N[8][9] = {};  // [A^T A | A^T b]
    for (size_t i = 0; i < n; i++) {
        double r[2][9];
        dlt_rows(sb * (xb[i] - cbx), sb * (yb[i] - cby), sa * (xa[i] - cax), sa * (ya[i] - cay),
                 r[0], r[1]);
        for (int k = 0; k < 2; k++)
            for (int a = 0; a < 8; a++)
                for (int c = 0; c < 9; c++) N[a][c] += r[k][a] * r[k][c];
    }
    double hn[8];
    if (!solve8(N, hn)) return false;

    // H = Ta^-1 * Hn * Tb, with Tb = [sb 0 -sb*cbx; 0 sb -sb*cby; 0 0 1],
    // Ta^-1 = [1/sa 0 cax; 0 1/sa cay; 0 0 1].
    const double Hn[9] = {hn[0], hn[1], hn[2], hn[3], hn[4], hn[5], hn[6], hn[7], 1.0};
    const double Tb[9] = {sb, 0, -sb * cbx, 0, sb, -sb * cby, 0, 0, 1};
    const double Tai[9] = {1 / sa, 0, cax, 0, 1 / sa, cay, 0, 0, 1};
    double T[9], R[9];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            T[i * 3 + j] = 0;
            for (int k = 0; k < 3; k++) T[i * 3 + j] += Hn[i * 3 + k] * Tb[k * 3 + j];
        }
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            R[i * 3 + j] = 0;
            for (int k = 0; k < 3; k++) R[i * 3 + j] += Tai[i * 3 + k] * T[k * 3 + j];
        }
    if (std::fabs(R[8]) < 1e-12) return false;
    for (int i = 0; i < 9; i++) H.h[i] = R[i] / R[8];
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
        int count = count_inliers(H, kpA, kpB, matches, nullptr);
        // Strictly greater: on a tie the earlier iteration wins (same rule on the GPU).
        if (count > best) {
            best = count;
            H_BtoA = H;
        }
    }
    if (best < 0) return false;
    inliers = best;

    // Refit: a 4-match sample fits only its own 4 points exactly, so its H carries their
    // pixel noise. A least-squares H over all of its inliers averages that noise out.
    // `inliers` is then counted again for the final H (it can differ by a few matches
    // near the 3 px threshold). This step runs once, on the host, in every version.
    std::vector<int> idx;
    count_inliers(H_BtoA, kpA, kpB, matches, &idx);
    Homography refit;
    if (refit_h(kpA, kpB, matches, idx, refit)) {
        H_BtoA = refit;
        inliers = count_inliers(refit, kpA, kpB, matches, nullptr);
    }
    return true;
}
