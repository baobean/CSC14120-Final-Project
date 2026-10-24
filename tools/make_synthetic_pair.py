"""
make_synthetic_pair.py - make an image pair with a KNOWN homography, to measure V0's H error.
Authors: DHDH
External sources: none

SKELETON created by NDB. DHDH implements the TODOs.

TODO summary (Stage 1):
  Week 1 Day 4   DHDH  write this script
  Week 2 Day 8   DHDH  run V0 on the synthetic pair, check the H error is small

Idea: A = a real photo. B = A seen through a known H_true, where H_true maps a point of B
to A (the same direction as the program's H). Then:
    B(p) = A(H_true * p)        (sample A with bilinear interpolation, black outside A)

Output:
  <out_dir>/synth_A.png, <out_dir>/synth_B.png
  <out_dir>/H_true.txt   9 numbers, row-major, h[8] = 1

Usage:
  python tools/make_synthetic_pair.py dataset/small/site1_indoor_A.png dataset/synthetic/small
"""
import argparse
from pathlib import Path


def make_h_true(w, h):
    """Return a 3x3 numpy array: the known homography (B -> A)."""
    # TODO(DHDH, Week 1 Day 4): a camera turn like our real pairs, for example
    #   shift x by +0.35 * w (B shows the right part of A, about 65% overlap),
    #   a small rotation (about 2 degrees) and a small perspective term (h[6] ~ 1e-5).
    #   Normalize so H[2][2] = 1. Keep the numbers fixed (no randomness).
    raise NotImplementedError


def warp(img_a, H, out_w, out_h):
    """B(p) = A(H * p) with bilinear sampling, black where H * p is outside A."""
    # TODO(DHDH, Week 1 Day 4): plain numpy is fine (np.meshgrid for all p, apply H,
    #   divide by w, bilinear from the 4 neighbors). cv2.warpPerspective(img_a,
    #   np.linalg.inv(H), ...) gives the same result if you want to cross-check.
    raise NotImplementedError


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("image_a", type=Path)
    ap.add_argument("out_dir", type=Path)
    args = ap.parse_args()
    # TODO(DHDH, Week 1 Day 4): load A (RGB), H = make_h_true, B = warp(A, H, w, h),
    #   save synth_A.png (copy of A), synth_B.png and H_true.txt (np.savetxt, 9 values).
    raise NotImplementedError


if __name__ == "__main__":
    main()
