from pathlib import Path
from collections import Counter, defaultdict
import os
import hashlib

ROOT = Path(r"I:\Projects\corrina plant")
MAX_TREE_DEPTH = 3
TOP_N_LARGEST = 25
TOP_N_DUPLICATES = 20

IGNORE_DIRS = {
    ".git", "__pycache__", ".vscode", ".idea", "node_modules",
    "dist", "build", ".vs", "Debug", "Release", ".pio"
}

MESSY_KEYWORDS = [
    "copy", "backup", "old", "temp", "test", "finalfinal",
    "new folder", "untitled", "draft", "ver", "version"
]

LIKELY_PROJECT_FILES = {
    ".ino", ".py", ".cpp", ".c", ".h", ".hpp", ".md",
    ".stl", ".step", ".f3d", ".csv", ".json", ".txt",
    ".kicad_pcb", ".kicad_sch", ".pcbdoc", ".schdoc"
}

IMAGE_EXTS = {".png", ".jpg", ".jpeg", ".webp"}
VIDEO_EXTS = {".mp4", ".mov", ".avi", ".mkv"}


def human_size(num):
    for unit in ["B", "KB", "MB", "GB", "TB"]:
        if num < 1024:
            return f"{num:.1f} {unit}"
        num /= 1024
    return f"{num:.1f} PB"


def safe_stat_size(path):
    try:
        return path.stat().st_size
    except Exception:
        return 0


def file_hash(path, chunk_size=65536):
    h = hashlib.sha256()
    try:
        with open(path, "rb") as f:
            while True:
                chunk = f.read(chunk_size)
                if not chunk:
                    break
                h.update(chunk)
        return h.hexdigest()
    except Exception:
        return None


def print_tree(path, prefix="", depth=0, max_depth=3, lines=None):
    if lines is None:
        lines = []
    if depth > max_depth:
        return lines
    try:
        items = sorted(path.iterdir(), key=lambda p: (p.is_file(), p.name.lower()))
    except Exception:
        lines.append(prefix + "[unreadable]")
        return lines

    for i, item in enumerate(items):
        connector = "└── " if i == len(items) - 1 else "├── "
        lines.append(prefix + connector + item.name)

        if item.is_dir() and item.name not in IGNORE_DIRS:
            extension = "    " if i == len(items) - 1 else "│   "
            print_tree(item, prefix + extension, depth + 1, max_depth, lines)
    return lines


def choose_cover_images(images):
    scored = []
    for img in images:
        name = img.name.lower()
        score = 0
        if "cover" in name:
            score += 10
        if "final" in name:
            score += 7
        if "front" in name:
            score += 6
        if "assembled" in name:
            score += 6
        if "photo" in name:
            score += 5
        if "plant" in name:
            score += 5
        if "monitor" in name:
            score += 5
        if "box" in name:
            score += 3
        if "display" in name:
            score += 3
        score -= len(img.parts) * 0.1
        scored.append((score, img))
    scored.sort(key=lambda x: (-x[0], str(x[1]).lower()))
    return [p for _, p in scored[:3]]


if not ROOT.exists():
    print(f"Path does not exist: {ROOT}")
    raise SystemExit(1)

file_count = 0
dir_count = 0
ext_counter = Counter()
size_by_ext = defaultdict(int)
largest_files = []
messy_files = []
candidate_project_dirs = Counter()
readme_dirs = set()
hash_groups = defaultdict(list)
images = []
videos = []

for current_root, dirs, files in os.walk(ROOT):
    dirs[:] = [d for d in dirs if d not in IGNORE_DIRS]
    dir_count += len(dirs)

    current_path = Path(current_root)

    for f in files:
        if f.lower() == "readme.md":
            readme_dirs.add(current_path)

    for f in files:
        file_count += 1
        full_path = current_path / f

        ext = full_path.suffix.lower() if full_path.suffix else "[no extension]"
        ext_counter[ext] += 1

        size = safe_stat_size(full_path)
        size_by_ext[ext] += size
        largest_files.append((size, full_path))

        if full_path.suffix.lower() in IMAGE_EXTS:
            images.append(full_path)
        if full_path.suffix.lower() in VIDEO_EXTS:
            videos.append(full_path)

        lower_name = f.lower()
        if any(keyword in lower_name for keyword in MESSY_KEYWORDS):
            messy_files.append(full_path)

        try:
            relative_parts = full_path.relative_to(ROOT).parts
            if len(relative_parts) > 0:
                top_folder = relative_parts[0]
                if ext in LIKELY_PROJECT_FILES:
                    candidate_project_dirs[top_folder] += 1
        except Exception:
            pass

        if size <= 50 * 1024 * 1024:
            h = file_hash(full_path)
            if h:
                hash_groups[h].append(full_path)

largest_files.sort(reverse=True, key=lambda x: x[0])

duplicate_groups = [paths for paths in hash_groups.values() if len(paths) > 1]
duplicate_groups.sort(key=lambda g: (-len(g), -sum(safe_stat_size(p) for p in g)))

print("=" * 80)
print("PLANT MONITOR REPO AUDIT")
print("=" * 80)
print(f"Root: {ROOT}")

print("\nFOLDER TREE (depth 3)")
print("-" * 80)
print(ROOT.name)
tree_lines = print_tree(ROOT, max_depth=MAX_TREE_DEPTH)
for line in tree_lines:
    print(line)

print("\nSUMMARY")
print("-" * 80)
print(f"Folders: {dir_count}")
print(f"Files:   {file_count}")

print("\nFILE TYPES")
print("-" * 80)
for ext, count in ext_counter.most_common():
    print(f"{ext:15} {count:6} files   {human_size(size_by_ext[ext])}")

print("\nLARGEST FILES")
print("-" * 80)
for size, path in largest_files[:TOP_N_LARGEST]:
    print(f"{human_size(size):>10}   {path}")

print("\nMESSY / REVIEW THESE")
print("-" * 80)
if messy_files:
    for path in messy_files[:100]:
        print(path)
else:
    print("No obviously messy file names found.")

print("\nPOSSIBLE DUPLICATES (same file content)")
print("-" * 80)
if duplicate_groups:
    shown = 0
    for group in duplicate_groups:
        print(f"\nDuplicate group ({len(group)} files):")
        for path in group[:10]:
            print(f"  {path}")
        if len(group) > 10:
            print("  ...")
        shown += 1
        if shown >= TOP_N_DUPLICATES:
            break
else:
    print("No duplicates found.")

print("\nIMAGE CANDIDATES")
print("-" * 80)
cover_candidates = choose_cover_images(images)
if cover_candidates:
    for img in cover_candidates:
        print(img.relative_to(ROOT))
else:
    print("No images found.")

print("\nTOP-LEVEL CONTENT")
print("-" * 80)
try:
    for item in sorted(ROOT.iterdir(), key=lambda p: (p.is_file(), p.name.lower())):
        if item.is_dir():
            file_total = sum(1 for x in item.rglob("*") if x.is_file())
            print(f"[DIR ] {item.name:35} {file_total:6} files")
        else:
            print(f"[FILE] {item.name:35} {human_size(safe_stat_size(item))}")
except Exception as e:
    print(f"Could not list top-level content: {e}")

print("\nSUGGESTED CLEANUP CHECKLIST")
print("-" * 80)
print("1. Keep only final, meaningful project folders at the repo root.")
print("2. Move extra experiments into an /archive folder or separate repo.")
print("3. Add README.md to the main folders that matter.")
print("4. Remove duplicate files and weirdly named copies.")
print("5. Move giant media/output files out of the repo if not needed.")
print("6. Add a .gitignore for build/cache/generated files.")
print("7. Keep firmware, sensors, docs, and media organized by folder.")
print("8. Make the repo root easy to understand in under 30 seconds.")

report_path = ROOT / "repo_audit_report.txt"
with open(report_path, "w", encoding="utf-8") as f:
    f.write("Plant Monitor Repo Audit completed successfully.\n")
    f.write(f"Root: {ROOT}\n")
    f.write(f"Folders: {dir_count}\n")
    f.write(f"Files: {file_count}\n")

if cover_candidates:
    image_html = []
    rel1 = cover_candidates[0].relative_to(ROOT).as_posix()
    image_html.append(f'  <img src="{rel1}" alt="Plant monitor cover" width="520">')
    if len(cover_candidates) >= 2:
        rel2 = cover_candidates[1].relative_to(ROOT).as_posix()
        image_html.append("  <br/><br/>")
        image_html.append(f'  <img src="{rel2}" alt="Plant monitor detail" width="420">')
    if len(cover_candidates) >= 3:
        rel3 = cover_candidates[2].relative_to(ROOT).as_posix()
        image_html.append(f'  <img src="{rel3}" alt="Plant monitor detail 2" width="420">')
    image_html.append("  <br/>")
    image_html.append("  <em>Plant monitoring system build and project images</em>")
    image_block = "\n".join(image_html)
else:
    image_block = "  <em>No images detected yet</em>"

repo_tree_block = "\n".join(tree_lines[:80]) if tree_lines else "└── README.md"

readme_lines = [
    "# Plant Monitoring System (ESP32-C3 Mini)",
    "",
    "<p align=\"center\">",
    image_block,
    "</p>",
    "",
    "An interactive plant monitoring system built around an **ESP32-C3 Mini** that helps care for a plant by reading multiple environmental sensors and giving clear, personality-driven feedback.",
    "",
    "This system is designed to sit beside a plant with the probe placed in the soil. It monitors:",
    "- **Soil moisture** using a capacitive soil sensor",
    "- **Temperature and humidity**",
    "- **Ambient light levels**",
    "- plant condition over time using animated and expressive feedback",
    "",
    "The goal is to make plant care feel less like guessing and more like a conversation. The device can be tuned for different plants and gives feedback through changing faces, animations, and messages depending on whether the plant is doing well or needs attention.",
    "",
    "It also supports **BLE** and **Telegram-style alert capability** for notifications and remote monitoring.",
    "",
    "---",
    "",
    "## Features",
    "",
    "- **ESP32-C3 Mini** based controller",
    "- **Capacitive soil moisture sensing**",
    "- **Temperature and humidity sensing**",
    "- **Light sensing**",
    "- Adjustable thresholds for different plant types",
    "- Visual feedback with:",
    "  - happy face / sad face states",
    "  - animations when the plant is doing well",
    "  - encouraging messages for good long-term care",
    "  - occasional quips when the caretaker is neglecting the plant",
    "- Notification support through **BLE / Telegram-style alerts**",
    "",
    "---",
    "",
    "## How It Works",
    "",
    "The monitor is placed next to a plant and powered on, while the soil probe is inserted into the potting soil. Sensor readings are combined to judge the plant’s condition.",
    "",
    "Examples:",
    "- If the soil is too dry, it tells you the plant needs water",
    "- If the temperature is too low, it warns that the plant may be too cold",
    "- If light levels stay low, it indicates the plant is not receiving enough light",
    "- If conditions stay healthy for a while, the system responds with positive animations and supportive messages",
    "",
    "Because plant needs vary, the system can be tuned for different species by adjusting sensor thresholds and care logic.",
    "",
    "---",
    "",
    "## Repository Layout",
    "",
    "```text",
    ".",
    repo_tree_block,
    "```",
    "",
    "---",
    "",
    "## Main Components",
    "",
    "- **Microcontroller:** ESP32-C3 Mini",
    "- **Soil sensor:** capacitive moisture sensor",
    "- **Environmental sensor:** temperature and humidity sensor",
    "- **Light sensor:** ambient light sensor",
    "- **Output/feedback:** animated faces, messages, and alerts",
    "",
    "---",
    "",
    "## Project Goal",
    "",
    "This project combines embedded systems, sensing, and user interaction to create a plant monitor that feels more alive than a normal sensor box. Instead of only showing raw data, it interprets conditions and reacts with personality.",
    "",
    "---",
    "",
    "## Status",
    "",
    "Working plant monitoring system with multi-sensor input, plant-state logic, expressive visual feedback, and notification capability.",
    "",
    "---",
    "",
    "## Notes",
    "",
    "This README was auto-generated from the project folder structure and should be refined after cleanup.",
    ""
]

readme_path = ROOT / "README_generated.md"
with open(readme_path, "w", encoding="utf-8") as f:
    f.write("\n".join(readme_lines))

print(f"\nSaved short report to: {report_path}")
print(f"Generated README draft: {readme_path}")