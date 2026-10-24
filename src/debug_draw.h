/*
 * debug_draw.h - debug images for keypoints and matches (not timed, debug only).
 * Authors: NDB (draw_keypoints), LCK (draw_matches)
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#pragma once

#include "common.h"

// Copy of img with a small colored square at each keypoint.
void draw_keypoints(const ImageRGB& img, const std::vector<Keypoint>& kps, ImageRGB& out);

// A and B side by side (B to the right of A), with one line per match from
// kpA[m.idxA] to kpB[m.idxB] (shifted by A's width).
void draw_matches(const ImageRGB& A, const std::vector<Keypoint>& kpA,
                  const ImageRGB& B, const std::vector<Keypoint>& kpB,
                  const std::vector<Match>& matches, ImageRGB& out);
