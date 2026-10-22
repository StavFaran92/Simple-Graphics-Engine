# Simple Graphics Engine

C++17 OpenGL engine with an ImGui editor, PhysX, assimp, Lua (sol2) scripting. Windows / MSVC only.

## Layout
- `Engine/` - core library, built as a DLL (`EngineAPI` = dllexport/dllimport; anything EditorApp calls must be exported)
  - `include/EntryPoint.h` - `main()` and command-line parsing into `InitParams`
  - `src/core/` Engine, Logger, project settings · `src/runtime/` Scene, Context, Entity · `src/component/` ECS components
  - `src/render/` RenderView, RenderCommand, Graphics · `src/texture/` Texture (`download()` reads GPU data back)
- `EditorApp/src/` - the editor. `Editor.cpp` holds the main loop hooks, menus and simulation start/stop/pause
- `NativeScripts/` - C++ gameplay scripts loaded by the editor
- `samples/` - sample projects (`game_2`, `sandbox`, `fps`, `feature_samples`); `*Sample/` dirs are standalone demos
- `scripts/` - build and tooling scripts (`gen_sol_bindings.py` generates Lua bindings)

## Build
- Already configured in `build/` (Visual Studio generator). Reconfigure only if CMakeLists change: `cmake -S . -B build`
- Build one target and get only the errors: `powershell -File scripts/build.ps1 -Target EditorApp -Config Debug`
- Changing an Engine header rebuilds most of the tree; prefer .cpp changes when possible
- Do not change `CMAKE_MSVC_RUNTIME_LIBRARY` or the Debug `NDEBUG` define - the prebuilt third-party libs need them

## Verify a change visually
- `powershell -File scripts/run_capture.ps1 [-Project samples/game_2] [-Frames 120]`
- Opens the editor on the project, runs N frames, writes `build/capture/frame.png` (the Editor View) and
  `build/capture/log.txt` (console log), then quits. Read both files to judge the result.
- Same flags by hand: `EditorApp.exe -ec <projectDir> -frames 120 -capture out.png -log log.txt`
  (working directory `build/EditorApp`, binary in `build/bin/<Config>/`)

## Live editor access (MCP)
- A running editor serves MCP tools at `http://127.0.0.1:7777/mcp` (not in `-capture` runs)
- Read-only tools: `get_scene_hierarchy`, `get_entity`, `get_component_schema`, `list_assets`, `get_asset`,
  `get_console`, `screenshot_viewport`, `get_selection`. Entity ids are entt handle ids (same as in .scene files)
- Code: `EditorApp/src/ai/` - `McpServer` (HTTP + JSON-RPC on its own thread), `MainThreadQueue` (tool calls run
  in `EditorApp::update()`), `AgentTools.cpp` (the tools). New tools: add them in `registerAgentTools`
- Mutation tools (`create_entity`, `set_components`, `set_parent`, `undo`, ...) are refused while the simulation runs
- `play` / `pause` / `stop` call the editor's own functions (`SimulationControls.h`); `play` keeps the game from
  capturing the mouse unless `lock_mouse` is true (`Window::setMouseLockAllowed`). `wait` runs on the HTTP thread
  (`McpTool::onServerThread`) so frames keep running; during play, `get_entity` shows live values and
  `screenshot_viewport` captures the Game View
- `new_scene` / `open_scene` switch the active scene as an undo step; `save_scene` writes only the active one
- `focus_entity` / `set_view_camera` aim the editor camera before `screenshot_viewport`; they move the view, not the
  scene, so they aren't undo steps
- `get_material` / `set_material` / `create_material` work on `MaterialData`'s own cereal JSON (the C++ in
  `Engine/src/render/MaterialData.h` defines it); `set_material` reads the result into a copy first, so bad data is
  refused before the material changes. Material edits are undo steps that apply in any scene
- `create_preset` and the Hierarchy's Create menu share one table: `EditorApp/src/EntityPresets.cpp`. A new preset
  (path like `"Environment/Water/Pool"` for nested submenus) shows up in both

## Crashes
- `ErrorHandler::run` (wraps every app's `main`) writes `build/bin/<Config>/crash/last_crash.txt` on a crash:
  error, stack (cpptrace, vendored in `EditorApp/extern/cpptrace`), and the editor's console tail with the `[MCP]`
  tool calls that led to it. Not active under a debugger: the debugger stops at the throw instead
- An MCP call failing with connection refused means the editor is gone: restart it with
  `powershell -File scripts/run_editor.ps1 -Project <dir>`, then call `get_last_crash`

## Undo (EditorApp/src/commands/)
- Undo stores component snapshots (`ComponentSnapshot`, cereal JSON via `Archiver::saveComponent`), not whole entities
- Change made live by a panel: snapshot before, change it, snapshot after, then
  `CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(before, after, label))`
- Entity created by existing code: `record(std::make_unique<CreateEntitiesCmd>(entity, label))` after creating it
- Delete: `CommandStack::Instance().run(std::make_unique<DeleteEntitiesCmd>(entity, label), error)`
- Parent changes: `reparentWithUndo(child, newParent, label)`
- Inspector sections get undo automatically through `displayComponent<T>` (Common.h); a whole drag is one step
- Assets: `AssetSnapshot` (the asset's own `serialize`) and `EditAssetCmd(before, after, label)`, for any type; an asset
  editor wraps its widgets in `InspectorUndo::trackAsset` / `commitAsset`, like components use `trackComponent` /
  `commitComponent`. Asset steps have no scene, so they apply in whichever scene is open
- Asset created by existing code: `record(std::make_unique<CreateAssetCmd>(asset, copyOfItsBuildDescriptor, label))`;
  undo deletes the asset and its file, redo recreates it from the descriptor under the same uuid
  (`AssetBuildDescriptor::hintUUID`)
- History is global: opening a scene is itself an undo step (`OpenSceneCmd`, via `openSceneInEditor`), so scene steps
  are undone with their scene open. Scene commands store their scene and refuse to run in another one
- History is cleared only when a project is opened; undo/redo do nothing while the simulation runs

## Conventions
- Components: `REGISTER_COMPONENT(...)`, serialized via `ComponentSerializer`
- Assets: `Assets::importAsset(AssetBuildDescriptor, <type>LoadDescriptor)`; asset files are `.prefab .mat .model .texture .anim .lua`
- Logging: `logInfo` / `logWarning` / `logError`; everything also lands in the editor Console
- GL state touched outside the renderer must be restored (bind 0 after use)
- Comments are short and explain why, not what
