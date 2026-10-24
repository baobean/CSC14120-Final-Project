/*
 * common.h - shared data types and fixed parameters for every version (V0..V4).
 * Authors: NDB (structs agreed at the Day 2 team meeting with LCK, DHDH)
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#pragma once

#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Fixed parameters (stage1_midterm_plan.md, section 2). Same in all versions.
// ---------------------------------------------------------------------------
// Stage 1: preprocessing
constexpr float  BLUR_SIGMA     = 1.0f;
constexpr int    BLUR_RADIUS    = 2;      // 5x5 kernel = 2*2+1 taps per pass
// Stage 2: Harris keypoints
constexpr float  HARRIS_K       = 0.04f;
constexpr int    HARRIS_WIN     = 5;      // window for summing Ix^2, Iy^2, IxIy
constexpr int    MAX_KEYPOINTS  = 2000;   // top K
// Stage 3: BRIEF
constexpr int    BRIEF_BITS     = 256;
constexpr int    BRIEF_PATCH    = 31;
constexpr unsigned BRIEF_SEED   = 42;
// Stage 4: matching
constexpr float  RATIO_TEST     = 0.8f;
// Stage 5: RANSAC
constexpr int    RANSAC_ITERS   = 1000;
constexpr double RANSAC_THRESH  = 3.0;    // pixels
constexpr unsigned RANSAC_SEED  = 42;

// ---------------------------------------------------------------------------
// Data types
// ---------------------------------------------------------------------------

// 8-bit RGB image, interleaved (R,G,B,R,G,B,...), row-major.
// Pixel (x, y) channel c is data[(y * w + x) * 3 + c].
struct ImageRGB {
    int w = 0, h = 0;
    std::vector<uint8_t> data;
};

// 1-channel float image, row-major. Pixel (x, y) is data[y * w + x].
// Gray values are kept in the range 0..255 (not normalized to 0..1).
struct ImageF {
    int w = 0, h = 0;
    std::vector<float> data;
};

// A detected corner. (x, y) in pixels of the original image.
struct Keypoint {
    float x, y;
    float score;      // Harris response R
};

// 256-bit BRIEF descriptor, stored as 8 x 32-bit words (bit i is in word i/32).
struct Descriptor {
    uint32_t bits[BRIEF_BITS / 32];
};

// One match between keypoint idxA in image A and idxB in image B.
struct Match {
    int idxA, idxB;
    int dist;         // Hamming distance
};

// 3x3 homography, row-major, h[8] = 1. Maps a point in B to A:
// [x_a, y_a, w]^T = H * [x_b, y_b, 1]^T, then divide by w.
struct Homography {
    double h[9];
};

// Pipeline stages, used as indices into StageTimes.
enum Stage {
    ST_PREPROCESS = 0,
    ST_DETECT,
    ST_DESCRIBE,
    ST_MATCH,
    ST_HOMOGRAPHY,
    ST_WARP,
    NUM_STAGES
};

static const char* const STAGE_NAMES[NUM_STAGES] = {
    "1.preprocess", "2.detect", "3.describe", "4.match", "5.homography", "6.warp+blend"
};

// Time of each stage in milliseconds for one pipeline run.
// Stages 1-3 run on both images, so their time is A + B.
struct StageTimes {
    double ms[NUM_STAGES] = {0};
};

struct PipelineOptions {
    const char* debug_dir = nullptr;   // nullptr = do not write debug images
    bool verbose = false;              // print keypoint / match / inlier counts
};

// Clamp v to [lo, hi]. Used for clamp-to-edge borders in all filters.
inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
