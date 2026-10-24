"""
prepare_dataset.py - crop each raw pair to 16:9 and resize it to the 3 benchmark sizes.
Authors: LCK
External sources: none

SKELETON created by NDB. LCK implements the TODOs.

TODO summary (Stage 1):
  Week 1 Day 4   LCK  write this script (crop 16:9, resize to 3 sizes)
  Week 1 Day 5   LCK  run it on all 4 pairs, upload dataset/ to the shared Drive

Input layout (raw photos, one folder per pair):
  raw/<pair>/A.jpg   left photo
  raw/<pair>/B.jpg   right photo
  <pair> = site1_indoor, site2_outdoor, site3_<name>, challenging

Output layout (what ./stitch and tools/run_benchmark.py read):
  dataset/small/<pair>_A.png    1280x720
  dataset/medium/<pair>_A.png   1920x1080
  dataset/large/<pair>_A.png    3840x2160
  (and <pair>_B.png next to each)

Usage:
  python tools/prepare_dataset.py raw/ dataset/
"""
import argparse
from pathlib import Path

SIZES = {"small": (1280, 720), "medium": (1920, 1080), "large": (3840, 2160)}


def crop_16_9(img):
    """Center-crop a PIL image to 16:9."""
    # TODO(LCK, Week 1 Day 4): if w / h > 16 / 9, crop the width to h * 16 / 9;
    #   otherwise crop the height to w * 9 / 16. Keep the crop centered.
    #   A and B of one pair must get the same crop box (they have the same size).
    raise NotImplementedError


def prepare_pair(pair_dir: Path, out_dir: Path):
    """Crop and resize A.jpg and B.jpg of one pair into the 3 size folders."""
    # TODO(LCK, Week 1 Day 4):
    #   - open with PIL, apply ImageOps.exif_transpose (phone photos store rotation in
    #     EXIF), convert("RGB")
    #   - crop_16_9, then resize to each size in SIZES with Image.LANCZOS
    #   - save as PNG to out_dir/<size>/<pair>_A.png and _B.png
    #   - check A and B have the same size before cropping, stop with an error if not
    raise NotImplementedError


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("raw_dir", type=Path)
    ap.add_argument("out_dir", type=Path)
    args = ap.parse_args()
    # TODO(LCK, Week 1 Day 4): loop over the sub-folders of raw_dir (sorted), call
    #   prepare_pair, print one line per pair and size written.
    # TODO(LCK, Week 1 Day 5): run on all 4 pairs, zip dataset/ + raw/, upload to the
    #   shared Drive, put the link in the notebook (section 3.6).
    raise NotImplementedError


if __name__ == "__main__":
    main()
