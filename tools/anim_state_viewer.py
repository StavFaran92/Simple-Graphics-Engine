#!/usr/bin/env python3
"""
Live JSONL viewer for engine animation graph trace output (anim_graph events).

Shows the current animation state of every entity, and the state history of the selected entity.

Usage:
    python tools/anim_state_viewer.py path/to/trace_....jsonl

Requires Python 3.8+ (stdlib only).
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

import tkinter as tk
from tkinter import ttk

from resource_viewer import JsonlTailReader

POLL_INTERVAL_MS = 300


@dataclass
class EntityAnimState:
    current_state: str = ""
    previous_state: str = ""
    transitions: int = 0
    last_frame: int = 0
    global_time: int = 0
    # (frame, global_time, event_type, from, to)
    history: list[tuple[int, int, str, str, str]] = field(default_factory=list)


@dataclass
class AnimTraceState:
    entities: dict[int, EntityAnimState] = field(default_factory=dict)
    parse_errors: int = 0

    def apply_line(self, obj: dict) -> None:
        if obj.get("system") != "anim_graph":
            return

        entity_raw = obj.get("entity")
        if entity_raw is None:
            return
        entity = int(entity_raw)
        frame = int(obj.get("frame", 0) or 0)
        global_time = int(obj.get("global_time", 0) or 0)
        typ = obj.get("type")

        if typ == "init":
            entry_state = obj.get("entry_state", "")
            # Entity ids are reused between simulation runs, init starts a fresh record
            state = EntityAnimState(current_state=entry_state)
            state.history.append((frame, global_time, "init", "", entry_state))
        elif typ == "enter_state":
            state = self.entities.setdefault(entity, EntityAnimState())
            from_state = obj.get("from", "")
            to_state = obj.get("to", "")
            state.previous_state = from_state
            state.current_state = to_state
            state.transitions += 1
            state.history.append((frame, global_time, "enter_state", from_state, to_state))
        else:
            return

        state.last_frame = frame
        state.global_time = global_time
        self.entities[entity] = state


class AnimStateViewerApp:
    def __init__(self, path: Path):
        self.path = path.resolve()
        self.state = AnimTraceState()
        self.reader = JsonlTailReader(self.path)
        self._last_size = -1
        self._poll_after_id: Optional[str] = None

        self.root = tk.Tk()
        self.root.title("Animation state viewer (JSONL)")
        self.root.geometry("900x620")

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
        ttk.Button(top, text="Reload full file", command=self._process_full_reload).pack(
            side=tk.LEFT
        )

        filter_row = ttk.Frame(self.root, padding=(4, 0))
        filter_row.pack(fill=tk.X)
        ttk.Label(filter_row, text="Filter:", width=6).pack(side=tk.LEFT)
        self.filter_var = tk.StringVar()
        self.filter_var.trace_add("write", lambda *_: self._refresh_entities_tree())
        ttk.Entry(filter_row, textvariable=self.filter_var, width=30).pack(side=tk.LEFT, padx=4)
        ttk.Label(filter_row, text="(entity id or state name)").pack(side=tk.LEFT)

        split = ttk.PanedWindow(self.root, orient=tk.VERTICAL)
        split.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # --- Entities (current state per entity) ---
        lf_entities = ttk.LabelFrame(split, text="Entities", padding=4)
        split.add(lf_entities, weight=3)
        self.tree_entities = self._build_tree(
            lf_entities,
            [
                ("entity", "Entity", 80, tk.E),
                ("current", "Current state", 180, tk.W),
                ("previous", "Previous state", 180, tk.W),
                ("transitions", "Transitions", 90, tk.E),
                ("frame", "Last change frame", 130, tk.E),
                ("global_time", "Last change time", 180, tk.E),
            ],
        )
        self.tree_entities.bind("<<TreeviewSelect>>", lambda _e: self._refresh_history_tree())

        # --- History of selected entity ---
        self.lf_history = ttk.LabelFrame(split, text="History", padding=4)
        split.add(self.lf_history, weight=2)
        self.tree_history = self._build_tree(
            self.lf_history,
            [
                ("frame", "Frame", 80, tk.E),
                ("global_time", "Time", 180, tk.E),
                ("event", "Event", 100, tk.W),
                ("from", "From", 180, tk.W),
                ("to", "To", 180, tk.W),
            ],
        )

        self.status = ttk.Label(self.root, text="", relief=tk.SUNKEN, anchor=tk.W)
        self.status.pack(fill=tk.X, side=tk.BOTTOM, padx=4, pady=2)

    @staticmethod
    def _build_tree(parent: ttk.Frame, columns: list[tuple[str, str, int, str]]) -> ttk.Treeview:
        tree = ttk.Treeview(
            parent, columns=[c[0] for c in columns], show="headings", selectmode=tk.BROWSE
        )
        for col_id, title, width, anchor in columns:
            tree.heading(col_id, text=title)
            tree.column(col_id, width=width, anchor=anchor)
        sy = ttk.Scrollbar(parent, orient=tk.VERTICAL, command=tree.yview)
        tree.configure(yscrollcommand=sy.set)
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        sy.pack(side=tk.RIGHT, fill=tk.Y)
        try:
            tree.configure(font=("Consolas", 10))
        except tk.TclError:
            pass
        return tree

    def _apply_lines(self, lines: list[str]) -> None:
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

    def _process_full_reload(self) -> None:
        if not self.path.is_file():
            self._set_status("File not found.")
            return
        self.state = AnimTraceState()
        self.reader = JsonlTailReader(self.path)
        data, size = self.reader.reset_full()
        self._last_size = size
        lines = [
            raw.decode("utf-8", errors="replace") for raw in data.split(b"\n") if raw.strip()
        ]
        self._apply_lines(lines)
        self._refresh_trees()
        self._set_status(f"Full reload: {len(lines)} lines.")

    def _poll_file(self) -> None:
        try:
            size = self.path.stat().st_size
            if size != self._last_size:
                self._last_size = size
                lines = self.reader.lines_from_buffer(self.reader.read_incremental())
                if lines:
                    self._apply_lines(lines)
                    self._refresh_trees()
                self._set_status(
                    f"Entities: {len(self.state.entities)} | parse errors: {self.state.parse_errors}"
                )
        except OSError as e:
            self._set_status(f"Read error: {e}")
        self._poll_after_id = self.root.after(POLL_INTERVAL_MS, self._poll_file)

    def _refresh_trees(self) -> None:
        self._refresh_entities_tree()
        self._refresh_history_tree()

    def _matches_filter(self, entity: int, state: EntityAnimState) -> bool:
        text = self.filter_var.get().strip().lower()
        if not text:
            return True
        return (
            text == str(entity)
            or text in state.current_state.lower()
            or text in state.previous_state.lower()
        )

    def _selected_entity(self) -> Optional[int]:
        selection = self.tree_entities.selection()
        if not selection:
            return None
        return int(selection[0])

    def _refresh_entities_tree(self) -> None:
        selected = self._selected_entity()
        for item in self.tree_entities.get_children():
            self.tree_entities.delete(item)

        for entity in sorted(self.state.entities.keys()):
            state = self.state.entities[entity]
            if not self._matches_filter(entity, state):
                continue
            self.tree_entities.insert(
                "",
                tk.END,
                iid=str(entity),
                values=(
                    entity,
                    state.current_state,
                    state.previous_state,
                    state.transitions,
                    state.last_frame,
                    self._format_global_time(state.global_time),
                ),
            )

        if selected is not None and self.tree_entities.exists(str(selected)):
            self.tree_entities.selection_set(str(selected))

    def _refresh_history_tree(self) -> None:
        for item in self.tree_history.get_children():
            self.tree_history.delete(item)

        entity = self._selected_entity()
        if entity is None or entity not in self.state.entities:
            self.lf_history.configure(text="History (select an entity)")
            return

        self.lf_history.configure(text=f"History - entity {entity}")
        # Newest first
        for frame, global_time, event, from_state, to_state in reversed(
            self.state.entities[entity].history
        ):
            self.tree_history.insert(
                "",
                tk.END,
                values=(frame, self._format_global_time(global_time), event, from_state, to_state),
            )

    @staticmethod
    def _format_global_time(global_time_us: int) -> str:
        try:
            us = int(global_time_us)
            if us <= 0:
                return ""
            return dt.datetime.fromtimestamp(us / 1_000_000.0).strftime("%H:%M:%S.%f")[:-3]
        except (TypeError, ValueError, OSError, OverflowError):
            return str(global_time_us)

    def _set_status(self, msg: str) -> None:
        ts = time.strftime("%H:%M:%S")
        self.status.config(text=f"[{ts}] {msg}")

    def run(self) -> None:
        self._poll_file()
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        self.root.mainloop()

    def _on_close(self) -> None:
        if self._poll_after_id is not None:
            try:
                self.root.after_cancel(self._poll_after_id)
            except tk.TclError:
                pass
        self.root.destroy()


def main() -> int:
    parser = argparse.ArgumentParser(description="Live viewer for animation graph trace JSONL.")
    parser.add_argument(
        "jsonl",
        type=Path,
        nargs="?",
        help="Path to the .jsonl trace file (optional; if omitted, uses latest .jsonl in build/EditorApp/logs).",
    )
    args = parser.parse_args()
    if args.jsonl is None:
        logs_dir = Path(__file__).resolve().parent.parent / "build" / "EditorApp" / "logs"
        candidates = [p for p in logs_dir.glob("*.jsonl") if p.is_file()] if logs_dir.is_dir() else []
        if not candidates:
            print(f"Error: no .jsonl files found in logs folder: {logs_dir}", file=sys.stderr)
            return 1
        path = max(candidates, key=lambda p: p.stat().st_mtime)
    else:
        path = args.jsonl
        if not path.is_file():
            print(f"Error: not a file or does not exist: {path}", file=sys.stderr)
            return 1

    app = AnimStateViewerApp(path)
    app.run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
