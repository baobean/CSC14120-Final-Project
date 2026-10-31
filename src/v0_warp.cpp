/*
 * v0_warp.cpp - V0 stage 6: inverse warp (bilinear) + linear blend.
 * Authors: NDB (written for DHDH's stage; reviewed by DHDH)
 * External sources: inverse warping and bilinear sampling as in R. Szeliski,
 *                   "Computer Vision: Algorithms and Applications", 2nd ed., ch. 3.6 and 8.
 *                   Implemented from the formulas, no code copied.
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * TODO summary (Stage 1):
 *   Week 2 Day 14  DHDH  one panorama per site + the challenging pair (notebook)
 */
#include <algorithm>
#include <cmath>
#include <cstdio>

#include "v0.h"

// Maps (x, y) with H and divides by w. Returns false if w is ~0 (point at infinity).
static bool map_point(const double h[9], double x, double y, double& ox, double& oy) {
    double w = h[6] * x + h[7] * y + h[8];
    if (std::fabs(w) < 1e-12) return false;
    ox = (h[0] * x + h[1] * y + h[2]) / w;
    oy = (h[3] * x + h[4] * y + h[5]) / w;
    return true;
}

// Inverse of a 3x3 matrix: adjugate / determinant. Returns false if det is ~0.
static bool invert3x3(const double m[9], double inv[9]) {
    double c0 = m[4] * m[8] - m[5] * m[7];
    double c1 = m[5] * m[6] - m[3] * m[8];
    double c2 = m[3] * m[7] - m[4] * m[6];
    double det = m[0] * c0 + m[1] * c1 + m[2] * c2;
    if (std::fabs(det) < 1e-12) return false;
    double d = 1.0 / det;
    inv[0] = c0 * d;
    inv[1] = (m[2] * m[7] - m[1] * m[8]) * d;
    inv[2] = (m[1] * m[5] - m[2] * m[4]) * d;
    inv[3] = c1 * d;
    inv[4] = (m[0] * m[8] - m[2] * m[6]) * d;
    inv[5] = (m[2] * m[3] - m[0] * m[5]) * d;
    inv[6] = c2 * d;
    inv[7] = (m[1] * m[6] - m[0] * m[7]) * d;
    inv[8] = (m[0] * m[4] - m[1] * m[3]) * d;
    return true;
}

static uint8_t to_u8(float v) {
    return (uint8_t)std::min(255.0f, std::max(0.0f, v + 0.5f));
}

void v0_warp_blend(const ImageRGB& A, const ImageRGB& B, const Homography& H_BtoA,
                   ImageRGB& out) {
    const double* H = H_BtoA.h;

    // Canvas: bounding box of A's rectangle and the 4 corners of B mapped into A's frame.
    const double cx[4] = {0, (double)B.w - 1, 0, (double)B.w - 1};
    const double cy[4] = {0, 0, (double)B.h - 1, (double)B.h - 1};
    double bminx = 1e30, bmaxx = -1e30, bminy = 1e30, bmaxy = -1e30;
    bool ok = true;
    for (int i = 0; i < 4 && ok; i++) {
        double px, py;
        ok = map_point(H, cx[i], cy[i], px, py);
        bminx = std::min(bminx, px);
        bmaxx = std::max(bmaxx, px);
        bminy = std::min(bminy, py);
        bmaxy = std::max(bmaxy, py);
    }
    double Hinv[9];
    ok = ok && invert3x3(H, Hinv);
    int x0 = 0, y0 = 0, x1 = A.w - 1, y1 = A.h - 1;
    if (ok) {
        x0 = (int)std::floor(std::min(0.0, bminx));
        y0 = (int)std::floor(std::min(0.0, bminy));
        x1 = (int)std::ceil(std::max((double)A.w - 1, bmaxx));
        y1 = (int)std::ceil(std::max((double)A.h - 1, bmaxy));
    }
    // A bad H can make a huge canvas: keep A alone instead.
    if (!ok || x1 - x0 + 1 > 4 * A.w || y1 - y0 + 1 > 4 * A.h) {
        fprintf(stderr, "Warning: homography gives an invalid or too large canvas, "
                        "output is image A only\n");
        out = A;
        return;
    }
    const int W = x1 - x0 + 1, Hc = y1 - y0 + 1;
    const int tx = -x0, ty = -y0;  // A's pixel (0, 0) sits at (tx, ty) in the canvas

    // Overlap in x (canvas coordinates): A's columns intersected with the warped B's box.
    // alpha goes from 1 at the overlap's left edge to 0 at its right edge, so the seam
    // fades from A into B. A is the left image (CLI: <left> <right>).
    const double ovl_left = std::max((double)tx, bminx + tx);
    const double ovl_right = std::min((double)(tx + A.w - 1), bmaxx + tx);
    const double ovl_width = ovl_right - ovl_left;

    out.w = W;
    out.h = Hc;
    out.data.assign((size_t)W * Hc * 3, 0);  // black where neither image is

    for (int y = 0; y < Hc; y++) {
        for (int x = 0; x < W; x++) {
            uint8_t* o = &out.data[((size_t)y * W + x) * 3];
            const int ax = x - tx, ay = y - ty;  // this pixel in A's frame
            const bool inA = ax >= 0 && ax < A.w && ay >= 0 && ay < A.h;

            // Inverse warp: which point of B lands here?
            double bx, by;
            bool inB = map_point(Hinv, ax, ay, bx, by) &&
                       bx >= 0 && bx <= B.w - 1 && by >= 0 && by <= B.h - 1;
            float bpix[3] = {0, 0, 0};
            if (inB) {
                // Bilinear: weighted average of the 4 neighbors, weights from the fractions.
                int bx0 = (int)bx, by0 = (int)by;
                int bx1 = std::min(bx0 + 1, B.w - 1), by1 = std::min(by0 + 1, B.h - 1);
                float fx = (float)(bx - bx0), fy = (float)(by - by0);
                const uint8_t* p00 = &B.data[((size_t)by0 * B.w + bx0) * 3];
                const uint8_t* p01 = &B.data[((size_t)by0 * B.w + bx1) * 3];
                const uint8_t* p10 = &B.data[((size_t)by1 * B.w + bx0) * 3];
                const uint8_t* p11 = &B.data[((size_t)by1 * B.w + bx1) * 3];
                for (int c = 0; c < 3; c++)
                    bpix[c] = (1 - fy) * ((1 - fx) * p00[c] + fx * p01[c]) +
                              fy * ((1 - fx) * p10[c] + fx * p11[c]);
            }

            if (inA && inB) {
                float alpha = ovl_width > 0 ? (float)((ovl_right - x) / ovl_width) : 1.0f;
                alpha = std::min(1.0f, std::max(0.0f, alpha));
                const uint8_t* a = &A.data[((size_t)ay * A.w + ax) * 3];
                for (int c = 0; c < 3; c++) o[c] = to_u8(alpha * a[c] + (1 - alpha) * bpix[c]);
            } else if (inA) {
                const uint8_t* a = &A.data[((size_t)ay * A.w + ax) * 3];
                o[0] = a[0];
                o[1] = a[1];
                o[2] = a[2];
            } else if (inB) {
                for (int c = 0; c < 3; c++) o[c] = to_u8(bpix[c]);
            }
        }
    }
}
