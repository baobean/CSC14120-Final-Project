/*
 * image_io.h - load/save images and write debug images (wraps stb).
 * Authors: NDB
 * External sources: uses stb_image / stb_image_write (https://github.com/nothings/stb).
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 * 
 * From src/image_io.h with function save_gray removed and struct ImageRGB added
 */
#pragma once

#include <string>
#include <vector>

// 8-bit RGB image, interleaved (R,G,B,R,G,B,...), row-major.
// Pixel (x, y) channel c is data[(y * w + x) * 3 + c].
struct ImageRGB {
    int w = 0, h = 0;
    std::vector<uint8_t> data;
};

// Loads any format stb supports as 8-bit RGB.
bool load_rgb(const char* path, ImageRGB& img);

// Writes JPEG (quality 95) if the path ends in .jpg/.jpeg, else PNG.
bool save_rgb(const std::string& path, const ImageRGB& img);

// Creates the directory (and parents) if needed.
bool make_dir(const std::string& dir);
