#pragma once

#include "core/Core.h"
#include "core/Application.h"

#ifndef TESTBED
//#include "core/EntryPoint.h"
#endif

//external
#ifdef GUI
#include "ui/ImguiHandler.h"
#include "imgui.h"
#endif

//sge
#include "Context.h"
#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Transformation.h"
#include "Material.h"
#include "Texture.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "ICamera.h"
#include "Scene.h"
#include "Skybox.h"
#include "PostProcess.h"
#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "EventSystem.h"
#include "Quad.h"
#include "Box.h"
#include "Sphere.h"
#include "Configurations.h"
#include "MeshBuilder.h"
#include "ModelImporter.h"
#include "Resource.h"
#include "GuiMenu.h"
#include "Entity.h"
#include "ShapeFactory.h"
#include "Component.h"
#include "Random.h"
#include "TimeManager.h"
#include "CommonTextures.h"
#include "ResourceManager.h"
#include "ObjectPicker.h"
#include "Physics.h"
#include "Assets.h"
#include "Archiver.h"
#include "AnimationLoader.h"
#include "Animator.h"
#include "Terrain.h"
#include "System.h"
#include "DebugHelper.h"
#include "WaterSystem.h"
