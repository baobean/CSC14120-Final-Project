# Stage 1 Plan: Midterm (Proposal + Sequential Version V0)

CSC14120 Parallel Programming. Panorama image stitching.
Team: **NDB** (lead), **LCK**, **DHDH**.
Duration: 20 days (Day 1 to Day 20).

## 1. What the Midterm asks for

The Midterm is **not only** the sequential version. It has 3 parts:

1. **Proposal**: team info and work plan, problem explanation, chosen methods (with parameters and reasons), dataset, GPU plan.
2. **Sequential version V0**: single-thread C/C++. No OpenMP, no `std::thread`, no hand-written SIMD.
3. **V0 results**: homography error on the synthetic pair, one panorama per site, per-stage timing for 3 resolutions, hot stages, Amdahl estimate.

Weight: 10% of the project grade.

Submit: `GroupXX_Midterm.zip` = `GroupXX_Midterm.ipynb` (all outputs saved) + `src/` (`common.h`, `cuda_utils.cuh`, `versions.h`, `main.cu`, `stb_impl.cpp`, `v0_<name>.cpp`, other files) + a dataset link in the notebook. Only one member submits.

## 2. Chosen methods (simplest that still works)

Goal: easy to understand, easy to code, easy to explain in the defence.

| Stage | Method | What it does (simple words) | Parameters |
|---|---|---|---|
| 1. Preprocessing | Grayscale + Gaussian blur 5x5 (separable: 1 row pass + 1 column pass) | Gray = 0.299R + 0.587G + 0.114B. Blur = weighted average of neighbors. | sigma = 1.0 |
| 2. Keypoint detection | Harris corner + NMS 3x3 + keep top K | Sobel gives gradients Ix, Iy. Sum Ix², Iy², IxIy in a 5x5 window. Score R = det - k·trace². Keep a pixel only if it is the max of its 3x3 neighbors. Sort by score, keep the best K. | k = 0.04, K = 2000 |
| 3. Description | BRIEF, 256 bits (no rotation) | Compare 256 fixed pixel pairs around the keypoint. Each pair gives 1 bit (1 if first pixel is brighter). | patch 31x31, fixed random pattern (seed 42) |
| 4. Matching | Brute-force Hamming + Lowe ratio test | For each keypoint in A, test all keypoints in B. Distance = number of different bits. Keep the match only if best < 0.8 × second best. | ratio = 0.8 |
| 5. Homography | RANSAC + 4-point DLT | Take 4 random matches, solve H (8x8 linear system, Gaussian elimination, set h33 = 1). Count matches that fit H. Repeat, keep the best H. | 1000 iterations, threshold 3 px, seed 42 |
| 6. Warping + blending | Inverse warp + bilinear sampling + linear blend | For each output pixel, use H⁻¹ to find where it comes from in B. Mix A and B in the overlap with a weight that goes from 1 to 0 across the overlap. | - |

**Why these are simple:**
- No rotation or scale handling is needed. The photos are taken by rotating the camera in one place, at the same zoom, so plain BRIEF works.
- No SVD library. Gaussian elimination on an 8x8 system is short code.
- No image pyramid, no multi-band blending.
- All stages are good for the GPU: blur and Harris are per pixel, BRIEF is per keypoint, matching is an all-pairs loop (bit count), RANSAC hypotheses are independent, warping is per pixel.

Use the **same** methods and parameters in all versions (V0 to V4).

## 3. Ownership (fixed for both stages)

Each member owns 2 pipeline stages. The same owner writes the V0 code and later the CUDA code for those stages.

| Member | Pipeline stages | Shared work |
|---|---|---|
| **NDB** | 1. Preprocessing, 2. Keypoint detection | Build and timing harness, Git repo, notebook setup cell, final integration |
| **LCK** | 3. Description (BRIEF), 4. Matching | Dataset preparation script (crop 16:9, resize 3 sizes), timing tables and charts |
| **DHDH** | 5. Homography (RANSAC), 6. Warping + blending | Synthetic pair test (homography error), panoramas gallery, README |

Each member also: captures 1 site, writes notebook sections for own stages, writes the GPU plan for own stages.

## 4. Day-by-day plan

### Week 1: setup, dataset, first code (Day 1 to Day 7)

| Day | NDB | LCK | DHDH |
|---|---|---|---|
| 1 | Read both specs and the starter notebook. Make the Git repo and the `src/` layout. | Read both specs. Read about BRIEF and Hamming distance. | Read both specs. Read about RANSAC, homography, inverse warping (Szeliski ch. 8). |
| 2 | **Team meeting** (all): read section 2 together, agree on the data structs in `common.h` (image, keypoint, descriptor, match, homography). NDB writes the structs. | Join meeting. Write short method notes for stages 3-4. | Join meeting. Write short method notes for stages 5-6. |
| 3 | Build the starter on Colab: `main.cu`, `stb_impl.cpp`, `versions.h`, the `--version` and `--runs` flags. | Site 2 (outdoor): capture photos. Rotate, do not walk. 30-50% overlap. | Site 3 (team choice): capture photos. Also capture the challenging pair (low texture or night). |
| 4 | Site 1 (indoor): capture photos. Add the timing harness: `std::chrono` per stage, 1 warm-up run, average of N runs. | Write the dataset script: crop to 16:9, resize to 1280x720, 1920x1080, 3840x2160. | Write the synthetic pair tool: warp one image with a known H, save the true H. |
| 5 | V0 stage 1: grayscale. Test on small image. | Run the dataset script on all 4 pairs. Upload `dataset/` to the shared Drive. | Write the homography error function (mean corner error in pixels vs true H). |
| 6 | V0 stage 1: separable Gaussian blur. Save debug images. | V0 stage 3: make the 256 fixed pixel pairs (seed 42). | V0 stage 5: 8x8 Gaussian elimination to solve H from 4 points. Test with 4 exact points. |
| 7 | **Check-in** (all, 30 min): show progress, fix interface issues. V0 stage 2: Sobel gradients Ix, Iy. | Join check-in. V0 stage 3: compute the 256-bit descriptor for each keypoint. | Join check-in. V0 stage 5: RANSAC loop (4 random matches, solve H, count inliers, keep best). |

### Week 2: finish V0 (Day 8 to Day 14)

| Day | NDB | LCK | DHDH |
|---|---|---|---|
| 8 | V0 stage 2: Harris score R for each pixel. | V0 stage 3: skip keypoints closer than 16 px to the border. Test on small image. | V0 stage 5: test on the synthetic pair. Check the H error is small. |
| 9 | V0 stage 2: NMS 3x3, sort by score, keep top K. Draw keypoints in a debug image. | V0 stage 4: Hamming distance (XOR + count bits). Find best and second best for each keypoint. | V0 stage 6: find the output canvas size from the 4 warped corners of B. |
| 10 | Connect stages 1-2 to the pipeline. Check keypoint count and spread on all sites. | V0 stage 4: ratio test. Draw matches in a debug image. | V0 stage 6: inverse warp with bilinear sampling. |
| 11 | **Integration day** (all): run full V0 on small images, all sites. NDB merges the code. | Integration: fix stage 3-4 bugs. | Integration: fix stage 5-6 bugs. V0 stage 6: linear blend in the overlap. |
| 12 | Fix integration bugs. Check: no OpenMP, no threads, no SIMD. | Test the ratio 0.7 / 0.8 on the challenging pair. Keep 0.8 unless results are bad. | Measure H error on the synthetic pair. |
| 13 | Add the author comment at the top of each file you wrote. Cite any adapted code. | Same for your files. | Same for your files. |
| 14 | **Freeze V0 code** (all). Tag the commit `v0-midterm`. Run on medium and large; check run time. | Check results on medium and large. | Make one panorama per site and one for the challenging pair. |

### Week 3: results, GPU plan, notebook (Day 15 to Day 20)

| Day | NDB | LCK | DHDH |
|---|---|---|---|
| 15 | Notebook setup cell: copy `src/` from Drive, compile, print GPU name. Must run with "Run all". | Run timing: 3 resolutions x 4 pairs, 5 runs each. Save as CSV. | Notebook: synthetic pair section (H error vs true H). |
| 16 | Notebook 3.2: problem explanation (pipeline in own words). | Notebook 3.5: per-stage timing table, stacked bar chart, hot stages and how they change with resolution. | Notebook 3.6: dataset section (sites, pairs, photos). Panorama gallery. |
| 17 | Notebook 3.1: team info + work plan table (use section 3 of this file and the Stage 2 plan). | Notebook 3.5: Amdahl estimate. Compute p from the timing, and the limit 1/(1-p). | Notebook 3.3: method table with parameters and reasons (from section 2). |
| 18 | Notebook 3.7: GPU plan for stages 1-2. | Notebook 3.7: GPU plan for stages 3-4. | Notebook 3.7: GPU plan for stages 5-6. |
| 19 | **Review day** (all): fresh Colab session, Runtime -> Run all. Check every number comes from an output cell. NDB fixes setup issues. | Check text against outputs. Fix charts. | Check the dataset link works. Check the panoramas. |
| 20 | Make `GroupXX_Midterm.zip`. Submit (NDB only). Buffer for late fixes. | Buffer. Read CUDA basics for Stage 2. | Buffer. Read CUDA basics for Stage 2. |

## 5. GPU plan draft (for notebook 3.7)

| Stage | V1 thread mapping | Later optimization ideas |
|---|---|---|
| 1. Preprocessing | 1 thread per pixel | Shared-memory tile with halo, constant memory for blur weights |
| 2. Detection | 1 thread per pixel; atomic counter to collect corners; sort for top K | Shared-memory tile for Sobel and window sums |
| 3. Description | 1 thread per keypoint | Pixel-pair pattern in constant memory |
| 4. Matching | 1 thread per keypoint of A, loop over all of B | Tiles of B descriptors in shared memory, `__popc` bit count |
| 5. Homography | 1 thread per RANSAC iteration | Random samples made once on host, same in all versions |
| 6. Warping | 1 thread per output pixel | Keep data on GPU, no copy back between stages |

Risks: GPU results differ from V0 (random numbers, atomics change keypoint order); large images are slow on V0.
Fix: use the same seed and the same list of random samples in all versions; sort keypoints by score before description.

## 6. Workload check

| Member | V0 stages | Shared work | Notebook sections |
|---|---|---|---|
| NDB | 1, 2 | Repo, harness, integration, submission | 3.1, 3.2, 3.7 (part) |
| LCK | 3, 4 | Dataset script, timing, charts, Amdahl | 3.5, 3.7 (part) |
| DHDH | 5, 6 | Synthetic test, panoramas, gallery | 3.3, 3.6, 3.7 (part) |

Each member: 2 stages + 1 site + about 2 notebook sections.

## 7. Rules for all members

- Push code every day. Do not edit another member's stage without telling them.
- Use the same methods and parameters as in section 2.
- Every number in the notebook must come from an output cell.
- Cite all external code in the source file and in the notebook. Record AI tool use.

## 8. Progress (as of 2026-10-31)

Read this first in the next work session, and update it at the end of every session.
Status words: **Done**, **Done (untested)**, **Partial**, **Not started**, **Blocked** (by whom/what), **Needs a person** (photos, IDs, Colab runs, submitting: things Claude cannot do).
"by NDB" = NDB wrote it for the owner (agreed with the team lead); the owner reviews it and owns it from then on.
"Tested locally" = g++ build on a laptop without a GPU, with a stand-in for stages 3–4 kept outside the repo; not yet run on Colab.

### NDB

| Week/Day | Task | Status | Note |
|---|---|---|---|
| W1 D1–3 | Repo, `src/` layout, structs, CLI | Done | commit c58fa9d |
| W1 D4 | Timing harness | Done | c58fa9d. Site 1 photos: **Needs a person** (not in Git, no Drive link yet) |
| W1 D5–6 | Grayscale, separable blur | Done | 3caf0a6 |
| W1 D7 | Sobel | Done | 8fda5c1 |
| W2 D8 | Harris score; `H,` line in the output | Done, tested locally | `v0_harris` in `v0_detect.cpp`; `v0_pipeline.cpp` prints `H,h0..h8` |
| W2 D9 | NMS 3x3, sort (score, y, x), top K; keypoint debug image | Done, tested locally | `draw_keypoints`; debug dir also gets `harris_*`, `keypoints_*`. Rectangles test: exactly the 16 corners |
| W2 D10 | Keypoint count + spread; method notes 1–2 | Partial | `V0 spread` line (8x8 grid) added, notes done. Checking it on all sites is **Blocked** (dataset) |
| W2 D11 | Integration on small images, all sites | Blocked | LCK stages 3–4 + dataset |
| W2 D12 | No OpenMP / threads / SIMD | Done | grep clean; `STBI_NO_SIMD` added in `stb_impl.cpp`. Integration bug fixing: Blocked |
| W2 D13 | Author comments | Done | NDB files + files written for DHDH |
| W2 D14 | Freeze, tag `v0-midterm`, run medium/large | Blocked | after integration: `git tag v0-midterm && git push origin v0-midterm` |
| W3 D15 | Notebook setup + dataset cells | Done (untested on Colab) | + helper cell (`run_stitch`, `parse_counts`). Needs `DRIVE_PROJECT` folder check and LCK's `DATASET_DRIVE_ID` |
| W3 D16 | Notebook 3.2 | Done | |
| W3 D17 | Notebook 3.1 | Done | names / student IDs: **Needs a person** |
| W3 D18 | Notebook 3.7 stages 1–2 | Done | |
| W3 D19 | Review day: Run all on fresh Colab; references check | Partial | references + AI table written (others add theirs); Run all: **Needs a person** |
| W3 D20 | Zip + submit; replace `XX` | Partial | `make submission GROUP=<n>` ready (zip = notebook + `src/` + `Makefile` + `tools/`). Group number + submit: **Needs a person** |

### LCK (from Git commits e8dbbfc, 952e3b9 and the code)

| Week/Day | Task | Status | Note |
|---|---|---|---|
| W1 D2 | Method notes 3–4 | Not started | `docs/method_notes.md` still TODO |
| W1 D3 | Site 2 photos | Needs a person | not in Git, unknown |
| W1 D4 | Dataset script (crop 16:9, 3 sizes) | Partial | `script/resize/` = C++ resizer (nearest / bilinear / bicubic, 1 image per call). No 16:9 crop, no loop over pairs/sizes. `tools/prepare_dataset.py` still a skeleton |
| W1 D5 | Run on 4 pairs, upload to Drive | Not started | no link; notebook needs `DATASET_DRIVE_ID` (cell 3) and the link in 3.6 |
| W1 D6 | 256 BRIEF pairs, seed 42 | Done (untested) | `brief_pattern.cpp`: mt19937(42), `% 31 − 15`, redraws identical points |
| W1 D7 | BRIEF descriptor | Done (untested), **does not compile** | see open issues 1–5 |
| W2 D8 | Skip keypoints near the border | Partial | margin 15 px, plan says 16 |
| W2 D9–10 | Hamming, best/second, ratio test; `draw_matches` | Not started | `v0_match.cpp`, `debug_draw.cpp` stubs |
| W2 D12 | Ratio 0.7 vs 0.8 on the challenging pair | Not started | |
| W2 D13 | Author comments | Not started | |
| W3 D15–17 | Benchmark CSV, timing table + chart, hot stages, Amdahl | Not started | `tools/run_benchmark.py` skeleton; notebook 3.5.3–3.5.5 TODO |
| W3 D18 | Notebook 3.7 stages 3–4 | Not started | |
| — | Check rows 3–4 of the method table (3.3), add references / AI use | Not started | |

### DHDH

| Week/Day | Task | Status | Note |
|---|---|---|---|
| W1 D2 | Method notes 5–6 | Done by NDB | DHDH to review |
| W1 D3 | Site 3 + challenging pair photos | Needs a person | not in Git, unknown |
| W1 D4 | `tools/make_synthetic_pair.py` | Done by NDB, tested locally | H_true = shift 0.35w + 2° rotation + perspective, same geometry at all sizes |
| W1 D5 | `tools/homography_error.py` | Done by NDB, tested locally | |
| W1 D6 | `solve_h4` (8x8 Gaussian elimination) | Done by NDB, tested locally | exact 4 points → reprojection 2.5e-13 px; collinear points rejected |
| W1 D7 | RANSAC loop, `ransac_samples.cpp` | Done by NDB, tested locally | 300 matches with 40% outliers: all 180 inliers found; same result every run |
| W2 D8 | Test on the synthetic pair | Done by NDB, tested locally | with the stand-in stages 3–4; must be rerun with LCK's code |
| W2 D9–11 | Canvas, inverse warp + bilinear, linear blend | Done by NDB, tested locally | identity H gives exactly A; no visible seam |
| W2 D12 | H error on the synthetic pair | Blocked | LCK stages 3–4; notebook cell 3.5.1 does it for the 3 sizes |
| W2 D13 | Author comments | Done by NDB | |
| W2 D14 | One panorama per site + challenging | Blocked | dataset + LCK; notebook cell 3.5.2 does it |
| W3 D15 | Notebook 3.5.1 | Done by NDB (untested on Colab) | |
| W3 D16 | Notebook 3.6 + thumbnails, panorama gallery | Done by NDB (untested on Colab) | where/when/who of each site, and why the challenging pair is hard: **Needs a person**; 1–2 sentences per panorama after the run |
| W3 D17 | Notebook 3.3 method table | Done by NDB | LCK checks the rows for stages 3–4 |
| W3 D18 | Notebook 3.7 stages 5–6 | Done by NDB | |
| W3 D19 | README; check Drive link + panoramas | Partial | README written by NDB; CUDA version, GPU, Drive link still TODO |

### Open issues

1. **The build fails** (`make`): `v0_describe.cpp` uses `memset` without `#include <cstring>` (LCK).
2. `v0_describe.cpp`: `memset(desc[i].bits, 0, BRIEF_BITS)` clears 256 bytes of a 32-byte struct, which overflows the heap. Use `sizeof(desc[i].bits)` (LCK).
3. `v0_describe.cpp`: pixel index is `(x+dx)*w + y+dy`; it must be `(y+dy)*w + (x+dx)` (LCK).
4. `v0_describe.cpp`: `kept.erase(remain_size + 1, ...)` keeps one point below the threshold; and the "score < 1% of max" filter itself is not in section 2. Agree on it as a team or remove it (LCK).
5. `v0_describe.cpp`: border margin 15 px vs 16 px in the plan (LCK).
6. H error without a final refit: on the synthetic pair (with the stand-in stages 3–4) the mean corner error was 0.8–2.8 px. A least-squares refit of H on all RANSAC inliers would lower it, but it is not in the section 2 method table. Team decision (DHDH owns stage 5).
7. Placeholders to fill: group number (title, file name), names + student IDs (3.1), site details (3.6), Drive link + `DATASET_DRIVE_ID` (cell 3, 3.6, README), CUDA version + GPU (README), notebook 3.4 (owner not set).
8. `DRIVE_PROJECT` in the setup cell assumes the shared Drive folder `MyDrive/CSC14120-Final-Project` with `src/`, `tools/`, `Makefile` and `dataset.zip`. Check the real folder name.
