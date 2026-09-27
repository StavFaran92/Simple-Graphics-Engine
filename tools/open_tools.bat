@echo off
rem Opens the engine tools launcher GUI
rem Prefers windowless python (pythonw / pyw), falls back to python which keeps a console open
cd /d "%~dp0"

where pythonw >nul 2>nul && (start "" pythonw tools_launcher.py & exit /b)
where pyw >nul 2>nul && (start "" pyw tools_launcher.py & exit /b)
start "" python tools_launcher.py
