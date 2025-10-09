// Auto-generated Lua bindings using Sol3
#include <sol/sol.hpp>

#include "animation/Animator.h"
#include "animation/Animation.h"
#include "component/Terrain.h"
#include "component/Transformation.h"
#include "geometry/Mesh.h"
#include "geometry/MeshCollection.h"
#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"
#include "render/Material.h"
#include "runtime/Prefab.h"
#include "scripts/LuaScript.h"
#include "texture/Texture.h"

using ComponentGetter = std::function<sol::object(Entity&, sol::state_view)>;

std::unordered_map<std::string, ComponentGetter> componentGetters{
    { "Transform", [](Entity& e, sol::state_view lua) { return sol::make_object(lua, e.getComponent<Transformation>()); } },
};

void BindAllToLua(sol::state& lua) {

    lua.new_usertype<glm::vec2>("vec2",
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<
        glm::vec3(),                    // default: vec3()
        glm::vec3(float, float, float)  // parameterized: vec3(x, y, z)
        >(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    lua.new_usertype<Entity>("Entity",
        // Constructors
        sol::no_constructor,  // you control lifetime

        "getComponent", [](Entity& e, const std::string& name, sol::this_state s) {
            auto it = componentGetters.find(name);
            if (it != componentGetters.end())
                return it->second(e, s);
            return sol::make_object(s, sol::nil);
        },

        "getTransform", [](Entity& self) -> Transformation& { return self.getComponent<Transformation>(); },

        // Parent/child hierarchy
        "setRoot", &Entity::setRoot,
        "setParent", &Entity::setParent,
        "removeParent", &Entity::removeParent,
        "getParent", &Entity::getParent,
        //"addChildren", &Entity::addChildren,
        //"removeChildren", &Entity::removeChildren,
        "getChildren", &Entity::getChildren,
        "getChildByName", &Entity::getChildByName,
        "getRoot", &Entity::getRoot,

        // Entity validity & comparison
        "valid", &Entity::valid,
        "equals", [](Entity& self, Entity& other) { return self == other; },
        "notEquals", [](Entity& self, Entity& other) { return self != other; },

        // Handlers
        "handler", &Entity::handler,
        "handlerID", &Entity::handlerID,

        // Remove function
        "remove", &Entity::remove
    );


    lua.new_usertype<Animation>("Animation",
        "getDuration", &Animation::getDuration,
        "getTicksPerSecond", &Animation::getTicksPerSecond,
        "import", &Animation::import
    );

    lua.new_usertype<Animator>("Animator",
        "playAnimation", sol::overload(
            [](Animator& self, const std::string& name) { self.playAnimation(name); },
            [](Animator& self, ResourceWrapper<Animation> animation) { self.playAnimation(animation); }
        ),
        "setPlaybackSpeed", &Animator::setPlaybackSpeed,
        "addAnimation", &Animator::addAnimation,
        "removeAnimation", &Animator::removeAnimation,
        "getAnimation", &Animator::getAnimation,
        "getAllAnimations", &Animator::getAllAnimations
        //"getCurrentAnimationName", &Animator::getCurrentAnimationName
    );

    lua.new_usertype<Terrain>("Terrain",
        "createTerrain", &Terrain::createTerrain,
        "getMesh", &Terrain::getMesh,
        "getHeightmap", &Terrain::getHeightmap,
        "getHeightAtPoint", &Terrain::getHeightAtPoint,
        "getScale", &Terrain::getScale,
        "getWidth", &Terrain::getWidth,
        "getHeight", &Terrain::getHeight,
        "setTexture", &Terrain::setTexture,
        "setTextureScaleX", &Terrain::setTextureScaleX,
        "setTextureScaleY", &Terrain::setTextureScaleY,
        "setTextureBlend", &Terrain::setTextureBlend,
        "getTexture", &Terrain::getTexture,
        "getTextureBlend", &Terrain::getTextureBlend,
        "getTextureScale", &Terrain::getTextureScale,
        "getTextureCount", &Terrain::getTextureCount
    );

    lua.new_usertype<Transformation>("Transformation",
        "setLocalPosition", &Transformation::setLocalPosition,
        "setWorldPosition", &Transformation::setWorldPosition,
        "setLocalRotation", sol::overload(
            [](Transformation& self, float angle, glm::vec3 axis) { self.setLocalRotation(angle, axis); },
            [](Transformation& self, glm::quat quat) { self.setLocalRotation(quat); }
        ),
        "setWorldRotation", &Transformation::setWorldRotation,
        "setLocalScale", &Transformation::setLocalScale,
        "setWorldScale", &Transformation::setWorldScale,
        "getLocalPosition", &Transformation::getLocalPosition,
        "getWorldPosition", &Transformation::getWorldPosition,
        "getLocalRotationQuat", &Transformation::getLocalRotationQuat,
        "getLocalRotationVec3", &Transformation::getLocalRotationVec3,
        "getWorldRotation", &Transformation::getWorldRotation,
        "getLocalScale", &Transformation::getLocalScale,
        "getWorldScale", &Transformation::getWorldScale,
        "getForward", &Transformation::getForward,
        "getUp", &Transformation::getUp,
        "getRight", &Transformation::getRight,
        "translate", sol::overload(
            [](Transformation& self, float x, float y, float z) { self.translate(x, y, z); },
            [](Transformation& self, glm::vec3 translation) { self.translate(translation); }
        ),
        "scale", sol::overload(
            [](Transformation& self, float x, float y, float z) { self.scale(x, y, z); },
            [](Transformation& self, glm::vec3 translation) { self.scale(translation); }
        ),
        "rotate", sol::overload(
            [](Transformation& self, glm::vec3 eulers) { self.rotate(eulers); },
            [](Transformation& self, glm::vec3 axis, float angle) { self.rotate(axis, angle); }
        ),
        "rotateAround", &Transformation::rotateAround,
        "rotateLerp", &Transformation::rotateLerp,
        "rotateAroundLerp", &Transformation::rotateAroundLerp,
        "getWorldTransformation", sol::overload(
            [](Transformation& self) { return self.getWorldTransformation(); }
        ),
        "getLocalTransformation", &Transformation::getLocalTransformation,
        "setParent", &Transformation::setParent,
        "removeParent", &Transformation::removeParent,
        "getParent", &Transformation::getParent,
        "getRoot", &Transformation::getRoot,
        "setRoot", &Transformation::setRoot,
        "update", &Transformation::update,
        "forceUpdate", &Transformation::forceUpdate,
        "worldToLocal", &Transformation::worldToLocal,
        "getChildren", &Transformation::getChildren
    );

    lua.new_usertype<Mesh>("Mesh",
        "getNumOfVertices", &Mesh::getNumOfVertices,
        "getPositions", &Mesh::getPositions,
        "getNormals", &Mesh::getNormals,
        "getAABB", &Mesh::getAABB,
        "getMaterialIndex", &Mesh::getMaterialIndex,
        "setRestTransform", &Mesh::setRestTransform,
        "getRestTransform", &Mesh::getRestTransform
    );

    lua.new_usertype<MeshCollection>("MeshCollection",
        "addMesh", &MeshCollection::addMesh,
        "getPrimaryMesh", &MeshCollection::getPrimaryMesh,
        "getMeshes", &MeshCollection::getMeshes,
        "getNumOfVertices", &MeshCollection::getNumOfVertices,
        "addBonesInfo", &MeshCollection::addBonesInfo,
        "getBoneOffsets", &MeshCollection::getBoneOffsets,
        "getBoneID", &MeshCollection::getBoneID,
        "import", &MeshCollection::import,
        "getLastLoadedMaterials", &MeshCollection::getLastLoadedMaterials
    );

    lua.new_usertype<DirectionalLight>("DirectionalLight",
        "useLight", &DirectionalLight::useLight,
        "SetAmbientIntensity", &DirectionalLight::SetAmbientIntensity,
        "SetDiffuseIntensity", &DirectionalLight::SetDiffuseIntensity,
        "SetColor", &DirectionalLight::SetColor,
        "getColor", &DirectionalLight::getColor
    );

    lua.new_usertype<PointLight>("PointLight",
        "SetAmbientIntensity", &PointLight::SetAmbientIntensity,
        "SetDiffuseIntensity", &PointLight::SetDiffuseIntensity,
        "SetColor", &PointLight::SetColor,
        "getColor", &PointLight::getColor,
        "SetAttenuation", &PointLight::SetAttenuation,
        "getAttenuation", &PointLight::getAttenuation
    );

    lua.new_usertype<Material>("Material",
        "getSampler", &Material::getSampler,
        "setSampler", &Material::setSampler,
        "hasTexture", &Material::hasTexture,
        "setTexture", &Material::setTexture,
        "setName", &Material::setName,
        "getName", &Material::getName,
        "getAllTextures", &Material::getAllTextures,
        "isOpaque", &Material::isOpaque,
        "import", &Material::import,
        "create", &Material::create,
        "updateAsset", &Material::updateAsset
    );

    lua.new_usertype<Shader>("Shader",
        "getShaderOverride", &Shader::getShaderOverride,
        "recompile", &Shader::recompile,
        "getSourceCode", &Shader::getSourceCode,
        "createOverrideShader", &Shader::createOverrideShader,
        "import", &Shader::import
    );

    lua.new_usertype<Prefab>("Prefab",
        "import", &Prefab::import,
        "create", &Prefab::create,
        "save", &Prefab::save,
        "Instansiate", &Prefab::Instansiate
    );

    lua.new_usertype<LuaScript>("LuaScript",
        "import", &LuaScript::import,
        "create", &LuaScript::create,
        "updateAsset", &LuaScript::updateAsset
    );

    lua.new_usertype<Texture>("Texture",
        "createEmptyTexture", sol::overload(
            [](int width, int height) {
                return Texture::createEmptyTexture(width, height);
            },
            [](int width, int height, int internalFormat, int format, int type) {
                return Texture::createEmptyTexture(width, height, internalFormat, format, type);
            }
        ),
        "create2DTextureFromBuffer", sol::overload(
            [](int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool isTransient, void* data) {
                return Texture::create2DTextureFromBuffer(width, height, internalFormat, format, type, params, isTransient, data);
            },
            [](const Texture::TextureData& textureData) {
                return Texture::create2DTextureFromBuffer(textureData);
            }
        ),
        "getWidth", &Texture::getWidth,
        "getHeight", &Texture::getHeight,
        "getBitDepth", &Texture::getBitDepth,
        "setData", &Texture::setData,
        "bind", &Texture::bind,
        "unbind", &Texture::unbind,
        "getID", &Texture::getID,
        "setSlot", &Texture::setSlot,
        "importTexture3D", &Texture::importTexture3D,
        "import", &Texture::import,
        "isHDRImage", &Texture::isHDRImage
    );

}