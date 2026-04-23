#!/usr/bin/env python3
"""
Live JSONL viewer for engine trace output (resource_view + scene_monitor events).

Usage:
    python tools/resource_viewer.py path/to/trace_....jsonl

Optional (efficient file watching):
    pip install -r tools/requirements.txt

Requires Python 3.8+ (stdlib only unless watchdog is installed).
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import sys
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, Tuple

# --- Optional watchdog -------------------------------------------------
try:
    from watchdog.events import FileSystemEventHandler
    from watchdog.observers import Observer

    _HAS_WATCHDOG = True
except ImportError:
    _HAS_WATCHDOG = False
    Observer = None  # type: ignore
    FileSystemEventHandler = object  # type: ignore

# --- Tkinter (stdlib) --------------------------------------------------
import tkinter as tk
from tkinter import ttk


@dataclass
class TraceState:
    """Merged view of resource_view and scene_monitor lines."""

    resources: dict[int, dict] = field(default_factory=dict)
    destroyed_resources: dict[int, dict] = field(default_factory=dict)
    last_added_resources: dict[int, dict] = field(default_factory=dict)
    last_added_frame: int = -1
    # scene_asset_id -> set of asset ids this scene depends on
    scene_deps: dict[int, set[int]] = field(default_factory=dict)
    # asset_id -> asset view payload (future-proof for additional asset data)
    assets: dict[int, dict] = field(default_factory=dict)
    parse_errors: int = 0

    def apply_line(self, obj: dict) -> None:
        system = obj.get("system")
        typ = obj.get("type")
        frame = obj.get("frame", 0)
        global_time = obj.get("global_time", 0)

        if system == "resource_view":
            rid = obj.get("id")
            if rid is not None and typ == "add_resource":
                rid_i = int(rid)
                frame_i = int(frame) if frame is not None else 0
                payload = {
                    "resource_type": obj.get("resource_type", ""),
                    "last_frame": frame_i,
                    "global_time": int(global_time) if global_time is not None else 0,
                }
                self.resources[rid_i] = payload
                if frame_i > self.last_added_frame:
                    self.last_added_frame = frame_i
                    self.last_added_resources.clear()
                    self.last_added_resources[rid_i] = payload
                elif frame_i == self.last_added_frame:
                    self.last_added_resources[rid_i] = payload
                self.destroyed_resources.pop(rid_i, None)
            elif rid is not None and typ == "destroy_resource":
                rid_i = int(rid)
                self.resources.pop(rid_i, None)
                self.last_added_resources.pop(rid_i, None)
                self.destroyed_resources[rid_i] = {
                    "resource_type": obj.get("resource_type", ""),
                    "last_frame": int(frame) if frame is not None else 0,
                    "global_time": int(global_time) if global_time is not None else 0,
                }
        elif system == "scene_monitor" and typ in {"add_dependency", "remove_dependency"}:
            scene = obj.get("scene")
            # Prefer explicit field, but support older traces that used "to".
            dep_asset_id = obj.get("asset_id", obj.get("to"))
            if scene is not None and dep_asset_id is not None:
                sid = int(scene)
                if sid not in self.scene_deps:
                    self.scene_deps[sid] = set()
                dep_id = int(dep_asset_id)
                if typ == "add_dependency":
                    self.scene_deps[sid].add(dep_id)
                else:
                    self.scene_deps[sid].discard(dep_id)
                    if not self.scene_deps[sid]:
                        self.scene_deps.pop(sid, None)
        elif system == "asset_view":
            frame_i = int(frame) if frame is not None else 0
            gtime_i = int(global_time) if global_time is not None else 0

            if typ == "add_asset":
                aid_raw = obj.get("id")
                if aid_raw is None:
                    return
                aid = int(aid_raw)
                existing = self.assets.get(aid, {})
                self.assets[aid] = {
                    "resource_id": existing.get("resource_id", ""),
                    "name": obj.get("name", ""),
                    "asset_type": obj.get("asset_type", ""),
                    "owner": obj.get("owner", ""),
                    "source": obj.get("source", ""),
                    "ext": obj.get("ext", ""),
                    "path": obj.get("path", ""),
                    "last_frame": frame_i,
                    "global_time": gtime_i,
                }
            elif typ == "bind_resource":
                aid_raw = obj.get("asset_id")
                rid = obj.get("resource_id")
                if aid_raw is None or rid is None:
                    return
                aid = int(aid_raw)
                existing = self.assets.get(aid, {})
                self.assets[aid] = {
                    "resource_id": int(rid),
                    "name": existing.get("name", ""),
                    "asset_type": existing.get("asset_type", ""),
                    "owner": existing.get("owner", ""),
                    "source": existing.get("source", ""),
                    "ext": existing.get("ext", ""),
                    "dir": existing.get("dir", ""),
                    "last_frame": frame_i,
                    "global_time": gtime_i,
                }


class JsonlTailReader:
    """Append-only JSONL: track byte offset and incomplete tail buffer."""

    def __init__(self, path: Path):
        self.path = path
        self._offset = 0
        self._carry = b""

    def reset_full(self) -> Tuple[bytes, int]:
        """Return full file content (bytes) and reset offset to end."""
        data = self.path.read_bytes()
        self._offset = len(data)
        self._carry = b""
        return data, self._offset

    def read_incremental(self) -> bytes:
        """Read new bytes since _offset; update offset to EOF."""
        with self.path.open("rb") as f:
            f.seek(self._offset)
            chunk = f.read()
        self._offset += len(chunk)
        return chunk

    def lines_from_buffer(self, data: bytes) -> list[str]:
        """Split (carry + data) on newlines; keep incomplete tail in carry."""
        combined = self._carry + data
        if not combined:
            return []
        parts = combined.split(b"\n")
        self._carry = parts.pop() if parts else b""
        out = []  # type: list[str]
        for p in parts:
            if p.strip():
                try:
                    out.append(p.decode("utf-8"))
                except UnicodeDecodeError:
                    out.append(p.decode("utf-8", errors="replace"))
        return out


class ResourceViewerApp:
    def __init__(self, path: Path):
        self.path = path.resolve()
        self.state = TraceState()
        self.reader = JsonlTailReader(self.path)
        self._last_mtime: float = 0.0
        self._last_size: int = 0
        self._watch_mode = "poll"
        self._observer: Optional[Observer] = None
        self._poll_after_id: Optional[str] = None

        self.root = tk.Tk()
        self.root.title("Resource viewer (JSONL)")
        self.root.geometry("1000x640")

        self._build_ui()
        self._process_full_reload()

    def _build_ui(self) -> None:
        top = ttk.Frame(self.root, padding=4)
        top.pack(fill=tk.X)
        ttk.Label(top, text="File:", width=6).pack(side=tk.LEFT)
        self.path_var = tk.StringVar(value=str(self.path))
        ttk.Entry(top, textvariable=self.path_var, state="readonly").pack(
            side=tk.LEFT, fill=tk.X, expand=True, padx=4
        )
        ttk.Button(top, text="Reload full file", command=self._on_reload_full).pack(
            side=tk.LEFT
        )

        paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # --- Assets view (same top level as Resources) ---
        lf_assets = ttk.LabelFrame(paned, text="Assets view", padding=4)
        paned.add(lf_assets, weight=1)
        asset_split = ttk.PanedWindow(lf_assets, orient=tk.VERTICAL)
        asset_split.pack(fill=tk.BOTH, expand=True)

        assets_top = ttk.Frame(asset_split)
        assets_bottom = ttk.LabelFrame(asset_split, text="Asset details", padding=4)
        asset_split.add(assets_top, weight=4)
        asset_split.add(assets_bottom, weight=1)

        self.tree_assets = ttk.Treeview(
            assets_top,
            columns=("asset_id", "asset_name", "resource_id", "frame", "global_time"),
            show="headings",
            height=20,
            selectmode=tk.BROWSE,
        )
        self.tree_assets.heading("asset_id", text="Asset ID")
        self.tree_assets.heading("asset_name", text="Asset Name")
        self.tree_assets.heading("resource_id", text="Bound Resource ID")
        self.tree_assets.heading("frame", text="Last frame")
        self.tree_assets.heading("global_time", text="Global time")
        self.tree_assets.column("asset_id", width=120, anchor=tk.E)
        self.tree_assets.column("asset_name", width=200, anchor=tk.W)
        self.tree_assets.column("resource_id", width=160, anchor=tk.E)
        self.tree_assets.column("frame", width=100, anchor=tk.E)
        self.tree_assets.column("global_time", width=180, anchor=tk.E)
        sy3 = ttk.Scrollbar(assets_top, orient=tk.VERTICAL, command=self.tree_assets.yview)
        self.tree_assets.configure(yscrollcommand=sy3.set)
        self.tree_assets.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        sy3.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree_assets.bind("<<TreeviewSelect>>", self._on_asset_selected)
        self.tree_assets.bind("<Double-1>", self._on_asset_double_click)
        try:
            self.tree_assets.configure(font=("Consolas", 10))
        except tk.TclError:
            pass

        self.asset_details_var = tk.StringVar(value="Select an asset to view details.")
        self.asset_details_label = ttk.Label(
            assets_bottom,
            textvariable=self.asset_details_var,
            anchor=tk.W,
            justify=tk.LEFT,
        )
        self.asset_details_label.pack(fill=tk.BOTH, expand=True)

        # --- Resource tabs (active / destroyed / last added) ---
        lf_res = ttk.LabelFrame(paned, text="Resources", padding=4)
        paned.add(lf_res, weight=1)
        self.tabs = ttk.Notebook(lf_res)
        self.tabs.pack(fill=tk.BOTH, expand=True)

        self.tree_res_active = self._build_resource_tree(self.tabs, "Active resources")
        self.tree_res_destroyed = self._build_resource_tree(self.tabs, "Destroyed resources")
        self.tree_res_last_added = self._build_resource_tree(self.tabs, "Last added resources")

        # --- Scene dependencies (parent = scene, children = asset ids) ---
        lf_scene = ttk.LabelFrame(paned, text="Scene dependencies", padding=4)
        paned.add(lf_scene, weight=1)
        self.tree_scene = ttk.Treeview(
            lf_scene, columns=("asset_id",), show="tree headings", height=20
        )
        self.tree_scene.heading("#0", text="Scene / Asset")
        self.tree_scene.heading("asset_id", text="Asset ID")
        self.tree_scene.column("#0", width=200, anchor=tk.W)
        self.tree_scene.column("asset_id", width=120, anchor=tk.E)
        sy2 = ttk.Scrollbar(lf_scene, orient=tk.VERTICAL, command=self.tree_scene.yview)
        self.tree_scene.configure(yscrollcommand=sy2.set)
        self.tree_scene.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        sy2.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree_scene.bind("<Double-1>", self._on_scene_dependency_double_click)

        self.status = ttk.Label(self.root, text="", relief=tk.SUNKEN, anchor=tk.W)
        self.status.pack(fill=tk.X, side=tk.BOTTOM, padx=4, pady=2)

    def _build_resource_tree(self, parent: ttk.Notebook, title: str) -> ttk.Treeview:
        frame = ttk.Frame(parent)
        parent.add(frame, text=title)
        cols = ("id", "type", "frame", "global_time")
        tree = ttk.Treeview(
            frame, columns=cols, show="headings", height=20, selectmode=tk.BROWSE
        )
        tree.heading("id", text="Resource ID")
        tree.heading("type", text="Type")
        tree.heading("frame", text="Last frame")
        tree.heading("global_time", text="Global time")
        tree.column("id", width=100, anchor=tk.E)
        tree.column("type", width=220, anchor=tk.W)
        tree.column("frame", width=100, anchor=tk.E)
        tree.column("global_time", width=180, anchor=tk.E)
        sy = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=tree.yview)
        tree.configure(yscrollcommand=sy.set)
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        sy.pack(side=tk.RIGHT, fill=tk.Y)
        try:
            tree.configure(font=("Consolas", 10))
        except tk.TclError:
            pass
        return tree

    def _parse_and_apply_lines(self, text_lines: list[str], reset_state: bool) -> None:
        if reset_state:
            self.state = TraceState()
            self.reader = JsonlTailReader(self.path)
        for line in text_lines:
            line = line.strip()
            if not line:
                continue
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                self.state.parse_errors += 1
                continue
            if isinstance(obj, dict):
                self.state.apply_line(obj)

    def _process_full_reload(self) -> None:
        if not self.path.is_file():
            self._set_status("File not found.")
            return
        data, _ = self.reader.reset_full()
        self._last_mtime = self.path.stat().st_mtime
        self._last_size = self.path.stat().st_size
        lines = []
        for raw in data.split(b"\n"):
            if raw.strip():
                try:
                    lines.append(raw.decode("utf-8"))
                except UnicodeDecodeError:
                    lines.append(raw.decode("utf-8", errors="replace"))
        self._parse_and_apply_lines(lines, reset_state=True)
        self._refresh_trees()
        self._set_status(f"Full reload: {len(lines)} lines.")

    def _process_incremental(self) -> None:
        if not self.path.is_file():
            return
        chunk = self.reader.read_incremental()
        if not chunk:
            return
        lines = self.reader.lines_from_buffer(chunk)
        n_err_before = self.state.parse_errors
        for line in lines:
            line = line.strip()
            if not line:
                continue
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                self.state.parse_errors += 1
                continue
            if isinstance(obj, dict):
                self.state.apply_line(obj)
        if lines or self.state.parse_errors != n_err_before:
            self._refresh_trees()
        self._set_status(
            f"Updated (+{len(lines)} lines) | parse errors: {self.state.parse_errors} | mode: {self._watch_mode}"
        )

    def _on_reload_full(self) -> None:
        self.path = Path(self.path_var.get()).resolve()
        self.reader = JsonlTailReader(self.path)
        self._process_full_reload()

    def _refresh_trees(self) -> None:
        self._refresh_resource_tree(self.tree_res_active, self.state.resources)
        self._refresh_resource_tree(self.tree_res_destroyed, self.state.destroyed_resources)
        self._refresh_resource_tree(self.tree_res_last_added, self.state.last_added_resources)
        self._refresh_assets_tree()

        for item in self.tree_scene.get_children():
            self.tree_scene.delete(item)
        for sid in sorted(self.state.scene_deps.keys()):
            deps = self.state.scene_deps[sid]
            parent = self.tree_scene.insert(
                "",
                tk.END,
                text=f"Scene asset {sid}",
                values=("",),
                open=True,
            )
            for rid in sorted(deps):
                self.tree_scene.insert(
                    parent,
                    tk.END,
                    text=f"  -> asset {rid}",
                    values=(rid,),
                )

    def _refresh_assets_tree(self) -> None:
        for item in self.tree_assets.get_children():
            self.tree_assets.delete(item)
        for aid in sorted(
            self.state.assets.keys(),
            key=lambda aid: (self.state.assets[aid].get("global_time", 0), aid),
            reverse=True,
        ):
            a = self.state.assets[aid]
            asset_name = a.get("name", "")
            rid = a.get("resource_id", "")
            self.tree_assets.insert(
                "",
                tk.END,
                values=(
                    aid,
                    asset_name,
                    rid,
                    a.get("last_frame", ""),
                    self._format_global_time(a.get("global_time", 0)),
                ),
            )

    def _on_asset_selected(self, _event=None) -> None:
        selection = self.tree_assets.selection()
        if not selection:
            self.asset_details_var.set("Select an asset to view details.")
            return
        item = self.tree_assets.item(selection[0])
        values = item.get("values", [])
        if len(values) < 5:
            self.asset_details_var.set("Select an asset to view details.")
            return
        try:
            asset_id = int(values[0])
        except (TypeError, ValueError):
            self.asset_details_var.set("Select an asset to view details.")
            return
        asset = self.state.assets.get(asset_id, {})
        self.asset_details_var.set(
            f"Asset ID: {values[0]}\n"
            f"Name: {values[1] or '-'}\n"
            f"Type: {asset.get('asset_type', '') or '-'}\n"
            f"Owner: {asset.get('owner', '') or '-'}\n"
            f"Source: {asset.get('source', '') or '-'}\n"
            f"Ext: {asset.get('ext', '') or '-'}\n"
            f"Path: {asset.get('path', '') or '-'}\n"
            f"Bound Resource ID: {values[2]}\n"
            f"Last frame: {values[3]}\n"
            f"Global time: {values[4]}"
        )

    def _on_asset_double_click(self, event) -> None:
        row_id = self.tree_assets.identify_row(event.y)
        if not row_id:
            return
        item = self.tree_assets.item(row_id)
        values = item.get("values", [])
        if len(values) < 3:
            return
        try:
            resource_id = int(values[2])
        except (TypeError, ValueError):
            self._set_status("Selected asset has no bound resource.")
            return
        self._select_active_resource_in_view(resource_id)

    def _on_scene_dependency_double_click(self, event) -> None:
        row_id = self.tree_scene.identify_row(event.y)
        if not row_id:
            return
        item = self.tree_scene.item(row_id)
        values = item.get("values", [])
        if not values:
            return
        try:
            asset_id = int(values[0])
        except (TypeError, ValueError):
            return
        self._select_asset_in_view(asset_id)

    def _select_asset_in_view(self, asset_id: int) -> None:
        for item_id in self.tree_assets.get_children():
            item = self.tree_assets.item(item_id)
            values = item.get("values", [])
            if not values:
                continue
            try:
                current_asset_id = int(values[0])
            except (TypeError, ValueError):
                continue
            if current_asset_id == asset_id:
                self.tree_assets.selection_set(item_id)
                self.tree_assets.focus(item_id)
                self.tree_assets.see(item_id)
                self._on_asset_selected()
                self._set_status(f"Selected asset {asset_id} from scene dependency.")
                return
        self._set_status(f"Asset {asset_id} not found in assets view.")

    def _select_active_resource_in_view(self, resource_id: int) -> None:
        for item_id in self.tree_res_active.get_children():
            item = self.tree_res_active.item(item_id)
            values = item.get("values", [])
            if not values:
                continue
            try:
                current_resource_id = int(values[0])
            except (TypeError, ValueError):
                continue
            if current_resource_id == resource_id:
                self.tabs.select(self.tree_res_active.master)
                self.tree_res_active.selection_set(item_id)
                self.tree_res_active.focus(item_id)
                self.tree_res_active.see(item_id)
                self._set_status(
                    f"Selected active resource {resource_id} from asset binding."
                )
                return
        self._set_status(f"Resource {resource_id} is not active.")

    def _refresh_resource_tree(self, tree: ttk.Treeview, data: dict[int, dict]) -> None:
        for item in tree.get_children():
            tree.delete(item)
        for rid in sorted(
            data.keys(),
            key=lambda rid: (data[rid].get("global_time", 0), rid),
            reverse=True,
        ):
            r = data[rid]
            tree.insert(
                "",
                tk.END,
                values=(
                    rid,
                    r.get("resource_type", ""),
                    r.get("last_frame", ""),
                    self._format_global_time(r.get("global_time", 0)),
                ),
            )

    @staticmethod
    def _format_global_time(global_time_us: int) -> str:
        try:
            us = int(global_time_us)
            if us <= 0:
                return ""
            ts = us / 1_000_000.0
            dt_obj = dt.datetime.fromtimestamp(ts)
            return dt_obj.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        except (TypeError, ValueError, OSError, OverflowError):
            return str(global_time_us)

    def _set_status(self, msg: str) -> None:
        ts = time.strftime("%H:%M:%S")
        self.status.config(text=f"[{ts}] {msg}")

    def _schedule_gui_refresh(self) -> None:
        def tick():
            try:
                self._process_incremental()
            except OSError as e:
                self._set_status(f"Read error: {e}")
            self._touch_stat()

        self.root.after_idle(lambda: self.root.after(0, tick))

    def _touch_stat(self) -> None:
        st = self.path.stat()
        self._set_status(
            f"size={st.st_size} B | parse errors: {self.state.parse_errors} | mode: {self._watch_mode}"
        )

    def _poll_file(self) -> None:
        try:
            st = self.path.stat()
        except OSError:
            self._poll_after_id = self.root.after(400, self._poll_file)
            return
        if st.st_mtime != self._last_mtime or st.st_size != self._last_size:
            self._last_mtime = st.st_mtime
            self._last_size = st.st_size
            self._schedule_gui_refresh()
        self._poll_after_id = self.root.after(400, self._poll_file)

    def _start_watchdog(self) -> bool:
        if not _HAS_WATCHDOG or Observer is None:
            return False

        class _Handler(FileSystemEventHandler):
            def __init__(self, app: "ResourceViewerApp"):
                super().__init__()
                self.app = app

            def on_modified(self, event):  # type: ignore[override]
                if getattr(event, "is_directory", False):
                    return
                try:
                    if Path(event.src_path).resolve() == self.app.path:
                        self.app._last_mtime = time.time()
                        self.app._schedule_gui_refresh()
                except OSError:
                    pass

        try:
            self._observer = Observer()
            handler = _Handler(self)
            watch_dir = str(self.path.parent)
            self._observer.schedule(handler, watch_dir, recursive=False)
            self._observer.start()
            self._watch_mode = "watchdog"
            return True
        except Exception:
            self._observer = None
            return False

    def run(self) -> None:
        if _HAS_WATCHDOG and self._start_watchdog():
            self._set_status("Watching with watchdog.")
        else:
            self._watch_mode = "poll"
            self._poll_file()
            self._set_status("Polling file (install watchdog for lower CPU).")

        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        self.root.mainloop()

    def _on_close(self) -> None:
        if self._poll_after_id is not None:
            try:
                self.root.after_cancel(self._poll_after_id)
            except tk.TclError:
                pass
        if self._observer is not None:
            self._observer.stop()
            self._observer.join(timeout=2.0)
        self.root.destroy()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Live viewer for trace JSONL (resource_view + scene_monitor)."
    )
    parser.add_argument(
        "jsonl",
        type=Path,
        nargs="?",
        help="Path to the .jsonl trace file (optional; if omitted, uses latest .jsonl in tools folder).",
    )
    args = parser.parse_args()
    if args.jsonl is None:
        tools_dir = Path(__file__).resolve().parent
        repo_root = tools_dir.parent
        fallback_dir = repo_root / "build" / "EditorApp" / "logs"

        candidates = (
            [p for p in fallback_dir.glob("*.jsonl") if p.is_file()]
            if fallback_dir.is_dir()
            else []
        )
        if not candidates:
            print(
                f"Error: no .jsonl files found in logs folder: {fallback_dir}",
                file=sys.stderr,
            )
            return 1
        path = max(candidates, key=lambda p: p.stat().st_mtime)
    else:
        path = args.jsonl
        if not path.is_file():
            print(f"Error: not a file or does not exist: {path}", file=sys.stderr)
            return 1

    app = ResourceViewerApp(path)
    app.run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
