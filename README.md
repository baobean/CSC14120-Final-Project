# CSC14120 Final Project: Panorama Image Stitching

Team: NDB (lead), LCK, DHDH.

Two photos taken by rotating the camera in one place (left `A`, right `B`) are stitched into one panorama:
grayscale + blur → Harris corners → BRIEF descriptors → Hamming matching → RANSAC homography → warp + blend.
Stage 1 (midterm) is the sequential version V0; Stage 2 adds the CUDA versions V1–V4.

> TODO summary (Stage 1):
>
> | Week | Day | Owner | What |
> |---|---|---|---|
> | 3 | 19 | DHDH | Fill the `TODO` values below (GPU, CUDA version, Drive link) from a fresh Colab session. Written by NDB for DHDH; DHDH checks every command. NDB completes it for all versions in Stage 2 (Week 3 Day 16). |

## Setup

Google Colab with a GPU runtime (Runtime → Change runtime type → GPU). `nvcc` is already installed there.

- CUDA version: TODO(DHDH, Week 3 Day 19): the `release` line of `nvcc --version`.
- Python packages for `tools/`: `numpy`, `Pillow` (both preinstalled on Colab); the notebook also uses `pandas`, `matplotlib`.

The notebook `GroupXX_Midterm.ipynb` does the setup in its first cells (copy the code, `make`, print the GPU, get the dataset).

## Build

```
make                          # builds ./stitch with nvcc (-O2, no OpenMP)
make clean
make submission GROUP=<n>     # Group<n>_Midterm.zip = notebook + src/
```

## Run

```
./stitch <left_image> <right_image> <output_image> --version <0|1|...> [--runs N] [--debug DIR]
```

- `--version` picks the pipeline (only `0` = V0 sequential in Stage 1).
- `--runs N` timed runs after 1 warm-up run (default 5).
- `--debug DIR` writes intermediate images from the warm-up run.

Example:

```
./stitch dataset/small/site1_indoor_A.png dataset/small/site1_indoor_B.png out/site1_small.png --version 0 --runs 5 --debug debug/site1
```

Tools (Python 3):

```
# crop the raw photos to 16:9 and resize to the 3 sizes (LCK)
python tools/prepare_dataset.py raw/ dataset/

# synthetic pair with a known homography H_true (B -> A)
python tools/make_synthetic_pair.py dataset/small/site1_indoor_A.png dataset/synthetic/small

# H error: run V0 on the synthetic pair, then compare its H line with H_true
./stitch dataset/synthetic/small/synth_A.png dataset/synthetic/small/synth_B.png out/synth_small.png --version 0 --runs 1 > run_small.log
python tools/homography_error.py dataset/synthetic/small/H_true.txt run_small.log --width 1280 --height 720

# timing of every pair and size, one CSV (LCK)
python tools/run_benchmark.py --stitch ./stitch --dataset dataset --out results/timing_v0.csv
```

## Repository layout

| Path | What | Owner |
|---|---|---|
| `src/main.cu` | command line, image loading, timing harness (1 warm-up + N runs), report | NDB |
| `src/common.h`, `src/versions.h`, `src/v0.h` | shared data types, fixed parameters, version table, V0 stage interfaces | NDB |
| `src/v0_pipeline.cpp` | V0: runs stages 1–6, times each one, debug output | NDB |
| `src/v0_preprocess.cpp` | stage 1: grayscale + separable Gaussian blur | NDB |
| `src/v0_detect.cpp` | stage 2: Sobel + Harris + NMS + top K | NDB |
| `src/v0_describe.cpp`, `src/brief_pattern.*` | stage 3: BRIEF descriptors and the fixed pixel-pair pattern | LCK |
| `src/v0_match.cpp` | stage 4: Hamming matching + ratio test | LCK |
| `src/v0_homography.cpp`, `src/ransac_samples.*` | stage 5: RANSAC + 4-point homography, the shared sample list | DHDH |
| `src/v0_warp.cpp` | stage 6: inverse warp + linear blend | DHDH |
| `src/debug_draw.*` | debug images of keypoints (NDB) and matches (LCK) | NDB, LCK |
| `src/image_io.*`, `src/stb_impl.cpp`, `src/stb_image*.h` | image loading / saving (stb) | NDB |
| `src/cuda_utils.cuh` | CUDA error check, GPU name | NDB |
| `tools/` | dataset preparation, synthetic pair, H error, benchmark scripts | LCK, DHDH |
| `docs/method_notes.md` | method notes per stage (input for notebook 3.3) | all |
| `script/resize/` | standalone C++ image resizer (nearest, bilinear, bicubic) | LCK |

## Hardware

TODO(DHDH, Week 3 Day 19): the `GPU:` line printed by `./stitch` on Colab (e.g. Tesla T4, 15 GB). V0 runs on the Colab CPU, single thread.

## Expected output

`./stitch` prints the GPU, the image size, then (warm-up run) the counts, the keypoint spread and the estimated homography, then the timing table and one `CSV,` line:

```
GPU: <name> (compute X.Y, N GB)
Version: V0 sequential | Image: 1280x720 | Runs: 5 (+1 warm-up)
V0 counts: keypoints A=2000 B=2000 | described A=... B=... | matches=... | inliers=...
V0 spread (8x8 grid cells with a keypoint): A=../64 B=../64
H,h0,h1,h2,h3,h4,h5,h6,h7,1
Stage                avg (ms)     min (ms)    share
1.preprocess              ...
...
6.warp+blend              ...
sum of stages             ...
end-to-end                ...
CSV,version,w,h,runs,pre,det,desc,match,homo,warp,e2e_avg,e2e_min
Output written to <output_image> (WxH)
```

- The panorama goes to `<output_image>` (PNG, or JPEG if the name ends in `.jpg`).
- With `--debug DIR`: `gray_*`, `blur_*`, `Ix_*`, `Iy_*`, `harris_*`, `keypoints_*` for A and B, and `matches.png` (TODO(LCK, Week 2 Day 10): not written yet).
- The `H,` line is read by `tools/homography_error.py`; the `CSV,` line by `tools/run_benchmark.py`.

## Dataset

TODO(DHDH, Week 3 Day 19): the shared Drive link (from LCK, Week 1 Day 5).

Layout: `dataset/<small|medium|large>/<pair>_A.png` and `<pair>_B.png`, with sizes 1280x720, 1920x1080, 3840x2160 and
`<pair>` = `site1_indoor`, `site2_outdoor`, `site3_<name>`, `challenging`. `dataset/` is not in Git.
