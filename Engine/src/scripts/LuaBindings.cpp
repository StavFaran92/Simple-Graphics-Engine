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
#include "runtime/Scene.h"
#include "scripts/LuaScript.h"
#include "texture/Texture.h"
#include "component/CameraComponent.h"
#include "component/SkyboxComponent.h"
#include "component/ShaderComponent.h"
#include "component/PlayerControllerComponent.h"
#include "component/PhysicsComponent.h"
#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"
#include "component/MaterialComponent.h"
#include "component/ImageComponent.h"

#include "core/Window.h"
#include "ui/Mouse.h"
#include "ui/Input.h"

using ComponentGetter = std::function<sol::object(Entity&, sol::this_state)>;

std::unordered_map<std::string, ComponentGetter> componentGetters{
    { "Transform", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<Transformation>()));
    } },

    {
        "Camera", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<CameraComponent>()));
    } },

    { "Terrain", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<Terrain>()));
    } },

    { "Foliage", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<FoliageComponent>()));
    } },

    { "Skybox", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<SkyboxComponent>()));
    } },

    { "Shader", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<ShaderComponent>()));
    } },

    { "PlayerController", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<PlayerController>()));
    } },

    { "Physics", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<PhysicsComponent>()));
    } },

    { "Object", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<ObjectComponent>()));
    } },

    { "Renderable", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<RenderableComponent>()));
    } },

    { "Material", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<MaterialComponent>()));
    } },

    { "Image", [](Entity& e, sol::this_state lua) -> sol::object {
        return sol::object(lua, sol::in_place, std::ref(e.getComponent<ImageComponent>()));
    } }
};

sol::object getComponentHelper(Entity & e, sol::stack_object key, sol::this_state lua)
{
    auto maybe_string_key = key.as<sol::optional<std::string>>();
    if (maybe_string_key) {
        const std::string& k = *maybe_string_key;

        auto it = componentGetters.find(k);
        if (it != componentGetters.end())
            return it->second(e, lua);
        return sol::make_object(lua, sol::nil);
    }
    return sol::make_object(lua, sol::nil);
}

void bindComponents(sol::state& lua)
{
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

    lua.new_usertype<DirectionalLight>("DirectionalLight",
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

    lua.new_usertype<MaterialComponent>("MaterialComponent",
        "addMaterial", &MaterialComponent::addMaterial,
        "setMaterial", &MaterialComponent::setMaterial,
        "at", &MaterialComponent::at
    );

    lua.new_usertype<PlayerController>("PlayerController",
        "move", &PlayerController::move
    );
}

void bindAssets(sol::state& lua)
{
    lua.new_usertype<Animation>("Animation",
        "getDuration", &Animation::getDuration,
        "getTicksPerSecond", &Animation::getTicksPerSecond,
        "import", &Animation::import
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

    lua.new_usertype<ResourceWrapper<Prefab>>("Prefab",
        sol::factories(
            [](const std::string& str) {
                // your custom UUID creation from string
                return ResourceWrapper<Prefab>(UUID(str));
            },
            [](int value) {
                // your custom UUID creation from int
                return ResourceWrapper<Prefab>(UUID(std::to_string(value)));
            }
        ),
        // Instance methods
        "save", [](ResourceWrapper<Prefab>& self) {
            self->save(self, {});
        },
        "instansiate", [](ResourceWrapper<Prefab>& self, glm::vec3 position) {
            self->Instansiate(position);
        },

        // Static methods wrapped as lambdas inside new_usertype
        "create", [](const Entity& e) {
            return Prefab::create(e); // returns ResourceWrapper<Prefab>
        },
        "import", [](const std::string& path) {
            return Prefab::import(path, {}); // returns ResourceWrapper<Prefab>
        }
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
}

void bindAll(sol::state& lua) 
{
    bindAssets(lua);
    bindComponents(lua);

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
        sol::no_constructor,

        sol::meta_function::index,
        &getComponentHelper,

        "setRoot", &Entity::setRoot,
        "setParent", &Entity::setParent,
        "removeParent", &Entity::removeParent,
        "getParent", &Entity::getParent,
        "getChildren", &Entity::getChildren,
        "getChildByName", &Entity::getChildByName,
        "getRoot", &Entity::getRoot,

        "valid", &Entity::valid,
        "equals", [](Entity& self, Entity& other) { return self == other; },
        "notEquals", [](Entity& self, Entity& other) { return self != other; },

        "handler", &Entity::handler,
        "handlerID", &Entity::handlerID,

        // Remove function
        "remove", &Entity::remove
    );

    lua.new_usertype<Scene>("Scene",
        "createEntity", sol::overload(
            [](Scene& self) { return self.createEntity(); },
            [](Scene& self, const std::string& name) { return self.createEntity(name); }
        ),
        "removeEntity", &Scene::removeEntity,
        "getEntityByName", &Scene::getEntityByName
    );

    lua.new_usertype<Assets>("Assets",
        // Constructor
        sol::no_constructor,

        // Methods
        "getAlias", &Assets::getAlias,
        "getAllAssetsOfType", &Assets::getAllAssetsOfType,
        "getAllAssets", &Assets::getAllAssets,
        "getAssetFromPath", &Assets::getAssetFromPath,
        "getAsset", &Assets::getAsset,
        "hasAsset", &Assets::hasAsset,
        "updateAsset", &Assets::updateAsset,
        "importAsset", &Assets::importAsset,
        "createAsset", &Assets::createAsset
    );

    lua.new_usertype<Window>("Window",
        sol::no_constructor,

        // Methods
        "width", &Window::getWidth,
        "height", &Window::getHeight
    );

    lua.new_enum("MouseButton",
        "LeftMousebutton", Mouse::MouseButton::LeftMousebutton,
        "RightMousebutton", Mouse::MouseButton::RightMousebutton,
        "MiddleMousebutton", Mouse::MouseButton::MiddleMousebutton
    );

    lua.new_usertype<Mouse>("Mouse",
        sol::no_constructor,
        "get", []() { return std::ref(*Engine::get()->getInput()->getMouse()); },

        // Methods
        "getButtonPressed", &Mouse::getButtonPressed
    );

    lua.new_usertype<GameKeyboard>("Keyboard",
        sol::no_constructor,
        "get", []() { return std::ref(*Engine::get()->getSubSystem<GameKeyboard>()); },

        // Methods
        "getKeyState", &GameKeyboard::getKeyState
    );

    

    lua.set_function("getActiveScene", []() { return Engine::get()->getContext()->getActiveScene(); });
    lua.set_function("assets", []() { return std::ref(*Engine::get()->getSubSystem<Assets>()); });

}