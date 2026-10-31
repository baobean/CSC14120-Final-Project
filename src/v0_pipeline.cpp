/*
 * v0_pipeline.cpp - V0 sequential pipeline: runs stages 1..6 and times each one.
 * Authors: NDB
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * Single thread only: no OpenMP, no std::thread, no hand-written SIMD.
 *
 * TODO summary (Stage 1):
 *   Week 2 Day 9   NDB  write the keypoint debug images
 *   Week 2 Day 10  LCK  write the match debug image (tell NDB: shared file)
 *   Week 2 Day 11  NDB  integration: run the full V0 on small images, all sites
 *   Week 2 Day 12  NDB  check no OpenMP, no threads, no SIMD in every src/ file
 */
#include <chrono>
#include <cstdio>
#include <string>

#include "image_io.h"
#include "v0.h"
#include "versions.h"

using Clock = std::chrono::high_resolution_clock;

static double ms_since(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

static void write_debug_images(const std::string& dir, const char* tag,
                               const ImageF& gray, const ImageF& blurred) {
    save_gray(dir + "/gray_" + tag + ".png", gray, GRAY_CLAMP);
    save_gray(dir + "/blur_" + tag + ".png", blurred, GRAY_CLAMP);
    ImageF Ix, Iy;
    v0_sobel(blurred, Ix, Iy);
    save_gray(dir + "/Ix_" + tag + ".png", Ix, GRAY_ABS_MAX);
    save_gray(dir + "/Iy_" + tag + ".png", Iy, GRAY_ABS_MAX);
}

void run_v0(const ImageRGB& A, const ImageRGB& B, ImageRGB& out,
            StageTimes& t, const PipelineOptions& opt) {
    Clock::time_point t0;

    // Stage 1: preprocessing (A and B)
    ImageF grayA, grayB, blurA, blurB;
    t0 = Clock::now();
    v0_grayscale(A, grayA);
    v0_gaussian_blur(grayA, blurA);
    v0_grayscale(B, grayB);
    v0_gaussian_blur(grayB, blurB);
    t.ms[ST_PREPROCESS] = ms_since(t0);

    // Stage 2: keypoint detection (A and B)
    std::vector<Keypoint> kpA, kpB;
    t0 = Clock::now();
    v0_detect(blurA, kpA);
    v0_detect(blurB, kpB);
    t.ms[ST_DETECT] = ms_since(t0);

    // Stage 3: description (A and B)
    std::vector<Keypoint> keptA, keptB;
    std::vector<Descriptor> descA, descB;
    t0 = Clock::now();
    v0_describe(blurA, kpA, keptA, descA);
    v0_describe(blurB, kpB, keptB, descB);
    t.ms[ST_DESCRIBE] = ms_since(t0);

    // Stage 4: matching
    std::vector<Match> matches;
    t0 = Clock::now();
    v0_match(descA, descB, matches);
    t.ms[ST_MATCH] = ms_since(t0);

    // Stage 5: homography
    Homography H;
    int inliers = 0;
    t0 = Clock::now();
    bool found = v0_homography(keptA, keptB, matches, H, inliers);
    t.ms[ST_HOMOGRAPHY] = ms_since(t0);
    if (!found) {
        // Fall back to identity so the program still writes an image.
        H = Homography{{1, 0, 0, 0, 1, 0, 0, 0, 1}};
    }

    // Stage 6: warping + blending
    t0 = Clock::now();
    v0_warp_blend(A, B, H, out);
    t.ms[ST_WARP] = ms_since(t0);

    // Debug output is written after all timers stopped.
    if (opt.verbose) {
        printf("V0 counts: keypoints A=%zu B=%zu | described A=%zu B=%zu | matches=%zu | "
               "inliers=%d%s\n",
               kpA.size(), kpB.size(), keptA.size(), keptB.size(), matches.size(), inliers,
               found ? "" : " (no homography found, using identity)");
        // One line for tools/homography_error.py: the estimated H (B -> A), row-major.
        printf("H");
        for (int i = 0; i < 9; i++) printf(",%.10g", H.h[i]);
        printf("\n");
    }
    if (opt.debug_dir) {
        std::string dir = opt.debug_dir;
        if (make_dir(dir)) {
            write_debug_images(dir, "A", grayA, blurA);
            write_debug_images(dir, "B", grayB, blurB);
            // TODO(NDB, Week 2 Day 9): draw_keypoints(A, kpA, img) -> keypoints_A.png,
            //   same for B (debug_draw.h).
            // TODO(LCK, Week 2 Day 10): draw_matches(A, keptA, B, keptB, matches, img)
            //   -> matches.png.
            printf("Debug images written to %s/\n", dir.c_str());
        }
    }
}
