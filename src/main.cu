/*
 * main.cu - command line, image I/O and timing harness for all versions.
 * Authors: NDB
 * External sources: none
 * AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.
 *
 * Usage: ./stitch <left> <right> <out> --version <0|1|...> [--runs N] [--debug DIR]
 *
 * Timing rules (project spec 4.2): 1 warm-up run (not timed), then N timed runs.
 * Each version times its own stages; main also times the whole call (end-to-end).
 * Image loading and saving are not timed.
 */
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "common.h"
#include "cuda_utils.cuh"
#include "image_io.h"
#include "versions.h"

static void usage(const char* prog) {
    fprintf(stderr,
            "Usage: %s <left_image> <right_image> <output_image> --version <id> "
            "[--runs N] [--debug DIR]\n"
            "  --version  pipeline version (required)\n"
            "  --runs     number of timed runs, default 5 (plus 1 warm-up)\n"
            "  --debug    write intermediate images to DIR (warm-up run only)\n"
            "Available versions:\n",
            prog);
    for (int i = 0; i < NUM_VERSIONS; i++)
        fprintf(stderr, "  %d  %s\n", VERSIONS[i].id, VERSIONS[i].name);
}

int main(int argc, char** argv) {
    if (argc < 4) {
        usage(argv[0]);
        return 1;
    }
    const char* left_path = argv[1];
    const char* right_path = argv[2];
    const char* out_path = argv[3];
    int version = -1;
    int runs = 5;
    const char* debug_dir = nullptr;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 && i + 1 < argc) {
            version = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--runs") == 0 && i + 1 < argc) {
            runs = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--debug") == 0 && i + 1 < argc) {
            debug_dir = argv[++i];
        } else {
            fprintf(stderr, "Unknown or incomplete argument: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }
    const VersionEntry* ver = get_version(version);
    if (!ver) {
        fprintf(stderr, "Unknown or missing --version (%d)\n", version);
        usage(argv[0]);
        return 1;
    }
    if (runs < 1) {
        fprintf(stderr, "--runs must be >= 1\n");
        return 1;
    }

    print_gpu_name();

    ImageRGB A, B, out;
    if (!load_rgb(left_path, A) || !load_rgb(right_path, B)) return 1;
    if (A.w != B.w || A.h != B.h) {
        fprintf(stderr, "Images must have the same size: %dx%d vs %dx%d\n", A.w, A.h, B.w, B.h);
        return 1;
    }
    printf("Version: %s | Image: %dx%d | Runs: %d (+1 warm-up)\n", ver->name, A.w, A.h, runs);

    // Warm-up run: not timed, prints counts and writes debug images.
    StageTimes t;
    PipelineOptions warm_opt;
    warm_opt.debug_dir = debug_dir;
    warm_opt.verbose = true;
    ver->fn(A, B, out, t, warm_opt);

    // Timed runs.
    PipelineOptions opt;
    std::vector<StageTimes> all(runs);
    std::vector<double> e2e(runs);
    for (int r = 0; r < runs; r++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        ver->fn(A, B, out, all[r], opt);
        auto t1 = std::chrono::high_resolution_clock::now();
        e2e[r] = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // Report: average and minimum per stage.
    double avg[NUM_STAGES], mn[NUM_STAGES];
    for (int s = 0; s < NUM_STAGES; s++) {
        avg[s] = 0.0;
        mn[s] = all[0].ms[s];
        for (int r = 0; r < runs; r++) {
            avg[s] += all[r].ms[s];
            mn[s] = std::min(mn[s], all[r].ms[s]);
        }
        avg[s] /= runs;
    }
    double e2e_avg = 0.0, e2e_min = e2e[0], sum_avg = 0.0;
    for (int r = 0; r < runs; r++) {
        e2e_avg += e2e[r];
        e2e_min = std::min(e2e_min, e2e[r]);
    }
    e2e_avg /= runs;
    for (int s = 0; s < NUM_STAGES; s++) sum_avg += avg[s];

    printf("%-16s %12s %12s %8s\n", "Stage", "avg (ms)", "min (ms)", "share");
    for (int s = 0; s < NUM_STAGES; s++)
        printf("%-16s %12.3f %12.3f %7.1f%%\n", STAGE_NAMES[s], avg[s], mn[s],
               sum_avg > 0 ? 100.0 * avg[s] / sum_avg : 0.0);
    printf("%-16s %12.3f\n", "sum of stages", sum_avg);
    printf("%-16s %12.3f %12.3f\n", "end-to-end", e2e_avg, e2e_min);

    // One machine-readable line for the benchmark script (stage averages, then end-to-end).
    printf("CSV,%d,%d,%d,%d", ver->id, A.w, A.h, runs);
    for (int s = 0; s < NUM_STAGES; s++) printf(",%.3f", avg[s]);
    printf(",%.3f,%.3f\n", e2e_avg, e2e_min);

    if (!save_rgb(out_path, out)) return 1;
    printf("Output written to %s (%dx%d)\n", out_path, out.w, out.h);
    return 0;
}
