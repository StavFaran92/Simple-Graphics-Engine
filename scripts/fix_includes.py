import os
import re

# Adjusted for script location
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", "Engine"))

SOURCE_ROOTS = [
    os.path.join(ENGINE_ROOT, "src"),
    os.path.join(ENGINE_ROOT, "include")
]

FILE_EXTENSIONS = (".cpp", ".h", ".hpp", ".cxx")

INCLUDE_PATTERN = re.compile(r'#include\s+"([^"]+)"')

def build_header_map():
    header_map = {}

    for root in SOURCE_ROOTS:
        for dirpath, _, filenames in os.walk(root):
            for filename in filenames:
                if filename.endswith(".h"):
                    abs_path = os.path.join(dirpath, filename)

                    # Always strip the include *root* (either src/ or include/)
                    rel_path = os.path.relpath(abs_path, root).replace(os.sep, "/")

                    header_map[filename] = rel_path
    return header_map


def process_file(filepath, header_map):
    changed = False
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    with open(filepath, 'w', encoding='utf-8') as f:
        for line in lines:
            match = INCLUDE_PATTERN.search(line)
            if match:
                include_path = match.group(1)
                filename = os.path.basename(include_path)

                if filename in header_map:
                    correct_path = header_map[filename]

                    if include_path != correct_path:
                        print(f"{filepath}: {include_path} → {correct_path}")
                        line = line.replace(f'"{include_path}"', f'"{correct_path}"')
                        changed = True

            f.write(line)
    return changed

def main():
    header_map = build_header_map()
    print(f"Tracking {len(header_map)} headers...")

    for root in SOURCE_ROOTS:
        for dirpath, _, filenames in os.walk(root):
            for filename in filenames:
                if filename.endswith(FILE_EXTENSIONS):
                    filepath = os.path.join(dirpath, filename)
                    process_file(filepath, header_map)

if __name__ == "__main__":
    main()
