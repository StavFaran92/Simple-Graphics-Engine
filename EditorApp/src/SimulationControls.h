#pragma once

// The editor's play mode: the same code the toolbar, F9/F10/F11 and the agent's play tools run.
// Besides the engine's simulation, these switch the rendered view, the editor UI and the editor camera.
void startsimulation();
void pauseSimulation();
void stopSimulation();
