/*
 * image_io.h - load/save images and write debug images (wraps stb).
 * Authors: NDB
 * External sources: uses stb_image / stb_image_write (https://github.com/nothings/stb).
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#pragma once

#include <string>
#include "common.h"

// Loads any format stb supports as 8-bit RGB.
bool load_rgb(const char* path, ImageRGB& img);

// Writes JPEG (quality 95) if the path ends in .jpg/.jpeg, else PNG.
bool save_rgb(const std::string& path, const ImageRGB& img);

enum GrayMode {
    GRAY_CLAMP,     // values are 0..255: clamp and round (gray, blurred images)
    GRAY_ABS_MAX    // signed values: write |v| scaled so the max |v| maps to 255 (Ix, Iy)
};

// Writes a float image as an 8-bit gray PNG (for debugging only).
bool save_gray(const std::string& path, const ImageF& img, GrayMode mode);

// Creates the directory (and parents) if needed.
bool make_dir(const std::string& dir);
