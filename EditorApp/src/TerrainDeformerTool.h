#pragma once

#include "imgui.h"

#include "EditorTool.h"
#include "core/Subscriber.h"

#include "component/Terrain.h"
#include "sge.h"

class TerrainDeformerTool : public EditorTool
{
public:
    TerrainDeformerTool() = default;

    void update(ImVec2 viewportPos, ImVec2 viewportSize) override;

    bool onEvent(SDL_Event e);

    const char* name() const override
    {
        return "TerrainDeformerTool";
    }

private:
    TexturePainter m_texturePainter;
};