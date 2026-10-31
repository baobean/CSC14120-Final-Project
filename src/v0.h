/*
 * v0.h - per-stage functions of the sequential version V0.
 *        Each owner implements their stages in their own v0_*.cpp file.
 * Authors: NDB (interfaces agreed with LCK, DHDH)
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#pragma once

#include "common.h"

// ---- Stage 1: preprocessing (NDB, v0_preprocess.cpp) ----
// gray = 0.299 R + 0.587 G + 0.114 B, values 0..255
void v0_grayscale(const ImageRGB& in, ImageF& gray);
// Separable 5x5 Gaussian (sigma = BLUR_SIGMA): row pass, then column pass.
// Pixels outside the image are clamped to the nearest edge pixel.
void v0_gaussian_blur(const ImageF& in, ImageF& out);

// ---- Stage 2: keypoint detection (NDB, v0_detect.cpp) ----
// 3x3 Sobel gradients, clamped border.
void v0_sobel(const ImageF& in, ImageF& Ix, ImageF& Iy);
// Harris response R = det(M) - HARRIS_K * trace(M)^2, where M sums Ix^2, Iy^2, IxIy
// over a HARRIS_WIN x HARRIS_WIN window (clamped border).
void v0_harris(const ImageF& Ix, const ImageF& Iy, ImageF& R);
// Sobel + Harris + NMS 3x3 + top K. Output sorted by score (highest first),
// equal scores by y, then x.
void v0_detect(const ImageF& blurred, std::vector<Keypoint>& kps);

// ---- Stage 3: description (LCK, v0_describe.cpp) ----
// Computes BRIEF for each keypoint. Keypoints too close to the border are
// dropped, so `kept` holds the keypoints that have a descriptor (same order).
void v0_describe(const ImageF& blurred, const std::vector<Keypoint>& kps,
                 std::vector<Keypoint>& kept, std::vector<Descriptor>& desc);

// ---- Stage 4: matching (LCK, v0_match.cpp) ----
// Brute-force Hamming + Lowe ratio test. idxA / idxB index into descA / descB.
void v0_match(const std::vector<Descriptor>& descA, const std::vector<Descriptor>& descB,
              std::vector<Match>& matches);

// ---- Stage 5: homography (DHDH, v0_homography.cpp) ----
// RANSAC + 4-point DLT. Returns false if no valid H was found.
bool v0_homography(const std::vector<Keypoint>& kpA, const std::vector<Keypoint>& kpB,
                   const std::vector<Match>& matches, Homography& H_BtoA, int& inliers);

// ---- Stage 6: warping + blending (DHDH, v0_warp.cpp) ----
// Inverse warp of B into A's frame (bilinear) + linear blend in the overlap.
void v0_warp_blend(const ImageRGB& A, const ImageRGB& B, const Homography& H_BtoA,
                   ImageRGB& out);
