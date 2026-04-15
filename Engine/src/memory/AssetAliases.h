#pragma once

#include "memory/ResourceWrapper.h"
#include "memory/AssetHandle.h"

class Texture;
class TextureAsset;
class Shader;
class ShaderAsset;
class Material;
class MaterialAsset;
class Model;
class ModelAsset;
class Animation;
class AnimationAsset;
class LuaScript;
class LuaScriptAsset;
class Prefab;
class PrefabAsset;
class Scene;
class SceneAsset;

using TextureAssetRef = AssetHandle<TextureAsset>;
using TextureResourceRef = ResourceWrapper<Texture>;

using ShaderAssetRef = AssetHandle<ShaderAsset>;
using ShaderResourceRef = ResourceWrapper<Shader>;

using MaterialAssetRef = AssetHandle<MaterialAsset>;
using MaterialResourceRef = ResourceWrapper<Material>;

using ModelAssetRef = AssetHandle<ModelAsset>;
using ModelResourceRef = ResourceWrapper<Model>;

using AnimationAssetRef = AssetHandle<AnimationAsset>;
using AnimationResourceRef = ResourceWrapper<Animation>;

using LuaScriptAssetRef = AssetHandle<LuaScriptAsset>;
using LuaScriptResourceRef = ResourceWrapper<LuaScript>;

using PrefabAssetRef = AssetHandle<PrefabAsset>;
using PrefabResourceRef = ResourceWrapper<Prefab>;

using SceneAssetRef = AssetHandle<SceneAsset>;
using SceneResourceRef = ResourceWrapper<Scene>;
