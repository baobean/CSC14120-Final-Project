/*
 * debug_draw.cpp - debug images for keypoints and matches (not timed, debug only).
 * Authors: NDB (draw_keypoints), LCK (draw_matches)
 * External sources: 
 *  - Random color generator: https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
 *  - HSV to RGB: https://en.wikipedia.org/wiki/HSL_and_HSV#Color_conversion_formulae
 *  - DDA (line drawing): https://www.geeksforgeeks.org/computer-graphics/dda-line-generation-algorithm-computer-graphics/
 * 
 * AI assistance (draw_keypoints): drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 */

#include <algorithm>
#include <cstdlib>
#include "debug_draw.h"

#include <cmath>
struct RGB{
    uint8_t R,G,B;
};

void draw_keypoints(const ImageRGB& img, const std::vector<Keypoint>& kps, ImageRGB& out) {
    out = img;
    // 5x5 hollow red square centered on each keypoint; pixels outside the image are skipped.
    for (const Keypoint& kp : kps) {
        int cx = (int)std::lround(kp.x), cy = (int)std::lround(kp.y);
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx != -2 && dx != 2 && dy != -2 && dy != 2) continue;  // inside: hollow
                int x = cx + dx, y = cy + dy;
                if (x < 0 || x >= out.w || y < 0 || y >= out.h) continue;
                uint8_t* p = &out.data[((size_t)y * out.w + x) * 3];
                p[0] = 255;
                p[1] = 0;
                p[2] = 0;
            }
        }
    }
}

// Ref: 
// - Generate random colors: https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
// - HSV to RGB: https://en.wikipedia.org/wiki/HSL_and_HSV#Color_conversion_formulae
void generateRandomColor(int n, float value, float saturation, std::vector<RGB>& out){
    // Limit input value
    value = value > 1? 1 : (value < 0? 0: value);
    saturation = saturation > 1? 1 : (saturation < 0? 0: saturation);

    out.clear();
    out.resize(n);
    const double golden_ratio = 0.618033988749895;
    srand(42); // temp seed
    float hue = (rand() % 360) / 360.0f + golden_ratio; 
    hue = fmod(hue, 1.0);
    for (int i = 0; i < n; i++){
        int h1 = hue * 6;
        hue = fmod(hue + golden_ratio, 1.0);
        float c = value * saturation,
            x = c *(1 - abs(h1 % 2 - 1));
        float r, g, b;
        switch (h1){
            case 0: 
            r = c;
            g = x;
            b = 0;
            break;
            case 1:
            r = x;
            g = c;
            b = 0;
            break;
            case 2:
            r = 0;
            g = c;
            b = x;
            break;
            case 3:
            r = 0;
            g = x;
            b = c;
            break;
            case 4:
            r = x;
            g = 0;
            b = c;
            break;
            case 5:
            r = c;
            g = 0;
            b = x;
            break;
            default:;
        }
        const float m = value - c;
        out[i] = {
            (uint8_t)clampi((r + m) * 255, 0, 255), 
            (uint8_t)clampi((g + m) * 255, 0, 255), 
            (uint8_t)clampi((b + m) * 255, 0, 255)
        };
    }
}
// Ref: https://www.geeksforgeeks.org/computer-graphics/dda-line-generation-algorithm-computer-graphics/
void draw_matches(const ImageRGB& A, const std::vector<Keypoint>& kpA,
                  const ImageRGB& B, const std::vector<Keypoint>& kpB,
                  const std::vector<Match>& matches, ImageRGB& out) 
{
    out.w = A.w + B.w;
    out.h = std::max(A.h, B.h);
    out.data.resize(out.w * out.h * 3, 0);
    // Setting image A, B in output
    for (int row = 0; row < A.h; row++){
        for (int col = 0; col < A.w; col++){
            out.data[(row * out.w + col) * 3] = A.data[(row * A.w + col) * 3];
            out.data[(row * out.w + col) * 3 + 1] = A.data[(row * A.w + col) * 3 + 1];
            out.data[(row * out.w + col) * 3 + 2] = A.data[(row * A.w + col) * 3 + 2];
        }
    }

    for (int row = 0; row < B.h; row++){
        for (int col = 0; col < B.w; col++){
            out.data[(row * out.w + A.w + col) * 3] = B.data[(row * B.w + col) * 3];
            out.data[(row * out.w + A.w + col) * 3 + 1] = B.data[(row * B.w + col) * 3 + 1];
            out.data[(row * out.w + A.w + col) * 3 + 2] = B.data[(row * B.w + col) * 3 + 2];
        }
    }
    
    // Drawing lines
    Keypoint a,b; // Treated as regular points
    float dx, dy, y, x;
    std::vector<RGB> colors;
    generateRandomColor(matches.size(), 0.9, 0.7, colors);
    for (int v = 0; v < matches.size(); v++){
        a = kpA[matches[v].idxA];
        b = kpB[matches[v].idxB];
        dx = (b.x + A.w) - a.x; // A.w is added as we will have two image side by side
        dy = b.y - a.y;
        y = a.y;
        x = a.x;
        int steps = std::max(fabs(dx), fabs(dy));
        for (int i = 0; i < steps; i++){
            int index = (((int)y) * out.w + (int)(x)) * 3;
            out.data[index] = colors[v].R;
            out.data[index + 1] = colors[v].G;
            out.data[index + 2] = colors[v].B;
            y = y + dy/steps;
            x = x + dx/steps;
        }
    }
}
