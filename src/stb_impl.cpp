/*
 * stb_impl.cpp - compiles the stb image loader/writer once.
 * Authors: NDB
 * External sources: stb_image.h v2.30 and stb_image_write.h v1.16 by Sean Barrett,
 *                   https://github.com/nothings/stb (public domain / MIT).
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
// stb_image has SSE2/NEON code for JPEG decoding; turn it off so nothing in the
// program uses SIMD intrinsics (V0 rule), even in the untimed image loading.
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
