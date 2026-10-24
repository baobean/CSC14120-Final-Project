/*
 * image_io.cpp - load/save images and write debug images (wraps stb).
 * Authors: NDB
 * External sources: uses stb_image / stb_image_write (https://github.com/nothings/stb).
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#include "image_io.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>

#include "stb_image.h"
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

bool save_gray(const std::string& path, const ImageF& img, GrayMode mode) {
    size_t n = (size_t)img.w * img.h;
    float scale = 1.0f;
    if (mode == GRAY_ABS_MAX) {
        float maxv = 0.0f;
        for (size_t i = 0; i < n; i++) maxv = std::max(maxv, std::fabs(img.data[i]));
        scale = maxv > 0.0f ? 255.0f / maxv : 0.0f;
    }
    std::vector<uint8_t> out(n);
    for (size_t i = 0; i < n; i++) {
        float v = (mode == GRAY_ABS_MAX) ? std::fabs(img.data[i]) * scale : img.data[i];
        out[i] = (uint8_t)std::min(255.0f, std::max(0.0f, v + 0.5f));
    }
    int ok = stbi_write_png(path.c_str(), img.w, img.h, 1, out.data(), img.w);
    if (!ok) fprintf(stderr, "Cannot write %s\n", path.c_str());
    return ok != 0;
}

bool make_dir(const std::string& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) fprintf(stderr, "Cannot create directory %s: %s\n", dir.c_str(), ec.message().c_str());
    return !ec;
}
