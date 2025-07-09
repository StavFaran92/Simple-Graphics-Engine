import os
import re

# Folder where your headers and sources live
ROOT_DIR = "../Engine/src"

# Extensions to process
FILE_EXTENSIONS = (".cpp", ".h", ".hpp", ".cxx")

# Regex to find flat includes
INCLUDE_PATTERN = re.compile(r'#include\s+"([\w\d_]+\.h)"')

def build_header_map():
    header_map = {}
    for dirpath, _, filenames in os.walk(ROOT_DIR):
        for filename in filenames:
            if filename.endswith(".h"):
                rel_path = os.path.relpath(os.path.join(dirpath, filename), ROOT_DIR)
                header_map[filename] = rel_path.replace(os.sep, "/")
    return header_map

def process_file(filepath, header_map):
    changed = False
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    with open(filepath, 'w', encoding='utf-8') as f:
        for line in lines:
            match = INCLUDE_PATTERN.search(line)
            if match:
                header = match.group(1)
                if header in header_map:
                    new_path = header_map[header]
                    if f'"{new_path}"' not in line:
                        print(f"Updating include in {filepath}: {line.strip()} → {new_path}")
                        line = line.replace(f'"{header}"', f'"{new_path}"')
                        changed = True
            f.write(line)

    return changed

def main():
    header_map = build_header_map()
    print(f"Found {len(header_map)} headers.")

    for dirpath, _, filenames in os.walk(ROOT_DIR):
        for filename in filenames:
            if filename.endswith(FILE_EXTENSIONS):
                process_file(os.path.join(dirpath, filename), header_map)

if __name__ == "__main__":
    main()