"""
homography_error.py - error of an estimated H against the true H (synthetic pair test).
Authors: DHDH
External sources: none

SKELETON created by NDB. DHDH implements the TODOs.

TODO summary (Stage 1):
  Week 1 Day 5   DHDH  write the error function
  Week 2 Day 12  DHDH  measure the H error on the synthetic pair (all 3 sizes)
  Week 3 Day 15  DHDH  notebook section: synthetic pair, H error vs true H

Error = mean over the 4 corners c of image B of || H_est(c) - H_true(c) ||, in pixels,
where H(c) maps c with H and divides by w. Below about 1-2 px is a good result.

The estimated H comes from the program output: with the warm-up run, ./stitch prints
one line "H,h0,h1,...,h8" (TODO NDB Week 2 Day 8 in v0_pipeline.cpp).

Usage:
  ./stitch synth_A.png synth_B.png out.png --version 0 --runs 1 > run.log
  python tools/homography_error.py H_true.txt run.log --width 1280 --height 720
"""
import argparse
from pathlib import Path


def read_h_from_log(log_path: Path):
    """Return the 3x3 H from the line that starts with 'H,' in the program output."""
    # TODO(DHDH, Week 1 Day 5): find the first line starting with "H,", split by ",",
    #   take the 9 numbers, reshape to 3x3. Error if no such line.
    raise NotImplementedError


def corner_error(H_est, H_true, w, h):
    """Mean distance in pixels between the 4 corners of B mapped by H_est and by H_true."""
    # TODO(DHDH, Week 1 Day 5): corners (0,0) (w-1,0) (0,h-1) (w-1,h-1); map each with
    #   both H (divide by the 3rd coordinate); return the mean Euclidean distance.
    raise NotImplementedError


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("h_true", type=Path, help="H_true.txt from make_synthetic_pair.py")
    ap.add_argument("log", type=Path, help="saved output of ./stitch")
    ap.add_argument("--width", type=int, required=True)
    ap.add_argument("--height", type=int, required=True)
    args = ap.parse_args()
    # TODO(DHDH, Week 1 Day 5): load both H, print the error with 3 decimals.
    raise NotImplementedError


if __name__ == "__main__":
    main()
