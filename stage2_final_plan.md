# Stage 2 Plan: Final (Parallel CUDA Versions + Report + Video)

CSC14120 Parallel Programming. Panorama image stitching.
Team: **NDB** (lead), **LCK**, **DHDH**.
Duration: 20 days (Day 1 to Day 20, after the Midterm).

Methods are the same as in Stage 1 (grayscale + blur, Harris, BRIEF, Hamming + ratio test, RANSAC + DLT, inverse warp + linear blend). Do not change them, so the timing comparison is fair.

## 1. What the Final asks for

Weight: 90% of the project grade (final submission + oral defence).

1. **V1 Basic CUDA**: every stage on the GPU, simple kernels (1 thread per pixel, keypoint, or match). Global memory is OK.
2. **V2, V3, ... Optimized**: at least 2. Each version adds **one** main idea, builds on the previous one, and must be motivated by profiling. Results must match V0.
3. **One program**: `./stitch <left> <right> <out> --version <0|1|2|...> [--runs N]`.
4. **Timing rules**: CUDA events per stage, 1 warm-up, average of >= 5 runs, report per-stage time, kernel time, H2D/D2H time, end-to-end time. Speedup vs V0 and vs the previous version, for all 3 resolutions. Same GPU, same Colab session. Print the GPU name.
5. **Profiling evidence** for each optimized version (Nsight Systems or Nsight Compute).
6. **Deliverables**: notebook (.ipynb, runs with "Run all", outputs saved) + PDF export; source code + README.md; slides; video 20-30 min on YouTube (Unlisted), link in notebook and README.

Oral defence: **every member must explain all code**, not only their own part.

## 2. Ownership and optimized versions

Same stage owners as Stage 1. Each member also owns one optimized version. The 3 ideas below are the easiest ones in the spec's menu.

| Member | V1 stages | Optimized version | Idea (simple words) |
|---|---|---|---|
| **LCK** | 3. Description, 4. Matching | **V2**: shared memory + `__popc` in matching | Matching is all-pairs, so it is usually the slowest stage. Load a block of B descriptors into shared memory once, and all threads in the block use it. Use `__popc` (1 instruction) to count bits. |
| **NDB** | 1. Preprocessing, 2. Detection | **V3**: shared-memory tiles for blur and Harris | Each block loads its image tile plus a border (halo) into shared memory once, then reads neighbors from there, not from global memory. Put blur weights in constant memory. |
| **DHDH** | 5. Homography, 6. Warping | **V4**: keep data on the GPU + pinned memory | Remove copies between host and device between stages. Use pinned host memory for the copies that stay. Optional: 2 CUDA streams so image A and image B run stages 1-3 at the same time. |

Shared work:
- **NDB**: CUDA timing harness, integration, README, notebook sections 1-3 and 9-10, submission.
- **LCK**: benchmark script, speedup tables and charts, slides, notebook section 7.
- **DHDH**: verify tool (compare with V0), results gallery, failure case, notebook sections 4 and 8, video editing.

**Note:** on Day 9 the team checks the V1 profile. If another stage is the biggest bottleneck, change the order of V2-V4. Each idea needs a measured reason.

## 3. Day-by-day plan

### Week 1: V1 Basic CUDA (Day 1 to Day 7)

| Day | NDB | LCK | DHDH |
|---|---|---|---|
| 1 | **Team meeting** (all): read midterm feedback, agree on V1 thread mapping. Write CUDA harness: `cudaEvent` per stage, H2D/D2H timing, warm-up, GPU name print, error-check macro in `cuda_utils.cuh`. | Join meeting. V1 stage 3: BRIEF kernel, 1 thread per keypoint. | Join meeting. V1 stage 6: inverse warp kernel, 1 thread per output pixel. |
| 2 | V1 stage 1: grayscale kernel + blur kernels (row pass, column pass), 1 thread per pixel. | V1 stage 3: copy the pixel-pair pattern to the GPU. Compare descriptors with V0. | V1 stage 6: linear blend kernel. Compare output with V0. |
| 3 | V1 stage 2: Sobel + Harris score kernel, 1 thread per pixel. | V1 stage 4: Hamming match kernel, 1 thread per keypoint of A, simple bit-count loop. | V1 stage 5: RANSAC kernel, 1 thread per iteration. Use the same random samples as V0 (made on host). |
| 4 | V1 stage 2: NMS kernel, collect corners with an atomic counter. Sort by score (`thrust::sort`), keep top K. | V1 stage 4: ratio test on GPU. | V1 stage 5: count inliers, pick the best H. |
| 5 | **Integration** (all): full V1 pipeline in `--version 1`. NDB merges. | Fix stage 3-4 bugs. | Write the verify tool: compare with V0 (keypoint count, match count, H difference, output image difference). |
| 6 | Fix integration bugs. Keypoints must be sorted so V1 gives the same order as V0. | Write the benchmark script: all versions x 3 resolutions x 4 pairs, save CSV. | Run verify on all pairs. Report differences to owners. |
| 7 | **Freeze V1** (all). Tag `v1`. | Run the benchmark for V0 and V1. | Run Nsight Systems on V1, medium and large. Save screenshots. |

### Week 2: Optimized versions (Day 8 to Day 14)

| Day | NDB | LCK | DHDH |
|---|---|---|---|
| 8 | Notebook: V1 section for stages 1-2 (thread mapping, timing). | Notebook: V1 section for stages 3-4. Run Nsight Compute on the match kernel. | Notebook: V1 section for stages 5-6. |
| 9 | **Profile meeting** (all): read the V1 profile, find the top bottleneck, confirm the order of V2-V4. Start V3: shared-memory tile + halo for blur. | Start V2: load tiles of B descriptors into shared memory. | Start V4: list every `cudaMemcpy` in V1. Mark the ones that are not needed. |
| 10 | V3: shared-memory tile for Sobel + Harris window sums. Blur weights in constant memory. | V2: replace the bit-count loop with `__popc`. | V4: keep all data on the GPU between stages. Use `cudaMallocHost` (pinned memory) for the input and output copies. |
| 11 | V3: test on all sizes. Wait for V2 tag. | V2: verify vs V0. Tag `v2`. Run benchmark + Nsight Compute (memory throughput). | V4 (optional): 2 CUDA streams for image A and image B in stages 1-3. |
| 12 | V3: build on V2. Verify vs V0. Tag `v3`. Benchmark + Nsight Compute. | Notebook: V2 section (bottleneck, idea, code, result, analysis). | V4: build on V3. Verify vs V0. Tag `v4`. Nsight Systems timeline (fewer copies, overlap if streams). |
| 13 | Notebook: V3 section (5 parts). | Help DHDH with V4 bugs if needed. | Notebook: V4 section (5 parts). |
| 14 | **Freeze code** (all). Run full benchmark: V0-V4, 3 resolutions, same GPU, same session. | Make the final speedup table and charts (vs V0 and vs previous). | Make the per-stage breakdown chart of the best version. |

### Week 3: Report, slides, video (Day 15 to Day 20)

| Day | NDB | LCK | DHDH |
|---|---|---|---|
| 15 | Notebook: 1. Team info + contribution table, 2. Introduction, 3. Chosen methods. | Notebook: 7. Final comparison (table + charts from output cells). | Notebook: 8. Results gallery, team pairs + failure case with explanation. |
| 16 | Write README.md: setup, CUDA version, compile, run examples, hardware, expected output. | Slides: problem, pipeline, methods, versions, results. | Notebook: 4. V0 summary. Check that V1-V4 outputs match V0. |
| 17 | Notebook: 9. Conclusion, 10. References + AI tool declaration. | Slides: speedup charts, lessons learned. | Video plan: who says what, demo script. |
| 18 | **Review day** (all): fresh Colab GPU, Runtime -> Run all. Check every number is from an output cell. Export PDF. | Check charts and text. | Check gallery and README commands. |
| 19 | Record video part: problem overview + approach (4-6 min). | Record: performance results and analysis (6-9 min). | Record: live demo (6-9 min). All record: own optimized version (4-6 min total). DHDH edits and uploads to YouTube (Unlisted). |
| 20 | Add video link to notebook and README. Package and submit. | **Defence practice** (all): each member explains all stages and all versions. | Buffer for last fixes. |

## 4. Final notebook sections and owners

| Section | Owner |
|---|---|
| 1. Team info + contribution table | NDB |
| 2. Introduction | NDB |
| 3. Chosen methods | NDB (from midterm) |
| 4. V0 Sequential | DHDH (from midterm) |
| 5. V1 Basic CUDA | Each owner writes own stages |
| 6. V2 / V3 / V4 | LCK / NDB / DHDH |
| 7. Final comparison | LCK |
| 8. Results gallery + failure case | DHDH |
| 9. Conclusion | NDB |
| 10. References + AI declaration | NDB (all add their sources) |

## 5. Workload check

| Member | V1 kernels | Optimized version | Shared work | Video |
|---|---|---|---|---|
| NDB | Stages 1, 2 | V3 | Harness, integration, README, sections 1-3, 9-10, submission | Overview |
| LCK | Stages 3, 4 | V2 | Benchmark script, tables, charts, slides, section 7 | Results |
| DHDH | Stages 5, 6 | V4 | Verify tool, gallery, failure case, sections 4, 8, video editing | Demo |

All: one optimization segment in the video, defence practice.

## 6. Rules for all members

- Each version builds on the previous one. Do not remove an optimization without a written reason.
- Every version must give results consistent with V0. Run the verify tool before each tag.
- A slower version is OK if you explain why.
- All timings in one table: same GPU, same Colab session.
- Never edit results or images by hand. Fabricated results give a zero.
- Cite all external code. Record AI tool use. Each member must be able to explain and change any part of the code.
