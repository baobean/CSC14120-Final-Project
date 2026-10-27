/*
 * image_io.cpp - load/save images and write debug images (wraps stb).
 * Authors: NDB
 * External sources: uses stb_image / stb_image_write (https://github.com/nothings/stb).
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 * 
 * From src/image_io.cpp with function save_gray removed and external library init
 */
#include "image_io.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool load_rgb(const char* path, ImageRGB& img) {
    int w, h, channels;
    unsigned char* p = stbi_load(path, &w, &h, &channels, 3);
    if (!p) {
        fprintf(stderr, "Cannot load %s: %s\n", path, stbi_failure_reason());
        return false;
    }
    img.w = w;
    img.h = h;
    img.data.assign(p, p + (size_t)w * h * 3);
    stbi_image_free(p);
    return true;
}

static bool ends_with(const std::string& s, const std::string& suffix) {
    if (s.size() < suffix.size()) return false;
    std::string tail = s.substr(s.size() - suffix.size());
    std::transform(tail.begin(), tail.end(), tail.begin(), ::tolower);
    return tail == suffix;
}

bool save_rgb(const std::string& path, const ImageRGB& img) {
    int ok;
    if (ends_with(path, ".jpg") || ends_with(path, ".jpeg"))
        ok = stbi_write_jpg(path.c_str(), img.w, img.h, 3, img.data.data(), 95);
    else
        ok = stbi_write_png(path.c_str(), img.w, img.h, 3, img.data.data(), img.w * 3);
    if (!ok) fprintf(stderr, "Cannot write %s\n", path.c_str());
    return ok != 0;
}

bool make_dir(const std::string& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) fprintf(stderr, "Cannot create directory %s: %s\n", dir.c_str(), ec.message().c_str());
    return !ec;
}
