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

#include "component/ImageComponent.h"
#include "physics/Physics.h"

#include "core/Window.h"
#include "ui/Mouse.h"
#include "ui/Input.h"

#include "ui/KeyCodes_Lua.gen.h"

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
            [](Animator& self, AssetWrapper<Animation> animation) { self.playAnimation(animation); }
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

    lua.new_usertype<PlayerController>("PlayerController",
        "move", &PlayerController::move
    );

    lua.new_usertype<CameraComponent>("CameraComponent",
        "front", &CameraComponent::front,
        "right", &CameraComponent::right,
        "center", &CameraComponent::center,
        "up", &CameraComponent::up
    );
}

void bindAssets(sol::state& lua)
{
    lua.new_usertype<Animation>("Animation",
        "getDuration", &Animation::getDuration,
        "getTicksPerSecond", &Animation::getTicksPerSecond,
        "import", &Animation::import
    );


    /// TODO fix
    //lua.new_usertype<Material>("Material",
    //    "getSampler", &Material::getSampler,
    //    "setSampler", &Material::setSampler,
    //    "hasTexture", &Material::hasTexture,
    //    "setTexture", &Material::setTexture,
    //    "setName", &Material::setName,
    //    "getName", &Material::getName,
    //    "getAllTextures", &Material::getAllTextures,
    //    "isOpaque", &Material::isOpaque,
    //    "import", &Material::import,
    //    "create", &Material::create,
    //    "updateAsset", &Material::updateAsset
    //);



    lua.new_usertype<Shader>("Shader",
        "getShaderOverride", &Shader::getShaderOverride,
        "recompile", &Shader::recompile,
        "getSourceCode", &Shader::getSourceCode,
        "createOverrideShader", &Shader::createOverrideShader,
        "import", &Shader::import
    );

    lua.new_usertype<ResourceWrapper<Prefab>>("Prefab",
        sol::factories(
            [](int value) {
                // your custom UUID creation from int
                return ResourceWrapper<Prefab>(UUID(value));
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
            [](int width, int height, int internalFormat, int format, int type, std::map<int, int> params, bool isEngineOwned, void* data) {
                return Texture::create2DTextureFromBuffer(width, height, internalFormat, format, type, params, isEngineOwned, data);
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

void bindUI(sol::state& lua)
{
    // Mouse
    lua.new_enum("MouseButton",
        "MOUSE_BUTTON_LEFT", MouseButton::MOUSE_BUTTON_LEFT,
        "MOUSE_BUTTON_RIGHT", MouseButton::MOUSE_BUTTON_RIGHT,
        "MOUSE_BUTTON_MIDDLE", MouseButton::MOUSE_BUTTON_MIDDLE
    );

    lua.new_enum("MouseEventType",
        "Invalid", Mouse::MouseEventType::Invalid,
        "Motion", Mouse::MouseEventType::Motion,
        "ButtonPressed", Mouse::MouseEventType::ButtonPressed,
        "ButtonReleased", Mouse::MouseEventType::ButtonReleased
    );

    lua.new_usertype<Mouse::MouseEvent>("MouseEvent",
        "type", &Mouse::MouseEvent::type,
        "x", &Mouse::MouseEvent::x,
        "y", &Mouse::MouseEvent::y,
        "xrel", &Mouse::MouseEvent::xrel,
        "yrel", &Mouse::MouseEvent::yrel,
        "clicks", &Mouse::MouseEvent::clicks,
        "button", &Mouse::MouseEvent::button
    );

    lua.new_usertype<GameMouse>("Mouse",
        sol::no_constructor,
        "get", []() { return std::ref(*Engine::get()->getSubSystem<GameMouse>()); },

        // Methods
        "getButtonPressed", & GameMouse::getButtonPressed,
        "onMousePressed", &GameMouse::onMousePressed,
        "onMouseReleased", &GameMouse::onMouseReleased,
        "onMouseMotion", &GameMouse::onMouseMotion,
        "lock", []() {Engine::get()->getWindow()->lockMouse(); },
        "unlock", []() {Engine::get()->getWindow()->unlockMouse(); }
    );

    // Keyboard
    loadKeyCodes(lua);

    lua.new_enum("KeyState",
        "Invalid", Keyboard::KeyState::Invalid,
        "Pressed", Keyboard::KeyState::Pressed,
        "Released", Keyboard::KeyState::Released
    );

    lua.new_usertype<Keyboard::KeyEvent>("KeyEvent",
        "state", &Keyboard::KeyEvent::state,
        "repeat", &Keyboard::KeyEvent::repeat,
        "code", &Keyboard::KeyEvent::keysym
    );

    lua.new_usertype<GameKeyboard>("Keyboard",
        sol::no_constructor,
        "get", []() { return std::ref(*Engine::get()->getSubSystem<GameKeyboard>()); },

        // Methods
        "getKeyState", &GameKeyboard::getKeyState,
        "onKeyPressed", &GameKeyboard::onKeyPressed,
        "onKeyReleased", &GameKeyboard::onKeyReleased
    );

    lua.new_usertype<Physics::HitResult>("HitResult",
        // Members
        "position", &Physics::HitResult::position,
        "normal", &Physics::HitResult::normal,
        "distance", &Physics::HitResult::distance,
        "entity", &Physics::HitResult::e
    );

    lua.new_enum("LayerMask",
        "LAYER_0", Physics::LayerMask::LAYER_0,
        "LAYER_1", Physics::LayerMask::LAYER_1,
        "LAYER_2", Physics::LayerMask::LAYER_2,
        "LAYER_3", Physics::LayerMask::LAYER_3,
        "LAYER_4", Physics::LayerMask::LAYER_4,
        "LAYER_5", Physics::LayerMask::LAYER_5,
        "LAYER_6", Physics::LayerMask::LAYER_6,
        "LAYER_7", Physics::LayerMask::LAYER_7,
        "LAYER_8", Physics::LayerMask::LAYER_8,
        "LAYER_9", Physics::LayerMask::LAYER_9,
        "LAYER_10", Physics::LayerMask::LAYER_10,
        "LAYER_11", Physics::LayerMask::LAYER_11,
        "LAYER_12", Physics::LayerMask::LAYER_12,
        "LAYER_13", Physics::LayerMask::LAYER_13,
        "LAYER_14", Physics::LayerMask::LAYER_14,
        "LAYER_15", Physics::LayerMask::LAYER_15,
        "LAYER_16", Physics::LayerMask::LAYER_16,
        "LAYER_17", Physics::LayerMask::LAYER_17
    );


    lua.set_function("raycast", Physics::raycast);
}

void bindAll(sol::state& lua) 
{
    bindAssets(lua);
    bindComponents(lua);
    bindUI(lua);

    lua.new_usertype<glm::vec2>("vec2",
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<
        glm::vec3(),                    // default: vec3()
        glm::vec3(float),  // parameterized: vec3(x, y, z)
        glm::vec3(float, float, float)  // parameterized: vec3(x, y, z)
        >(),
        // Arithmetic operators
        sol::meta_function::addition, sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(glm::operator+),
        sol::meta_function::subtraction, sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(glm::operator-),
        sol::meta_function::multiplication, sol::overload(
            sol::resolve<glm::vec3(const glm::vec3&, float)>(glm::operator*),
            sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(glm::operator*)
        ),
        sol::meta_function::division, sol::resolve<glm::vec3(const glm::vec3&, float)>(glm::operator/),
        sol::meta_function::to_string, [](const glm::vec3& v) {
            return "vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        },
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



    

    lua.set_function("getActiveScene", []() { return  std::ref(*Engine::get()->getContext()->getActiveScene()); });
    lua.set_function("assets", []() { return std::ref(*Engine::get()->getSubSystem<Assets>()); });

}