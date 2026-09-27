#!/usr/bin/env python3
"""
Launcher GUI for the trace tools in this folder.

Usage:
    tools/open_tools.bat
    python tools/tools_launcher.py

Requires Python 3.8+ (stdlib only).
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import tkinter as tk
from tkinter import filedialog, ttk

TOOLS_DIR = Path(__file__).resolve().parent

# (display name, script file, description)
TOOLS = [
    ("Resource viewer", "resource_viewer.py", "Resources, assets and scene resource cache"),
    ("Animation state viewer", "anim_state_viewer.py", "Current animation graph state per entity"),
]


class ToolsLauncherApp:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Engine tools")
        self.root.resizable(False, False)
        self._build_ui()

    def _build_ui(self) -> None:
        frame = ttk.Frame(self.root, padding=10)
        frame.pack(fill=tk.BOTH, expand=True)

        ttk.Label(frame, text="Trace file (empty = latest in build/EditorApp/logs):").grid(
            row=0, column=0, columnspan=2, sticky=tk.W
        )
        self.path_var = tk.StringVar()
        ttk.Entry(frame, textvariable=self.path_var, width=60).grid(
            row=1, column=0, sticky=tk.EW, pady=(2, 10)
        )
        ttk.Button(frame, text="Browse...", command=self._on_browse).grid(
            row=1, column=1, padx=(4, 0), pady=(2, 10)
        )

        for i, (name, script, description) in enumerate(TOOLS):
            ttk.Button(
                frame, text=name, width=26, command=lambda s=script: self._launch(s)
            ).grid(row=2 + i, column=0, sticky=tk.W, pady=2)
            ttk.Label(frame, text=description).grid(row=2 + i, column=1, sticky=tk.W, padx=(8, 0))

        self.status = ttk.Label(frame, text="", foreground="gray")
        self.status.grid(row=2 + len(TOOLS), column=0, columnspan=2, sticky=tk.W, pady=(10, 0))

    def _on_browse(self) -> None:
        initial_dir = TOOLS_DIR.parent / "build" / "EditorApp" / "logs"
        path = filedialog.askopenfilename(
            initialdir=str(initial_dir) if initial_dir.is_dir() else str(TOOLS_DIR),
            filetypes=[("Trace files", "*.jsonl"), ("All files", "*.*")],
        )
        if path:
            self.path_var.set(path)

    def _launch(self, script: str) -> None:
        cmd = [sys.executable, str(TOOLS_DIR / script)]
        trace_path = self.path_var.get().strip()
        if trace_path:
            cmd.append(trace_path)
        subprocess.Popen(cmd, cwd=str(TOOLS_DIR))
        self.status.config(text=f"Launched {script}")

    def run(self) -> None:
        self.root.mainloop()


if __name__ == "__main__":
    ToolsLauncherApp().run()
