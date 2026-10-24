# Method notes (input for notebook section 3.3)

Short notes per stage: what the method does, the parameters, and why it is good for both quality and the GPU. DHDH turns these into the method table in notebook 3.3 (Week 3 Day 17). The parameters are fixed in `src/common.h`.

TODO summary (Stage 1):

| Week | Day | Owner | What |
|---|---|---|---|
| 1 | 2 | LCK | Notes for stages 3–4 |
| 1 | 2 | DHDH | Notes for stages 5–6 |
| 2 | 10 | NDB | Notes for stages 1–2 (not in the day plan; must exist before Week 3 Day 17) |

For each stage, write 4 short bullets:
- **Method:** what it computes, in plain words.
- **Parameters:** values and units, from `common.h`.
- **Why (quality):** why it is good enough for our photos (rotate-only, same zoom).
- **Why (GPU):** which unit of work is independent (pixel, keypoint, pair, hypothesis).

## Stage 1. Preprocessing (NDB)

TODO(NDB, Week 2 Day 10): grayscale (BT.601 weights), separable Gaussian 5x5, sigma 1.0, clamp-to-edge border, 10 instead of 25 multiply-adds per pixel.

## Stage 2. Keypoint detection (NDB)

TODO(NDB, Week 2 Day 10): Sobel 3x3, Harris k = 0.04 on a 5x5 window, NMS 3x3, top K = 2000 sorted by score (tie: y, then x).

## Stage 3. Description (LCK)

TODO(LCK, Week 1 Day 2): BRIEF 256 bits, 31x31 patch, fixed pattern from seed 42, border skip 16 px. Why no rotation: rotate-only capture, same zoom.

## Stage 4. Matching (LCK)

TODO(LCK, Week 1 Day 2): brute-force Hamming (XOR + bit count), Lowe ratio 0.8, tie rule (smaller index wins). Why: all-pairs work, like matrix multiplication.

## Stage 5. Homography (DHDH)

TODO(DHDH, Week 1 Day 2): RANSAC with 1000 iterations, 3 px threshold, seed 42, shared sample list; 4-point DLT solved as an 8x8 system (h33 = 1) with Gaussian elimination.

## Stage 6. Warping and blending (DHDH)

TODO(DHDH, Week 1 Day 2): canvas from the warped corners of B, inverse warp with H⁻¹ and bilinear sampling, linear blend weight 1 → 0 across the overlap.
