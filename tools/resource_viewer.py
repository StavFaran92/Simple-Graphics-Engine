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
    # scene_asset_id -> set of resource ids this scene depends on
    scene_deps: dict[int, set[int]] = field(default_factory=dict)
    parse_errors: int = 0

    def apply_line(self, obj: dict) -> None:
        system = obj.get("system")
        typ = obj.get("type")
        frame = obj.get("frame", 0)

        if system == "resource_view" and typ == "add_resource":
            rid = obj.get("id")
            if rid is not None:
                self.resources[int(rid)] = {
                    "resource_type": obj.get("resource_type", ""),
                    "last_frame": int(frame) if frame is not None else 0,
                }
        elif system == "scene_monitor" and typ == "add_dependency":
            scene = obj.get("scene")
            to_id = obj.get("to")
            if scene is not None and to_id is not None:
                sid = int(scene)
                if sid not in self.scene_deps:
                    self.scene_deps[sid] = set()
                self.scene_deps[sid].add(int(to_id))


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

        # --- Active resources ---
        lf_res = ttk.LabelFrame(paned, text="Active resources", padding=4)
        paned.add(lf_res, weight=1)
        cols = ("id", "type", "frame")
        self.tree_res = ttk.Treeview(
            lf_res, columns=cols, show="headings", height=20, selectmode=tk.BROWSE
        )
        self.tree_res.heading("id", text="Resource ID")
        self.tree_res.heading("type", text="Type")
        self.tree_res.heading("frame", text="Last frame")
        self.tree_res.column("id", width=100, anchor=tk.E)
        self.tree_res.column("type", width=280, anchor=tk.W)
        self.tree_res.column("frame", width=100, anchor=tk.E)
        sy1 = ttk.Scrollbar(lf_res, orient=tk.VERTICAL, command=self.tree_res.yview)
        self.tree_res.configure(yscrollcommand=sy1.set)
        self.tree_res.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        sy1.pack(side=tk.RIGHT, fill=tk.Y)

        try:
            self.tree_res.configure(font=("Consolas", 10))
        except tk.TclError:
            pass

        # --- Scene dependencies (parent = scene, children = resource ids) ---
        lf_scene = ttk.LabelFrame(paned, text="Scene asset dependencies", padding=4)
        paned.add(lf_scene, weight=1)
        self.tree_scene = ttk.Treeview(
            lf_scene, columns=("rid",), show="tree headings", height=20
        )
        self.tree_scene.heading("#0", text="Scene / Resource")
        self.tree_scene.heading("rid", text="Resource ID")
        self.tree_scene.column("#0", width=200, anchor=tk.W)
        self.tree_scene.column("rid", width=120, anchor=tk.E)
        sy2 = ttk.Scrollbar(lf_scene, orient=tk.VERTICAL, command=self.tree_scene.yview)
        self.tree_scene.configure(yscrollcommand=sy2.set)
        self.tree_scene.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        sy2.pack(side=tk.RIGHT, fill=tk.Y)

        self.status = ttk.Label(self.root, text="", relief=tk.SUNKEN, anchor=tk.W)
        self.status.pack(fill=tk.X, side=tk.BOTTOM, padx=4, pady=2)

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
        for item in self.tree_res.get_children():
            self.tree_res.delete(item)
        for rid in sorted(self.state.resources.keys()):
            r = self.state.resources[rid]
            self.tree_res.insert(
                "",
                tk.END,
                values=(rid, r.get("resource_type", ""), r.get("last_frame", "")),
            )

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
                    text=f"  → resource {rid}",
                    values=(rid,),
                )

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
        help="Path to the .jsonl trace file (must exist).",
    )
    args = parser.parse_args()
    path: Path = args.jsonl
    if not path.is_file():
        print(f"Error: not a file or does not exist: {path}", file=sys.stderr)
        return 1

    app = ResourceViewerApp(path)
    app.run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
