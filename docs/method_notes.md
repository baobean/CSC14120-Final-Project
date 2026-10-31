# Method notes (input for notebook section 3.3)

Short notes per stage: what the method does, the parameters, and why it is good for both quality and the GPU. DHDH turns these into the method table in notebook 3.3 (Week 3 Day 17). The parameters are fixed in `src/common.h`.

TODO summary (Stage 1):

| Week | Day | Owner | What |
|---|---|---|---|
| 1 | 2 | LCK | Notes for stages 3–4 |
| 1 | 2 | DHDH | Notes for stages 5–6 |
| 2 | 10 | NDB | Notes for stages 1–2 (done) |

For each stage, write 4 short bullets:
- **Method:** what it computes, in plain words.
- **Parameters:** values and units, from `common.h`.
- **Why (quality):** why it is good enough for our photos (rotate-only, same zoom).
- **Why (GPU):** which unit of work is independent (pixel, keypoint, pair, hypothesis).

## Stage 1. Preprocessing (NDB)

- **Method:** gray = 0.299 R + 0.587 G + 0.114 B (ITU-R BT.601 luma weights), kept as float 0..255. Then a 5x5 Gaussian blur done as two 1D passes (rows, then columns). Pixels outside the image take the value of the nearest edge pixel (clamp-to-edge).
- **Parameters:** `BLUR_SIGMA = 1.0`, `BLUR_RADIUS = 2` (5 taps per pass, weights normalized to sum 1).
- **Why (quality):** Harris and BRIEF compare single pixel values, so sensor noise would create fake corners and flip descriptor bits. A light blur (sigma 1) removes that noise but keeps real corners sharp. Color is not needed to find corners.
- **Why (GPU):** every output pixel is independent: 1 thread per pixel for gray, row pass and column pass. The separable form needs 10 instead of 25 multiply-adds per pixel and only reads one row or one column, which suits a shared-memory tile with a 2-pixel halo later (V3).

## Stage 2. Keypoint detection (NDB)

- **Method:** 3x3 Sobel gives the gradients Ix, Iy. For each pixel, sum Ix², Iy², IxIy over a 5x5 window (separable box sum) into the matrix M = [Sxx Sxy; Sxy Syy]. Score R = det(M) − k·trace(M)²: R > 0 at corners (2 strong directions), R < 0 on edges, R ≈ 0 on flat areas. Non-maximum suppression 3x3: keep a pixel only if R > 0 and R is strictly greater than its 8 neighbors (the 1-pixel border is skipped). Sort by R, highest first; equal R by y, then x. Keep the first K.
- **Parameters:** `HARRIS_K = 0.04`, `HARRIS_WIN = 5`, `MAX_KEYPOINTS = 2000`.
- **Why (quality):** Harris corners are found again at the same place after a small rotation, which is all our rotate-only, same-zoom pairs need, so there is no scale pyramid. NMS gives one point per corner instead of a blob. The full sort key (score, y, x) makes the list exactly the same every run.
- **Why (GPU):** Sobel, the window sums, R and the NMS test are all 1 thread per pixel. Only the collection of corners needs care: an atomic counter appends them in a random order, and sorting by (score, y, x) gives back exactly V0's list. Later: shared-memory tile for Sobel and the window sums (V3).

## Stage 3. Description (LCK)

TODO(LCK, Week 1 Day 2): BRIEF 256 bits, 31x31 patch, fixed pattern from seed 42, border skip 16 px. Why no rotation: rotate-only capture, same zoom.

## Stage 4. Matching (LCK)

TODO(LCK, Week 1 Day 2): brute-force Hamming (XOR + bit count), Lowe ratio 0.8, tie rule (smaller index wins). Why: all-pairs work, like matrix multiplication.

## Stage 5. Homography (DHDH)

TODO(DHDH, Week 1 Day 2): RANSAC with 1000 iterations, 3 px threshold, seed 42, shared sample list; 4-point DLT solved as an 8x8 system (h33 = 1) with Gaussian elimination.

## Stage 6. Warping and blending (DHDH)

TODO(DHDH, Week 1 Day 2): canvas from the warped corners of B, inverse warp with H⁻¹ and bilinear sampling, linear blend weight 1 → 0 across the overlap.
