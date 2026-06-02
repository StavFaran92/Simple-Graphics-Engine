#pragma once

#include "render/RenderData.h"
#include <vector>
#include "render/SSBO.h"

class SceneBuffer
{
public:
    uint32_t addObject(const RenderData::ObjectData& obj) { return push(m_objects, obj); }
    uint32_t addMaterial(const RenderData::MaterialData& mat) { return push(m_materials, mat); }

    void upload();

    void bind(int slot);

private:
    template<typename T>
    uint32_t push(std::vector<T>& vec, const T& val)
    {
        uint32_t index = vec.size();
        vec.push_back(val);
        return index; // return index so shader can look it up
    }

    std::vector<RenderData::ObjectData>   m_objects;
    std::vector<RenderData::MaterialData> m_materials;
    std::vector<RenderData::DrawCommand>  m_drawCommands;
    SSBO m_ssbo;
};