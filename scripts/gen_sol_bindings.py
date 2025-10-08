import os
import re

input_dir = "../Engine/src"  # Update this to your source directory
output_file = "lua_bindings.cpp"

class_pattern = re.compile(r'class\s+EngineAPI\s+(\w+)\s*[{:]')
method_pattern = re.compile(
    r'\b([a-zA-Z_][\w:<>\s*&]+?)\s+([a-zA-Z_]\w*)\s*\(([^)]*)\)\s*(const)?\s*[;{]'
)
access_specifier_pattern = re.compile(r'^\s*(public|private|protected):')
comment_pattern = re.compile(r'^\s*//')

def parse_class_methods(filepath):
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as file:
        lines = file.readlines()

    in_class = False
    class_name = None
    brace_depth = 0
    current_access = "private"
    classes = {}

    for line in lines:
        if comment_pattern.match(line):
            continue

        class_match = class_pattern.search(line)
        if class_match:
            class_name = class_match.group(1)
            classes[class_name] = []
            in_class = True
            brace_depth = 0
            current_access = "private"
            continue

        if in_class:
            brace_depth += line.count('{') - line.count('}')
            access_match = access_specifier_pattern.match(line)
            if access_match:
                current_access = access_match.group(1)
                continue

            if current_access == "public":
                method_match = method_pattern.search(line)
                if method_match:
                    ret_type = method_match.group(1).strip()
                    method_name = method_match.group(2).strip()
                    arglist = method_match.group(3).strip()
                    is_const = method_match.group(4) is not None
                    classes[class_name].append((ret_type, method_name, arglist, is_const))

            if brace_depth < 0:
                in_class = False
                class_name = None

    return classes

def generate_sol3_bindings(classes):
    lines = []
    lines.append("// Auto-generated Lua bindings using Sol3")
    lines.append('#include <sol/sol.hpp>')
    lines.append("#include \"your_engine_includes.h\"  // Update this")
    lines.append("")
    lines.append("void BindAllToLua(sol::state& lua) {")
    lines.append("")

    for class_name, methods in classes.items():
        lines.append(f'    lua.new_usertype<{class_name}>("{class_name}",')
        method_lines = []
        for _, method_name, _, _ in methods:
            method_lines.append(f'        "{method_name}", &{class_name}::{method_name}')
        lines.append(",\n".join(method_lines) + "\n    );\n")

    lines.append("}")
    return "\n".join(lines)

def main():
    all_classes = {}

    for root, _, files in os.walk(input_dir):
        for file in files:
            if file.endswith(('.h', '.cpp')):
                path = os.path.join(root, file)
                class_data = parse_class_methods(path)
                all_classes.update(class_data)

    cpp_code = generate_sol3_bindings(all_classes)

    with open(output_file, 'w') as f:
        f.write(cpp_code)

    print(f"Sol3 bindings written to {output_file}")

if __name__ == "__main__":
    main()
