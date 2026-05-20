#!/usr/bin/env python3
"""Align a folder of penguin PNGs by translating each frame so its bbox center
matches the median bbox center. Adapted from stack-chan/scripts/align_penguin_frames.py
(see docs/penguin-frame-alignment.md in that repo) with two changes:

  1. --src / --out paths configurable (default: ~/Desktop/gugugaga and ./out/aligned)
  2. --target-size N downsamples each frame to NxN before alignment, so source
     images of any resolution land on the same coord grid as the spec bboxes.

Usage:
    python3 align_penguin_frames.py
    python3 align_penguin_frames.py --src ~/Desktop/gugugaga --out ./out/aligned --target-size 240
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image

THRESHOLD = 30  # r+g+b > THRESHOLD counts as foreground


def load_frame(path: Path, target_size: int) -> np.ndarray:
    img = Image.open(path).convert("RGB")
    if img.size != (target_size, target_size):
        img = img.resize((target_size, target_size), Image.LANCZOS)
    return np.asarray(img, dtype=np.uint8)


def bbox_center(arr: np.ndarray) -> tuple[float, float]:
    mask = arr.astype(int).sum(axis=-1) > THRESHOLD
    rows = np.where(mask.any(axis=1))[0]
    cols = np.where(mask.any(axis=0))[0]
    if rows.size == 0 or cols.size == 0:
        h, w = mask.shape
        return (h / 2, w / 2)
    cy = (rows.min() + rows.max()) / 2
    cx = (cols.min() + cols.max()) / 2
    return (cy, cx)


def shift_frame(arr: np.ndarray, dy: int, dx: int) -> np.ndarray:
    shifted = np.roll(arr, (dy, dx), axis=(0, 1))
    if dy > 0:
        shifted[:dy] = 0
    elif dy < 0:
        shifted[dy:] = 0
    if dx > 0:
        shifted[:, :dx] = 0
    elif dx < 0:
        shifted[:, dx:] = 0
    return shifted


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", type=Path, default=Path("~/Desktop/gugugaga").expanduser())
    ap.add_argument("--out", type=Path, default=Path(__file__).parent / "out" / "aligned")
    ap.add_argument("--target-size", type=int, default=240,
                    help="all frames are resampled to this NxN before alignment")
    args = ap.parse_args()

    paths = sorted(args.src.glob("*.png"))
    if not paths:
        raise SystemExit(f"no *.png under {args.src}")

    frames = [load_frame(p, args.target_size) for p in paths]
    centers = np.array([bbox_center(a) for a in frames])
    ref_cy, ref_cx = np.median(centers, axis=0)

    args.out.mkdir(parents=True, exist_ok=True)
    # Side-by-side: stash the pre-aligned (resized) frames too, so preview HTML
    # can load both originals and aligned via relative paths under ./out/
    orig_dir = args.out.parent / "original"
    orig_dir.mkdir(parents=True, exist_ok=True)

    report: dict[str, object] = {
        "ref": [round(float(ref_cy), 2), round(float(ref_cx), 2)],
        "target_size": args.target_size,
        "threshold": THRESHOLD,
        "frames": {},
        "names": [p.name for p in paths],
    }

    print(f"ref bbox center (cy, cx) = ({ref_cy:.2f}, {ref_cx:.2f})")
    print(f"{'frame':24} {'orig_cy':>8} {'orig_cx':>8} {'dy':>4} {'dx':>4}")
    for path, arr, (cy, cx) in zip(paths, frames, centers):
        Image.fromarray(arr, mode="RGB").save(orig_dir / path.name)
        dy = int(round(ref_cy - cy))
        dx = int(round(ref_cx - cx))
        shifted = shift_frame(arr, dy, dx)
        out_path = args.out / path.name
        Image.fromarray(shifted, mode="RGB").save(out_path)
        print(f"{path.name:24} {cy:>8.2f} {cx:>8.2f} {dy:>4d} {dx:>4d}")
        report["frames"][path.name] = {
            "orig": [round(float(cy), 2), round(float(cx), 2)],
            "dy": dy,
            "dx": dx,
        }

    (args.out / "alignment-report.json").write_text(json.dumps(report, indent=2))
    # JS variant for the HTML preview (avoids fetch under file://)
    (args.out / "alignment-report.js").write_text(
        f"window.PENGUIN_ALIGNMENT = {json.dumps(report)};\n"
    )

    print(f"\nwrote {len(paths)} aligned frames to {args.out}")
    print("preview: open preview-alignment.html")


if __name__ == "__main__":
    main()
