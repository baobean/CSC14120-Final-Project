/*
 * main - script program to change the size of an image
 * Authors: LCK
 * External sources: https://paulinternet.nl/bicubic (Bicubic calculation)
 * AI assistance: bug fixing on some wrong assumption with ChatGPT
 * 
 */
#include <vector>
#include <iostream>
#include <algorithm>

#include "image_io.h"



// From src/common.h
// Clamp v to [lo, hi]. Used for clamp-to-edge borders in all filters.
inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void resize_nearest(const ImageRGB& in, ImageRGB& out, int newW, int newH){
    // Clearing and setting new values
    out.data.clear();
    out.data.resize((size_t)newW * newH * 3);
    out.w = newW;
    out.h = newH;

    // Calculating scale for each axis
    float scaleW = (float)in.w/newW, scaleH = (float)in.h/newH;
    const uint8_t stride = 3; // Each pixel contain 3 element (R,G,B) consecutively

    for (int x = 0, inX = 0, inY=0; x < newW; x++){
        for (int y = 0; y < newH; y++){
            // Calculate corresponding pixel in input image for each element
            inX = scaleW * (x + 0.5f);
            inY = scaleH * (y + 0.5f) ;
            inX = clampi(inX, 0, in.w - 1);
            inY = clampi(inY, 0, in.h - 1);

            const int outIndex = y * newW + x, inIndex = inY * in.w + inX; 
            // Writing out the nearest value to each color channel
            out.data[outIndex * stride] = in.data[inIndex * stride]; 
            out.data[outIndex * stride + 1] = in.data[inIndex * stride + 1]; 
            out.data[outIndex * stride + 2] = in.data[inIndex * stride + 2]; 
        }
    }
}

void resize_bilinear(const ImageRGB& in, ImageRGB& out, int newW, int newH){
    // Clearing and setting new values
    out.data.clear();
    out.data.resize((size_t)newW * newH * 3);
    out.w = newW;
    out.h = newH;

    float scaleW = (float)in.w/newW, scaleH = (float)in.h/newH;
    auto interpolate = [](uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3, float dx, float dy) -> float {
        return (1 - dy) * ((1 - dx) * p0 + dx * p1) + dy * ((1-dx) * p2 + dx * p3);
    };
    const uint8_t stride = 3;
    for (int x = 0,inX1 = 0, inY1=0, inX2 = 0, inY2 = 0; x < newW; x++){
        float inXf = 0, inYf = 0, dx, dy;
        for (int y = 0; y < newH; y++){
            inXf = scaleW * (x + 0.5f) - 0.5f;
            inYf = scaleH * (y + 0.5f) - 0.5f;
            // Floor inXf and inYf is taken as we need the point in negative value that the algorithm required
            inX1 = floorf(inXf);
            inY1 = floorf(inYf);
            inX2 = inX1 + 1;
            inY2 = inY1 + 1;
            dx = inXf - inX1;
            dy = inYf - inY1;

            // Note: inX1, inY1 can be negative, inX2, inY2 can be larger than input range.
            // So we limit value to input range. 
            // This is done after dx, dy is calculated so that dx, dy is calculated properly
            inX1 = clampi(inX1, 0, in.w - 1);
            inY1 = clampi(inY1, 0, in.h - 1);
            inX2 = clampi(inX2, 0, in.w - 1);
            inY2 = clampi(inY2, 0, in.h - 1);

            const int outIndex = y * newW + x,
                      p00 = (inY1 * in.w + inX1) * 3,
                      p01 = (inY1 * in.w + inX2) * 3,
                      p10 = (inY2 * in.w + inX1) * 3,
                      p11 = (inY2 * in.w + inX2) * 3;
            for (int c = 0; c < 3; c++)
                out.data[outIndex * stride + c] = interpolate(
                    in.data[p00 + c], 
                    in.data[p01 + c], 
                    in.data[p10 + c], 
                    in.data[p11 + c], 
                    dx, 
                    dy
                ) + 0.5f;

            // out.data[outIndex * 3] = (1 - dy) * ( (1 - dx) * in.data[p00] + dx * in.data[p01]) + 
            //                     dy * ((1-dx) * in.data[p10] + dx * in.data[p11]); 
            // out.data[outIndex * 3 + 1] = (1 - dy) * ((1 - dx) * in.data[p00 + 1] + dx * in.data[p01 + 1]) + 
            //                     dy * ((1-dx) * in.data[p10 + 1] + dx * in.data[p11 + 1]); 
            // out.data[outIndex * 3 + 2] = (1 - dy) * ( (1 - dx) * in.data[p00 + 2] + dx * in.data[p01 + 2]) + 
            //                     dy * ((1-dx) * in.data[p10 + 2] + dx * in.data[p11 + 2]); 
        }
    }
}

// Calculate the bicubic interpolation of point x (from 0-1 inclusive)
// If p0 is not present then it can be subtitude by setting lr to 1
// If p3 is not present then it can be subtidue by setting lr to 2
// The subtitude value is calculated by setting cubic term (x^3) to 0
// First view from: https://youtu.be/poY_nGzEEWM?si=6Ot0VZZfTm5Kzwmn
// Ref: https://paulinternet.nl/bicubic
// Bug fix by ChatGPT as the first assumption is that input points are ONLY parts of the input image (and thus have a range of 0-255)
static float img_bicubic(float x, float p0, float p1, float p2, float p3, float lr = 0){
    if (lr == 1) // The left most element (p0) is not present
        p0 = std::min((p1 - p2) * 3 + p3, 255.0f);
    
    if (lr == 2) // The right most element (p3) is not present
        p3 = std::min(p0 + (p2 - p1) * 3, 255.0f);
    
    const double a = (-p0 + 3 * p1 - 3 * p2 + p3) / 2.0,
              b = p0 - (5 * p1 + p3) / 2.0 + 2 * p2,
              c = (p2 - p0) /2.0,
              d = p1;
    return (((a * x) + b)*x + c) *x + d;
}

// Bug fixing some part of the image in 
void resize_bicubic(const ImageRGB& in, ImageRGB& out, int newW, int newH){
    // Clearing and setting new values
    out.data.clear();
    out.data.resize((size_t)newW * newH * 3);
    out.w = newW;
    out.h = newH;

    float scaleW = (float)in.w/newW, scaleH = (float)in.h/newH;

    const uint8_t n_line = 4, stride = 3, color_channel = 3;
    
    float line[4], inX, inY;

    auto sample = [&in,&stride](int col, int row, uint8_t channel){
        return in.data[(clampi(col, 0, in.w - 1) + row * in.w) * stride + channel];
    };
    for (int y = 0, positionsCol[4] = {0}, positionsRow[4] = {0}; y < newH; y++){
        for (int x = 0; x < newW; x++){
            inX = (0.5f + x) * scaleW - 0.5f, inY = (0.5f + y) * scaleH - 0.5f;
            // Listing out the affected columns and rows
            for (int8_t i = 0; i < 4; i++){
                int xCol = floor(inX) + i - 1, yRow = floor(inY) + i - 1;
                positionsCol[i] = xCol;
                positionsRow[i] = yRow;
            }

            // Calculate the output for each channel
            for (int8_t c = 0; c < color_channel; c++){
                // Calculate the output on each line of 4 pixels
                // If there isn't a valid value for first column then it is calculated by assuming the x^3 term to 0 (see img_bicubic)
                // If the second parameter isn't valid then it is populated with the nearest pixel 
                // If the third parameter isn't valid then it get the same treatment as second parameter
                // If the third parameter isn't valid then it get the same treatment as first parameter
                for (int8_t il = 0; il < n_line; il++){
                    const int row_index = clampi(positionsRow[il], 0, in.h - 1);
                    line[il] =  img_bicubic(
                            inX - (int)floorf(inX), 
                            sample(positionsCol[0], row_index, c),// in.data[(clampi(positionsCol[0], 0, in.w - 1) + row_index * in.w) * 3 + c], 
                            sample(positionsCol[1], row_index, c),// in.data[(clampi(positionsCol[1], 0, in.w - 1) + row_index * in.w) * 3 + c], 
                            sample(positionsCol[2], row_index, c),// in.data[(clampi(positionsCol[2], 0, in.w - 1) + row_index * in.w) * 3 + c], 
                            sample(positionsCol[3], row_index, c),// in.data[(clampi(positionsCol[3], 0, in.w - 1) + row_index * in.w) * 3 + c],
                        positionsCol[0] < 0 ? 1 :(positionsCol[3] >= in.w ? 2 : 0));
                }

                // Interpolate between computed 4 line
                // If the first or the last line is not valid then it it is calculated by assuming the x^3 term to 0 (see img_bicubic)
                // The other two lines should always exist
                out.data[(y * newW + x) * 3 + c] = clampi(
                    0.5f + img_bicubic(
                        inY - (int)floorf(inY), 
                        line[0], line[1], line[2], line[3], 
                        positionsRow[0] < 0 ? 1 : (positionsRow[3] >= in.h ? 2 : 0)
                    ) , 
                    0, 
                    255
                );
            }
        }
    }
}

// First arg is picture
// Second is width
// Third is height
// Fourth is type: near, bilinear, bicubic
// Fifth is output file name
int main(int argc, const char* argv[]){
    // Checking number of input
    if (argc != 6) {
        std::cout << "Invalid number of args";
        std::cout << "Usage: "<< argv[0] << " <input-file> <new-width> <new-height> <type> <output-file-name" << std::endl;
        std::cout << "<type> is one of the following values: near, bilinear, bicubic" << std::endl;
        return 1;
    }

    // Loading image
    ImageRGB input;
    if (!load_rgb(argv[1],input)) return 1;

    // Checking for integer
    int w = atoi(argv[2]), h = atoi(argv[3]);
    if (w == 0 || h == 0) {
        std::cout << "Given size is not valid";
        return 1;
    }

    // Computing the output
    ImageRGB output;
    if (strcmp(argv[4], "near") == 0){
        resize_nearest(input, output, w, h);
    }
    else if (strcmp(argv[4], "bilinear") == 0){
        resize_bilinear(input, output, w, h);
    }
    else if (strcmp(argv[4], "bicubic") == 0){
        resize_bicubic(input, output, w, h);
    }
    else {
        std::cout << "Unknown type" << std::endl;
    }

    // Saving the image
    if(!save_rgb(argv[5], output)){
        std::cout << "Fail to save file" << std::endl;
        return 1;
    };

    std::cout << "Input:" << input.w << "," << input.h << " Output:" << output.w << ',' << output.h;
    return 0;
}