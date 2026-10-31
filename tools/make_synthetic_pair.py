"""
make_synthetic_pair.py - make an image pair with a KNOWN homography, to measure V0's H error.
Authors: NDB (written for DHDH's stage; reviewed by DHDH)
External sources: none
AI assistance: drafted with Claude Code (Anthropic), reviewed by NDB.

Idea: A = a real photo. B = A seen through a known H_true, where H_true maps a point of B
to A (the same direction as the program's H). Then:
    B(p) = A(H_true * p)        (sample A with bilinear interpolation, black outside A)

H_true = shift x by +0.35 * w (B shows the right part of A, about 65% overlap)
         * rotation of 2 degrees about the image center
         + a small perspective term h[6] = 1e-5 * 1280 / w.
All terms scale with the image size, so the 3 sizes get the same geometry.

Output:
  <out_dir>/synth_A.png, <out_dir>/synth_B.png
  <out_dir>/H_true.txt   9 numbers, row-major, h[8] = 1

Usage:
  python tools/make_synthetic_pair.py dataset/small/site1_indoor_A.png dataset/synthetic/small
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def make_h_true(w, h):
    """Return a 3x3 numpy array: the known homography (B -> A)."""
    a = np.deg2rad(2.0)
    cx, cy = (w - 1) / 2.0, (h - 1) / 2.0
    to_center = np.array([[1, 0, -cx], [0, 1, -cy], [0, 0, 1]], dtype=np.float64)
    rot = np.array([[np.cos(a), -np.sin(a), 0], [np.sin(a), np.cos(a), 0], [0, 0, 1]])
    back = np.array([[1, 0, cx + 0.35 * w], [0, 1, cy], [0, 0, 1]], dtype=np.float64)
    persp = np.array([[1, 0, 0], [0, 1, 0], [1e-5 * 1280.0 / w, 0, 1]])
    H = persp @ back @ rot @ to_center
    return H / H[2, 2]


def warp(img_a, H, out_w, out_h):
    """B(p) = A(H * p) with bilinear sampling, black where H * p is outside A."""
    ha, wa = img_a.shape[:2]
    xs, ys = np.meshgrid(np.arange(out_w, dtype=np.float64), np.arange(out_h, dtype=np.float64))
    den = H[2, 0] * xs + H[2, 1] * ys + H[2, 2]
    u = (H[0, 0] * xs + H[0, 1] * ys + H[0, 2]) / den
    v = (H[1, 0] * xs + H[1, 1] * ys + H[1, 2]) / den
    inside = (u >= 0) & (u <= wa - 1) & (v >= 0) & (v <= ha - 1)
    u = np.clip(u, 0, wa - 1)
    v = np.clip(v, 0, ha - 1)
    x0 = np.floor(u).astype(np.int64)
    y0 = np.floor(v).astype(np.int64)
    x1 = np.minimum(x0 + 1, wa - 1)
    y1 = np.minimum(y0 + 1, ha - 1)
    fx = (u - x0)[..., None]
    fy = (v - y0)[..., None]
    a = img_a.astype(np.float64)
    out = ((1 - fy) * ((1 - fx) * a[y0, x0] + fx * a[y0, x1]) +
           fy * ((1 - fx) * a[y1, x0] + fx * a[y1, x1]))
    out[~inside] = 0
    return np.clip(out + 0.5, 0, 255).astype(np.uint8)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("image_a", type=Path)
    ap.add_argument("out_dir", type=Path)
    args = ap.parse_args()
    img_a = np.asarray(Image.open(args.image_a).convert("RGB"))
    h, w = img_a.shape[:2]
    H = make_h_true(w, h)
    img_b = warp(img_a, H, w, h)
    args.out_dir.mkdir(parents=True, exist_ok=True)
    Image.fromarray(img_a).save(args.out_dir / "synth_A.png")
    Image.fromarray(img_b).save(args.out_dir / "synth_B.png")
    np.savetxt(args.out_dir / "H_true.txt", H.reshape(1, 9), fmt="%.10g")
    print(f"Wrote {args.out_dir}/synth_A.png, synth_B.png, H_true.txt ({w}x{h})")


if __name__ == "__main__":
    main()
