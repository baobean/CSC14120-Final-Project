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
import subprocess
import csv
import os
from pathlib import Path

SIZES = ["small", "medium", "large"]
IMG_OUT_PATH = Path.cwd() / Path("results/pano")

BENCH_RESULT_FILE_MODE = 'w'

def get_part(plain: str, pattern, end_pattern = '\n'):
    start = plain.find(pattern)
    if start == -1:
      return ""
    end = plain.find(end_pattern, start + 1)
    return plain[start:end]

def run_one(stitch: Path, a: Path, b: Path, out_img: Path, version: int, runs: int):
    """Run ./stitch once (it does 1 warm-up + `runs` timed runs). Return (gpu_name, csv_fields)."""
    complete = subprocess.run([stitch, a, b, out_img, "--version", str(version), '--runs', str(runs)] ,capture_output=True, text=True)
    stdout = complete.stdout
    GPU_line = get_part(stdout, "GPU:")
    CSV_line = get_part(stdout, "CSV")
    if not CSV_line:
      print("CSV line not found")
    CSV_line = CSV_line.strip("CSV").strip()
    GPU_line = GPU_line.strip("GPU: ").strip()
    return (GPU_line, CSV_line.split(","))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--stitch", type=Path, default=Path.cwd() / Path("stitch"))
    ap.add_argument("--dataset", type=Path, default=Path("dataset"))
    ap.add_argument("--out", type=Path, default=Path("results/timing_v0.csv"))
    ap.add_argument("--versions", type=int, nargs="+", default=[0])
    ap.add_argument("--runs", type=int, default=5)
    args = ap.parse_args()
    results = dict()
    
    # Parameter check that the default may failed
    if not Path.exists(args.dataset):
      print("Dataset does not exists")
      return 1
    
    if not Path.exists(args.stitch) or not Path.is_file(args.stitch):
      print("Not found stitch program")
      return 1
    
    # Creating folder to save results
    os.makedirs(IMG_OUT_PATH, exist_ok=True) 
    os.makedirs(args.out.parent, exist_ok=True)
    
    # Running on dataset
    for size in SIZES:
      folder:Path = args.dataset / size
      files:list[Path] =  [f for f in folder.iterdir() if f.is_file()]
      files.sort()
      # Assuming that there is an even number of file (i.e. there are two picture per set)
      for i in range(0, len(files), 2):
        file_stem = files[i].stem[:-2] # Only get all but the last 2 character (i.e the _A and _B) name
        save_file_full_path = IMG_OUT_PATH / f"{file_stem}_{size}.png"
        r = run_one(args.stitch, files[i], files[i + 1], save_file_full_path, args.versions, args.runs)
        results[(file_stem, size)] = r
        print(f"Done benchmark for set: {file_stem}, of size: {size}")
      
    # Saving results
    CSV_HEADER = ["gpu","version","pair","size","w","h","runs","pre","det","desc","match","homo","warp","e2e_avg","e2e_min"]
    with open(args.out, BENCH_RESULT_FILE_MODE, newline='') as f:
      writer = csv.DictWriter(f, fieldnames=CSV_HEADER)
      writer.writeheader()
      for key,value in results.items():
        file_stem, size = key
        gpu, csv_values = value
        if len(csv_values) != 13:
          csv_values = [-1 for _ in range(13)]
        
        writer.writerow({
          "gpu": gpu, 
          "version": csv_values[1], # Starting at 1 because the CLI output have an extra , (for some reason)
          "pair": file_stem,
          "size": size,
          "w": csv_values[2],
          "h": csv_values[3],
          "runs": csv_values[4],
          "pre": csv_values[5], 
          "det": csv_values[6], 
          "desc": csv_values[7],
          "match": csv_values[8],
          "homo": csv_values[9],
          "warp": csv_values[10],
          "e2e_avg": csv_values[11],
          "e2e_min": csv_values[12]
          })
    
    print("Done")

if __name__ == "__main__":
    main()
