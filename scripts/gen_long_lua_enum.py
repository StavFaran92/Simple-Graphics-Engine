import re
import os

src = "../Engine/src/ui/KeyCodes.h"

# Create output filename next to source (e.g., "KeyCodes_Lua.cpp")
base, _ = os.path.splitext(src)
dst = base + "_Lua.cpp"

with open(src) as f, open(dst, "w") as out:
    for line in f:
        # Match an enum name like "SCANCODE_D = 7," or "SCANCODE_D,"
        match = re.match(r'\s*([A-Za-z_][A-Za-z0-9_]*)', line)
        if match and not line.strip().startswith("//"):
            name = match.group(1)
            out.write(f'    "{name}", KeyCode::{name},\n')

print(f"Generated: {dst}")
