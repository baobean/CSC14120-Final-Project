# CSC14120 Final Project: Panorama Image Stitching

Team: NDB (lead), LCK, DHDH.

> TODO summary (Stage 1):
>
> | Week | Day | Owner | What |
> |---|---|---|---|
> | 3 | 19 | DHDH | Fill every section below for V0 and check that the commands work on a fresh Colab session. README is DHDH's in the Stage 1 plan, but no day is set; Day 19 (review day) is the suggestion. NDB completes it for all versions in Stage 2 (Week 3 Day 16). |

## Setup

TODO(DHDH, Week 3 Day 19): Google Colab with a GPU runtime, CUDA version (`nvcc --version`), Python packages used by `tools/` (Pillow, numpy).

## Build

```
make            # builds ./stitch with nvcc
```

## Run

```
./stitch <left_image> <right_image> <output_image> --version <0|1|...> [--runs N] [--debug DIR]
```

TODO(DHDH, Week 3 Day 19): one example per tool:
- `tools/prepare_dataset.py`
- `tools/make_synthetic_pair.py`
- `tools/homography_error.py`
- `tools/run_benchmark.py`

## Repository layout

TODO(DHDH, Week 3 Day 19): one line per folder (`src/`, `tools/`, `docs/`) and per V0 stage file, with its owner.

## Hardware

TODO(DHDH, Week 3 Day 19): the GPU printed by `./stitch` on Colab (e.g. Tesla T4) and its memory.

## Expected output

TODO(DHDH, Week 3 Day 19): what the timing table and the `CSV,` line look like, and where the panoramas and debug images go.

## Dataset

TODO(DHDH, Week 3 Day 19): the shared Drive link (from LCK, Week 1 Day 5) and the `dataset/<size>/<pair>_A.png` layout.
