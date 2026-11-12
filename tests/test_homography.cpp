/*
 * test_homography.cpp - unit tests for stages 5-6 (v0_homography, v0_warp_blend)
 * on made-up points, so they run without images and without a GPU.
 * Authors: Đỗ Bảo
 * External sources: none
 *
 * Run: make test
 */
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "v0.h"

static int failures = 0;

static void check(bool ok, const char* name, const char* fmt = "", double v = 0) {
    printf("[%s] %s", ok ? "PASS" : "FAIL", name);
    if (fmt[0]) {
        printf("  (");
        printf(fmt, v);
        printf(")");
    }
    printf("\n");
    if (!ok) failures++;
}

static void map_pt(const Homography& H, double x, double y, double& ox, double& oy) {
    const double* h = H.h;
    double w = h[6] * x + h[7] * y + h[8];
    ox = (h[0] * x + h[1] * y + h[2]) / w;
    oy = (h[3] * x + h[4] * y + h[5]) / w;
}

// Mean distance between B's 4 corners mapped by H and by H_true (same as
// tools/homography_error.py).
static double corner_error(const Homography& H, const Homography& H_true, int w, int h) {
    const double cx[4] = {0, (double)w - 1, (double)w - 1, 0};
    const double cy[4] = {0, 0, (double)h - 1, (double)h - 1};
    double sum = 0;
    for (int i = 0; i < 4; i++) {
        double ax, ay, bx, by;
        map_pt(H, cx[i], cy[i], ax, ay);
        map_pt(H_true, cx[i], cy[i], bx, by);
        sum += std::hypot(ax - bx, ay - by);
    }
    return sum / 4;
}

// A homography like the synthetic pair's: shift by 35% of the width, 2 degree
// rotation, a small perspective term (h[6]).
static Homography make_h_true(int w) {
    const double a = 2.0 * M_PI / 180.0;
    Homography H = {{std::cos(a), -std::sin(a), 0.35 * w,
                     std::sin(a), std::cos(a), 10.0,
                     1e-5 * 1280.0 / w, 0, 1}};
    return H;
}

// n matches: B points spread over the image, A = H_true(B) + Gaussian noise
// (sigma px). A fraction `outliers` of the A points is replaced by random points.
static void make_matches(const Homography& H_true, int w, int h, int n, double sigma,
                         double outliers, unsigned seed, std::vector<Keypoint>& kpA,
                         std::vector<Keypoint>& kpB, std::vector<Match>& matches,
                         int& true_inliers) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> ux(0, w - 1), uy(0, h - 1), u01(0, 1);
    std::normal_distribution<double> noise(0, sigma);
    kpA.clear();
    kpB.clear();
    matches.clear();
    true_inliers = 0;
    for (int i = 0; i < n; i++) {
        double xb = ux(rng), yb = uy(rng), xa, ya;
        map_pt(H_true, xb, yb, xa, ya);
        if (u01(rng) < outliers) {
            xa = ux(rng) + 0.35 * w;
            ya = uy(rng);
        } else {
            xa += noise(rng);
            ya += noise(rng);
            true_inliers++;
        }
        kpB.push_back({(float)xb, (float)yb, 1.0f});
        kpA.push_back({(float)xa, (float)ya, 1.0f});
        matches.push_back({i, i, 0});
    }
}

int main() {
    // 1-2. Noisy inliers + 40% outliers, at 1280x720 and 3840x2160.
    //      The refit should bring the error well below 1 px.
    const int sizes[2][2] = {{1280, 720}, {3840, 2160}};
    for (auto& sz : sizes) {
        int w = sz[0], h = sz[1];
        Homography H_true = make_h_true(w);
        std::vector<Keypoint> kpA, kpB;
        std::vector<Match> matches;
        int true_inl;
        make_matches(H_true, w, h, 1000, 0.7, 0.4, 7, kpA, kpB, matches, true_inl);
        Homography H;
        int inl = 0;
        bool ok = v0_homography(kpA, kpB, matches, H, inl);
        double err = corner_error(H, H_true, w, h);
        char name[128];
        snprintf(name, sizeof name, "%dx%d, noise 0.7 px, 40%% outliers: corner error < 0.5 px", w, h);
        check(ok && err < 0.5, name, "error %.3f px", err);
        snprintf(name, sizeof name, "%dx%d: inliers close to the true %d (within 3%%)", w, h, true_inl);
        check(std::abs(inl - true_inl) <= 0.03 * true_inl, name, "found %.0f", inl);
    }

    // 3. Same input twice -> exactly the same H (fixed sample list, no randomness).
    {
        Homography H_true = make_h_true(1280);
        std::vector<Keypoint> kpA, kpB;
        std::vector<Match> matches;
        int true_inl, i1, i2;
        make_matches(H_true, 1280, 720, 500, 0.7, 0.3, 3, kpA, kpB, matches, true_inl);
        Homography H1, H2;
        v0_homography(kpA, kpB, matches, H1, i1);
        v0_homography(kpA, kpB, matches, H2, i2);
        bool same = i1 == i2;
        for (int k = 0; k < 9; k++) same = same && H1.h[k] == H2.h[k];
        check(same, "deterministic: two runs give the same H and inlier count");
    }

    // 4. Exact points (no noise, no outliers) -> H_true almost exactly, h[8] = 1.
    {
        Homography H_true = make_h_true(1280);
        std::vector<Keypoint> kpA, kpB;
        std::vector<Match> matches;
        int true_inl, inl;
        make_matches(H_true, 1280, 720, 200, 0.0, 0.0, 5, kpA, kpB, matches, true_inl);
        Homography H;
        v0_homography(kpA, kpB, matches, H, inl);
        double err = corner_error(H, H_true, 1280, 720);
        // Keypoints are float, so the points carry ~1e-4 px rounding.
        check(err < 0.01 && H.h[8] == 1.0 && inl == 200, "exact points: error < 0.01 px, all 200 inliers, h33 = 1",
              "error %.2e px", err);
    }

    // 5. Fewer than 4 matches -> no H, identity returned.
    {
        std::vector<Keypoint> kpA = {{1, 1, 1}, {5, 1, 1}, {1, 5, 1}};
        std::vector<Keypoint> kpB = kpA;
        std::vector<Match> matches = {{0, 0, 0}, {1, 1, 0}, {2, 2, 0}};
        Homography H;
        int inl = -1;
        bool ok = v0_homography(kpA, kpB, matches, H, inl);
        bool identity = H.h[0] == 1 && H.h[4] == 1 && H.h[8] == 1 && H.h[2] == 0;
        check(!ok && inl == 0 && identity, "3 matches: returns false, identity H, 0 inliers");
    }

    // 6. All points on one line (degenerate) -> must not crash or return NaN.
    {
        std::vector<Keypoint> kpA, kpB;
        std::vector<Match> matches;
        for (int i = 0; i < 50; i++) {
            kpB.push_back({(float)(10 * i), (float)(5 * i), 1});
            kpA.push_back({(float)(10 * i + 400), (float)(5 * i), 1});
            matches.push_back({i, i, 0});
        }
        Homography H;
        int inl;
        v0_homography(kpA, kpB, matches, H, inl);
        bool finite = true;
        for (double v : H.h) finite = finite && std::isfinite(v);
        check(finite, "collinear points: no NaN / inf in H");
    }

    // 7. Warp + blend with the identity H: B lands exactly on A, so out == A.
    {
        ImageRGB A;
        A.w = 64;
        A.h = 48;
        A.data.resize((size_t)A.w * A.h * 3);
        for (size_t i = 0; i < A.data.size(); i++) A.data[i] = (uint8_t)((i * 37) % 251);
        ImageRGB out;
        Homography I = {{1, 0, 0, 0, 1, 0, 0, 0, 1}};
        v0_warp_blend(A, A, I, out);
        bool same = out.w == A.w && out.h == A.h && out.data == A.data;
        check(same, "warp with identity H and B = A gives exactly A");
    }

    // 8. Warp with a pure shift of 40 px: canvas is 40 px wider than A.
    {
        ImageRGB A;
        A.w = 100;
        A.h = 50;
        A.data.assign((size_t)A.w * A.h * 3, 200);
        ImageRGB out;
        Homography T = {{1, 0, 40, 0, 1, 0, 0, 0, 1}};
        v0_warp_blend(A, A, T, out);
        check(out.w == 140 && out.h == 50, "shift of 40 px: canvas 140x50", "width %.0f", out.w);
    }

    printf("\n%s (%d failed)\n", failures == 0 ? "ALL PASSED" : "SOME TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
