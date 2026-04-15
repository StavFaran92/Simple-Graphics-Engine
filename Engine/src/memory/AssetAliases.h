#pragma once

#include "memory/ResourceRef.h"
#include "memory/AssetRef.h"

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

using TextureAssetRef = AssetRef<TextureAsset>;
using TextureResourceRef = ResourceRef<Texture>;

using ShaderAssetRef = AssetRef<ShaderAsset>;
using ShaderResourceRef = ResourceRef<Shader>;

using MaterialAssetRef = AssetRef<MaterialAsset>;
using MaterialResourceRef = ResourceRef<Material>;

using ModelAssetRef = AssetRef<ModelAsset>;
using ModelResourceRef = ResourceRef<Model>;

using AnimationAssetRef = AssetRef<AnimationAsset>;
using AnimationResourceRef = ResourceRef<Animation>;

using LuaScriptAssetRef = AssetRef<LuaScriptAsset>;
using LuaScriptResourceRef = ResourceRef<LuaScript>;

using PrefabAssetRef = AssetRef<PrefabAsset>;
using PrefabResourceRef = ResourceRef<Prefab>;

using SceneAssetRef = AssetRef<SceneAsset>;
using SceneResourceRef = ResourceRef<Scene>;
