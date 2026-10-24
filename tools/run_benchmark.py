"""
run_benchmark.py - run ./stitch on every pair and size, collect the per-stage timing.
Authors: LCK
External sources: none

SKELETON created by NDB. LCK implements the TODOs.

TODO summary (Stage 1):
  Week 3 Day 15  LCK  run 3 sizes x 4 pairs, 5 runs each; save one CSV
  (Stage 2 Week 1 Day 6 extends it to all versions)

./stitch prints one line per run that starts with "CSV,":
  CSV,version,w,h,runs,pre,det,desc,match,homo,warp,e2e_avg,e2e_min   (times in ms)
and a line "GPU: <name>" first.

Output CSV columns:
  gpu,version,pair,size,w,h,runs,pre,det,desc,match,homo,warp,e2e_avg,e2e_min

Usage:
  python tools/run_benchmark.py --stitch ./stitch --dataset dataset --out results/timing_v0.csv
"""
import argparse
from pathlib import Path

SIZES = ["small", "medium", "large"]


def run_one(stitch: Path, a: Path, b: Path, out_img: Path, version: int, runs: int):
    """Run ./stitch once (it does 1 warm-up + `runs` timed runs). Return (gpu_name, csv_fields)."""
    # TODO(LCK, Week 3 Day 15): subprocess.run([...], capture_output=True, text=True,
    #   check=True); find the "GPU:" line and the "CSV," line; return them split.
    #   Print the program's stdout if the CSV line is missing.
    raise NotImplementedError


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--stitch", type=Path, default=Path("./stitch"))
    ap.add_argument("--dataset", type=Path, default=Path("dataset"))
    ap.add_argument("--out", type=Path, default=Path("results/timing_v0.csv"))
    ap.add_argument("--versions", type=int, nargs="+", default=[0])
    ap.add_argument("--runs", type=int, default=5)
    args = ap.parse_args()
    # TODO(LCK, Week 3 Day 15): for each version, size in SIZES, pair (every
    #   dataset/<size>/*_A.png, sorted): run_one, append a row; write the CSV with the
    #   header above (csv module). Save panoramas to results/pano/<pair>_<size>.png.
    raise NotImplementedError


if __name__ == "__main__":
    main()
