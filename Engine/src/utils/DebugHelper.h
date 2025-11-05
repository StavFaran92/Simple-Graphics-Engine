#pragma once

#include <memory>

#include "core/Engine.h"
#include "core/Core.h"
#include "memory/ResourceWrapper.h"

#include <glm/glm.hpp>

class VertexArrayObject;
class VertexBufferObject;
class Shader;
struct AABB;

class EngineAPI DebugHelper
{
public:
    // Delete copy constructor and assignment operator to prevent duplication
    DebugHelper(const DebugHelper&) = delete;
    DebugHelper& operator=(const DebugHelper&) = delete;

    // Static method to get the instance
    static DebugHelper& getInstance()
    {
        // Use local static variable for thread-safe lazy initialization
        static DebugHelper instance;
        return instance;
    }

    void drawPoint(const glm::vec3& pos, const glm::vec3& color = {1.f,1.f,1.f}, float size = 10.f);

	void drawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color = { 1,1,1 }, float thickness = 1);

	void drawTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);

	void drawAABB(const AABB& aabb, const glm::vec3& color = { 1,1,1 }, float thickness = 1);

    void registerTextureForDebug(const std::string& name, ResourceWrapper<Texture> texture);

    const std::map<std::string, ResourceWrapper<Texture>>& getDebugTextures() const;

private:
    // Private constructor to prevent direct instantiation
    DebugHelper();

    // Optional: Private destructor
    ~DebugHelper() {  }

    std::shared_ptr<VertexArrayObject> m_vao;
    std::shared_ptr<VertexBufferObject> m_pointVBO;
    ResourceWrapper<Shader> m_pointShader;
    ResourceWrapper<Shader> m_lineShader;

    std::map<std::string, ResourceWrapper<Texture>> m_debugTextures;
};