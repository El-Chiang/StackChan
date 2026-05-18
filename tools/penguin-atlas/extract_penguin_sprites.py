#!/usr/bin/env python3
"""Extract eye + mouth sprite sheets from aligned penguin frames.

Adapted from stack-chan/scripts/extract_penguin_sprites.py with two changes:

  1. variants are read from penguin-face-spec.json (data-driven) instead of
     being hardcoded in the script
  2. additionally emits LVGL ARGB8888 .c files in the format the firmware
     expects (matching firmware/main/stackchan/avatar/skins/penguin/assets/*.c)

Outputs (under --out):
  sprites/
    penguin_base.png            — body base (from spec.base_frame)
    penguin_eyelid_left.png     — horizontal sheet, N variants
    penguin_eyelid_right.png    — same for right
    penguin_mouth.png           — mouth sheet
    atlas.json / atlas.js       — variants + layout for preview HTML
    variants.txt                — paste-snippet for eyes.cpp / mouth.cpp
  c/
    penguin_base.c
    penguin_eyelid_left.c
    penguin_eyelid_right.c
    penguin_mouth.c

Usage:
    python3 extract_penguin_sprites.py
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

# ---------- sprite extraction ----------

def crop(img: Image.Image, bbox: list[int]) -> Image.Image:
    return img.crop(tuple(bbox))


def make_sheet(crops: list[Image.Image], cell_w: int, cell_h: int) -> Image.Image:
    """Horizontally concatenate variant crops into a single sprite sheet."""
    sheet = Image.new("RGB", (cell_w * len(crops), cell_h), "black")
    for i, c in enumerate(crops):
        sheet.paste(c.convert("RGB"), (i * cell_w, 0))
    return sheet


# ---------- LVGL ARGB8888 .c writer ----------

def write_lvgl_c(out_path: Path, name: str, img: Image.Image) -> None:
    """Emit an LVGL v9 ARGB8888 image descriptor. Byte order is BGRA
    (little-endian 32-bit pixel), matching the existing penguin_*.c assets."""
    img = img.convert("RGBA")
    w, h = img.size
    rgba = img.tobytes()  # RGBA order in memory

    bgra = bytearray(len(rgba))
    for i in range(0, len(rgba), 4):
        bgra[i + 0] = rgba[i + 2]  # B
        bgra[i + 1] = rgba[i + 1]  # G
        bgra[i + 2] = rgba[i + 0]  # R
        bgra[i + 3] = rgba[i + 3]  # A

    upper = name.upper()
    lines = [
        "#if defined(LV_LVGL_H_INCLUDE_SIMPLE)",
        '#include "lvgl.h"',
        "#elif defined(LV_LVGL_H_INCLUDE_SYSTEM)",
        "#include <lvgl.h>",
        "#elif defined(LV_BUILD_TEST)",
        '#include "../lvgl.h"',
        "#else",
        '#include "lvgl/lvgl.h"',
        "#endif",
        "",
        "#ifndef LV_ATTRIBUTE_MEM_ALIGN",
        "#define LV_ATTRIBUTE_MEM_ALIGN",
        "#endif",
        "",
        f"#ifndef LV_ATTRIBUTE_{upper}",
        f"#define LV_ATTRIBUTE_{upper}",
        "#endif",
        "",
        "static const",
        f"LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_{upper}",
        f"uint8_t {name}_map[] = {{",
        "",
    ]
    chunk = 16
    row_stride = w * 4
    for row in range(h):
        row_bytes = bgra[row * row_stride : (row + 1) * row_stride]
        for col_start in range(0, row_stride, chunk * 4):
            byts = row_bytes[col_start:col_start + chunk * 4]
            lines.append("    " + ",".join(f"0x{b:02x}" for b in byts) + ",")
    lines += [
        "};",
        "",
        f"const lv_image_dsc_t {name} = {{",
        "  .header = {",
        "    .magic = LV_IMAGE_HEADER_MAGIC,",
        "    .cf = LV_COLOR_FORMAT_ARGB8888,",
        "    .flags = 0,",
        f"    .w = {w},",
        f"    .h = {h},",
        f"    .stride = {w * 4},",
        "    .reserved_2 = 0,",
        "  },",
        f"  .data_size = sizeof({name}_map),",
        f"  .data = {name}_map,",
        "  .reserved = NULL,",
        "};",
        "",
    ]
    out_path.write_text("\n".join(lines))


# ---------- main ----------

def build_sheet_for(part_key: str, part: dict, variants: list, aligned_dir: Path) -> tuple[Image.Image, list]:
    """Read each variant's source frame from `aligned_dir`, crop, stack.

    Each variant is [label, src] or [label, src, [dx, dy]]. The optional
    offset translates the part's default bbox per-variant (positive dx = crop
    region shifts right in source → eye visually moves LEFT on composite).
    """
    crops = []
    labels = []
    x1, y1, x2, y2 = part["bbox"]
    for entry in variants:
        if len(entry) == 2:
            label, src = entry
            dx, dy = 0, 0
        else:
            label, src, off = entry
            dx, dy = off
        src_path = aligned_dir / src
        if not src_path.exists():
            raise SystemExit(f"missing aligned frame: {src_path}\n"
                             f"  (referenced by sprites.{part_key} variant '{label}')\n"
                             f"  run align_penguin_frames.py first")
        frame = Image.open(src_path).convert("RGB")
        bbox = [x1 + dx, y1 + dy, x2 + dx, y2 + dy]
        crops.append(crop(frame, bbox))
        labels.append(label)
    return make_sheet(crops, part["w"], part["h"]), labels


def main() -> None:
    ap = argparse.ArgumentParser()
    here = Path(__file__).parent
    ap.add_argument("--spec",    type=Path, default=here / "penguin-face-spec.json")
    ap.add_argument("--aligned", type=Path, default=here / "out" / "aligned")
    ap.add_argument("--out",     type=Path, default=here / "out")
    ap.add_argument("--no-c",    action="store_true", help="skip .c emission (PNG only)")
    args = ap.parse_args()

    spec = json.loads(args.spec.read_text())
    sprites_out = args.out / "sprites"
    c_out       = args.out / "c"
    sprites_out.mkdir(parents=True, exist_ok=True)
    if not args.no_c:
        c_out.mkdir(parents=True, exist_ok=True)

    # 1. base body
    base_src = args.aligned / spec["base_frame"]
    if not base_src.exists():
        raise SystemExit(f"missing base frame: {base_src} — run align_penguin_frames.py first")
    base_img = Image.open(base_src).convert("RGB")
    base_img.save(sprites_out / "penguin_base.png")
    print(f"wrote {sprites_out / 'penguin_base.png'}  (from {spec['base_frame']})")

    # 2. sprite sheets for each part
    parts = spec["parts"]
    sprites = spec["sprites"]
    sheets: dict[str, Image.Image] = {}
    label_lists: dict[str, list[str]] = {}
    for part_key in ["left_eye", "right_eye", "mouth"]:
        sheet, labels = build_sheet_for(part_key, parts[part_key], sprites[part_key], args.aligned)
        sheets[part_key] = sheet
        label_lists[part_key] = labels
        out_name = {
            "left_eye":  "penguin_eyelid_left.png",
            "right_eye": "penguin_eyelid_right.png",
            "mouth":     "penguin_mouth.png",
        }[part_key]
        sheet.save(sprites_out / out_name)
        print(f"wrote {sprites_out / out_name}  ({sheet.width}x{sheet.height}, {len(labels)} variants: {', '.join(labels)})")

    # 3. atlas.json + atlas.js for preview HTML
    atlas = {
        "base": "penguin_base.png",
        "left_eye":  { **{k: parts["left_eye"][k]  for k in ("cx", "cy", "w", "h")},
                       "sprite": "penguin_eyelid_left.png",  "variants": label_lists["left_eye"] },
        "right_eye": { **{k: parts["right_eye"][k] for k in ("cx", "cy", "w", "h")},
                       "sprite": "penguin_eyelid_right.png", "variants": label_lists["right_eye"] },
        "mouth":     { **{k: parts["mouth"][k]     for k in ("cx", "cy", "w", "h")},
                       "sprite": "penguin_mouth.png",        "variants": label_lists["mouth"] },
        "emotions": spec.get("emotions", {}),
    }
    (sprites_out / "atlas.json").write_text(json.dumps(atlas, indent=2))
    (sprites_out / "atlas.js").write_text(f"window.PENGUIN_ATLAS = {json.dumps(atlas)};\n")
    print(f"wrote {sprites_out / 'atlas.json'} and atlas.js")

    # 4. emotion → variant index snippet for eyes.cpp / mouth.cpp
    def idx(part, label):
        try:
            return label_lists[part].index(label)
        except ValueError:
            return 0
    emo = spec.get("emotions", {})
    snippet = ["// generated by extract_penguin_sprites.py",
               "// emotion → (eye_variant, mouth_variant) index"]
    for name in ["Neutral", "Happy", "Angry", "Sad", "Doubt", "Sleepy"]:
        e = emo.get(name)
        if not e or not isinstance(e, dict):
            snippet.append(f"//   {name}: (no mapping)")
            continue
        eye = idx("left_eye",  e.get("eye", ""))
        mo  = idx("mouth",     e.get("mouth", ""))
        snippet.append(f"//   case Emotion::{name}: setEyeVariant({eye}); setMouthVariant({mo}); break;  // eye={e.get('eye')} mouth={e.get('mouth')}")
    snippet_text = "\n".join(snippet) + "\n"
    (sprites_out / "variants.txt").write_text(snippet_text)
    print()
    print(snippet_text)

    # 5. LVGL .c files
    if not args.no_c:
        write_lvgl_c(c_out / "penguin_base.c",         "penguin_base",         base_img)
        write_lvgl_c(c_out / "penguin_eyelid_left.c",  "penguin_eyelid_left",  sheets["left_eye"])
        write_lvgl_c(c_out / "penguin_eyelid_right.c", "penguin_eyelid_right", sheets["right_eye"])
        write_lvgl_c(c_out / "penguin_mouth.c",        "penguin_mouth",        sheets["mouth"])
        print(f"wrote 4 LVGL .c files to {c_out}")


if __name__ == "__main__":
    main()
