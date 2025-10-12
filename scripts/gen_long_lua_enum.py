import re
import os

def generate_enum_lua(src, func_name, enum_name):
    # Determine output path next to source (.gen.h)
    base, _ = os.path.splitext(src)
    dst = base + "_Lua.gen.h"

    # Safely read file (ignore invalid UTF-8 bytes)
    with open(src, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    entries = []
    for line in lines:
        line = line.strip()

        # Skip empty lines, comments, braces, enum declarations, or closing braces
        if (not line 
            or line.startswith("//") 
            or line.startswith("enum") 
            or "{" in line 
            or "}" in line):
            continue

        # Match entries like "SCANCODE_A = 4," or "SCANCODE_A,"
        match = re.match(r'([A-Za-z_][A-Za-z0-9_]*)', line)
        if match:
            name = match.group(1)
            entries.append(f'        "{name}", {enum_name}::{name}')

    # Write output header
    with open(dst, "w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write(f'#include "{os.path.basename(src)}"\n')
        out.write("#include <sol/sol.hpp>\n\n")
        out.write(f"inline void {func_name}(sol::state& lua)\n")
        out.write("{\n")
        out.write(f"    lua.new_enum(\"{enum_name}\",\n")
        out.write(",\n".join(entries))
        out.write("\n    );\n")
        out.write("}\n")

    print(f"Generated header: {dst}")


# Example usage:
generate_enum_lua("../Engine/src/ui/KeyCodes.h", "loadKeyCodes", "KeyCode")
