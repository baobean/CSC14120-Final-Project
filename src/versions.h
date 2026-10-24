/*
 * versions.h - one entry point per version, selected by --version in main.cu.
 * Authors: NDB
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 */
#pragma once

#include "common.h"

// Every version has the same signature: stitch B onto A's frame, write the
// panorama into `out`, and fill the per-stage times of this one run.
typedef void (*PipelineFn)(const ImageRGB& A, const ImageRGB& B, ImageRGB& out,
                           StageTimes& t, const PipelineOptions& opt);

// V0: sequential, single thread (v0_pipeline.cpp)
void run_v0(const ImageRGB& A, const ImageRGB& B, ImageRGB& out,
            StageTimes& t, const PipelineOptions& opt);

// V1..V4 are added here in Stage 2.

struct VersionEntry {
    int id;
    const char* name;
    PipelineFn fn;
};

static const VersionEntry VERSIONS[] = {
    {0, "V0 sequential", run_v0},
};
static const int NUM_VERSIONS = sizeof(VERSIONS) / sizeof(VERSIONS[0]);

// Returns nullptr if the version does not exist.
inline const VersionEntry* get_version(int id) {
    for (int i = 0; i < NUM_VERSIONS; i++)
        if (VERSIONS[i].id == id) return &VERSIONS[i];
    return nullptr;
}
