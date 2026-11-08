"""One-off: rewrites model records in ProjectAssetRegistry.json from the old nlohmann shape
([[slot, {"uuid": N}]]) to cereal's ([{"key": slot, "value": {"uuid": {"m_value": N}}}]).

Usage: python scripts/convert_model_registry.py [registry.json ...]
With no arguments it converts every samples/*/ProjectAssetRegistry.json.
"""
import glob
import json
import sys


def convert_materials(materials):
    converted = []
    for entry in materials:
        if isinstance(entry, dict):
            converted.append(entry)  # already cereal
            continue
        slot, ref = entry
        uuid = ref["uuid"]
        if not isinstance(uuid, dict):
            uuid = {"m_value": uuid}
        converted.append({"key": slot, "value": {"uuid": uuid}})
    return converted


def convert(path):
    with open(path, encoding="utf-8") as f:
        registry = json.load(f)

    changed = 0
    for record in registry.get("models", []):
        asset = record.get("asset", {})
        if "materials" in asset:
            before = asset["materials"]
            asset["materials"] = convert_materials(before)
            changed += asset["materials"] != before

    if changed:
        # Same layout nlohmann writes: 4-space indent, sorted keys
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            json.dump(registry, f, indent=4, sort_keys=True, ensure_ascii=False)
            f.write("\n")
    print(f"{path}: {changed} model record(s) converted")


paths = sys.argv[1:] or glob.glob("samples/*/ProjectAssetRegistry.json")
for p in paths:
    convert(p)
