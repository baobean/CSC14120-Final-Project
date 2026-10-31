"""
homography_error.py - error of an estimated H against the true H (synthetic pair test).
Authors: NDB (written for DHDH's stage; reviewed by DHDH)
External sources: none
AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.

Error = mean over the 4 corners c of image B of || H_est(c) - H_true(c) ||, in pixels,
where H(c) maps c with H and divides by w. Below about 1-2 px is a good result.

The estimated H comes from the program output: with the warm-up run, ./stitch prints
one line "H,h0,h1,...,h8".

Usage:
  ./stitch synth_A.png synth_B.png out.png --version 0 --runs 1 > run.log
  python tools/homography_error.py H_true.txt run.log --width 1280 --height 720
"""
import argparse
from pathlib import Path

import numpy as np


def read_h_from_log(log_path: Path):
    """Return the 3x3 H from the line that starts with 'H,' in the program output."""
    for line in Path(log_path).read_text().splitlines():
        if line.startswith("H,"):
            vals = [float(v) for v in line.strip().split(",")[1:]]
            if len(vals) != 9:
                raise ValueError(f"expected 9 numbers in the H line, got {len(vals)}: {line}")
            return np.array(vals).reshape(3, 3)
    raise ValueError(f"no line starting with 'H,' in {log_path}")


def corner_error(H_est, H_true, w, h):
    """Mean distance in pixels between the 4 corners of B mapped by H_est and by H_true."""
    corners = np.array([[0, 0, 1], [w - 1, 0, 1], [0, h - 1, 1], [w - 1, h - 1, 1]], dtype=np.float64).T
    pe = H_est @ corners
    pt = H_true @ corners
    pe = pe[:2] / pe[2]
    pt = pt[:2] / pt[2]
    return float(np.mean(np.linalg.norm(pe - pt, axis=0)))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("h_true", type=Path, help="H_true.txt from make_synthetic_pair.py")
    ap.add_argument("log", type=Path, help="saved output of ./stitch")
    ap.add_argument("--width", type=int, required=True)
    ap.add_argument("--height", type=int, required=True)
    args = ap.parse_args()
    H_true = np.loadtxt(args.h_true).reshape(3, 3)
    H_est = read_h_from_log(args.log)
    print(f"mean corner error: {corner_error(H_est, H_true, args.width, args.height):.3f} px")


if __name__ == "__main__":
    main()
