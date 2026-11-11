# Method notes (input for notebook section 3.3)

Short notes per stage: what the method does, the parameters, and why it is good for both quality and the GPU. DHDH turns these into the method table in notebook 3.3 (Week 3 Day 17). The parameters are fixed in `src/common.h`.

TODO summary (Stage 1):

| Week | Day | Owner | What |
|---|---|---|---|
| 1 | 2 | LCK | Notes for stages 3–4 |
| 1 | 2 | DHDH | Notes for stages 5–6 (written by NDB, done) |
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

- **Method:** BRIEF is used as the descriptor for key points with a patch of 31 x 31. The points are generated once on-the-fly (i.e the points are generated on first call and all next call will use the created values) with `mt19937` C++ random engine and `uniform_int_distribution` for a uniform number generation with random seed of `42`.
- **Parameters:** `BRIEF_BITS = 256` bits long, `BRIEF_PATCH = 31` pixel wide square, `BRIEF_SEED = 42`
- **Why (quality):**
  - BRIEF is selected as it used the binary descritor and it improve the SIFT decriptor by randomly select a set of smaller (and fixed) points instead of using all pixel of the patch to create the descriptor (both of which allows faster creation of descriptor). It is also choosen as the input images does not have any rotation.
  - Random seed is set to allow repeatable runs. Uniform point selection is use as it is simple and does not lose much recognition rate (based on [this article](https://gilscvblog.com/2013/09/19/a-tutorial-on-binary-descriptors-part-2-the-brief-descriptor/))
- **Possible GPU implementation:** All keypoints can be used to compute independly from each other and each image.
- **Note:** Due to the patch size, all keypoints which are too close to the border (the difference between the point to closed border is less then 31/ 2 = 15) are removed before BRIEF calculation.

## Stage 4. Matching (LCK)

- **Method:** A basic brute-force method with Hamming distance is used to find the distance between two keypoints of two images. After that, all points that is significant (i.e have a distance ratio between first and second best distance smaller than 0.8 - Lowe's ratio) is consider a match. For distance tie, the smallest index is choosen. Bit counting is based on [this page on bit operation](https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable) if the compiler is MSVC and based on `__builtin_popcountll` for gcc.
- **Parameter:** `RATIO_TEST = 0.8`
- **Why (quality):**
  - For current implementation, a brute-force method is a good enough (for current version and with 2000 keypoints per image max). Brute-force method give correct result as it compare all possible combination of keypoint pairs.
  - A Lowe's ratio of 0.8 is choosen as 0.7 give quite a bad result on cloud image pairs <!-- May need to change to image set instead of a description and add evident (but how?) -->
  - Bit counting not based on `__builtin_popcountll` by default as developing computer does not support `__builtin_popcountll` (current compliler is MSVC) and GPU does not have linux support.
  - Smallest index is choosen to allow the fulter version have consistent results.
- **Possible GPU implementation** Each of the keypoint in image B can be independly compare to all of image A.

## Stage 5. Homography (DHDH)

- **Method:** RANSAC. Each iteration takes 4 matches from a fixed sample list, solves the homography H (B → A) from those 4 point pairs, and counts the matches that H maps within the threshold of their partner (inliers). The H with the most inliers wins (on a tie, the earlier iteration). 4-point solve: with h33 = 1, each pair gives 2 linear equations, so 4 pairs give an 8x8 system, solved by Gaussian elimination with partial pivoting; a pivot below 1e-10 means the 4 points are degenerate (e.g. 3 on a line) and the iteration is skipped. **Refit:** the winning H fits its own 4 points exactly, so it carries their pixel noise. V0 then fits H again to all of its inliers by least squares: every inlier gives the same 2 equations, and the normal equations (AᵀA) h = Aᵀb are again an 8x8 system, solved the same way. The points are first normalized (centroid at 0, mean distance √2; Hartley 1997) so the system stays well conditioned at 3840x2160. The inlier count is then taken for the final H.
- **Parameters:** `RANSAC_ITERS = 1000`, `RANSAC_THRESH = 3.0` px, `RANSAC_SEED = 42`. The sample list (`ransac_samples.cpp`) is `mt19937(42)`, index = `rng() % n`, repeated indexes inside one iteration are drawn again.
- **Why (quality):** many matches are wrong (repeated texture, ratio test is not perfect). RANSAC ignores them as long as some 4-match samples are all correct; 1000 iterations is plenty for 30–50% inliers. A homography is the exact model for a camera that only rotates. No SVD library is needed. The refit averages the noise of hundreds of points instead of 4: on the synthetic pair the mean corner error goes from 1–3 px to about 0.1 px.
- **Why (GPU):** the 1000 hypotheses are independent: 1 thread per iteration solves its 8x8 system and counts its inliers, then a reduction picks the best count (smallest iteration index on a tie). The refit is one small solve, done once on the host. The sample list is made once on the host, so every version tests the same hypotheses and finds the same H.

## Stage 6. Warping and blending (DHDH)

- **Method:** map B's 4 corners with H; the canvas is the bounding box of those and A's rectangle (A is shifted by (tx, ty) if B reaches left of or above A). For every canvas pixel, H⁻¹ (3x3 inverse by adjugate / determinant) gives the point in B, sampled with bilinear interpolation (4 neighbors). Where A and B overlap, out = α·A + (1 − α)·B, with α going from 1 at the left edge of the overlap to 0 at its right edge (A is the left image). Only A → A, only B → B, neither → black. If the canvas would be more than 4x A in width or height (bad H), the output is A alone, with a warning.
- **Parameters:** none.
- **Why (quality):** inverse warping leaves no holes (every output pixel gets a value), and bilinear sampling avoids jagged edges. The linear blend hides small exposure differences and small alignment errors at the seam.
- **Why (GPU):** 1 thread per output pixel, all independent, and the work per pixel is the same. H⁻¹ and the overlap edges are computed once on the host and can go in constant memory.
