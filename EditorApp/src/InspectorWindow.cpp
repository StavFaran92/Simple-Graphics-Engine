#include "InspectorWindow.h"

#include <imgui_stdlib.h>

#include "Tables.h"
#include "EntityState.h"
#include "EditorState.h"
#include "NativeScriptsLoader.h"
#include "Dialogs.h"
#include "Widgets.h"

static void displayTransformation(Transformation& transform, bool& isChanged)
{
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	auto& localTransform = transform.getLocalTransformation();
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), matrixTranslation, matrixRotation, matrixScale);

	BEGIN_IMGUI_TABLE("Transform");

	addTableRow("Position", [&](std::string id) {
		if (ImGui::DragFloat3(id.c_str(), matrixTranslation, .1f)) {
			transform.setLocalPosition(glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]));
			isChanged = true;
		}
	});

	addTableRow("Rotation", [&](std::string id) {
		if (ImGui::DragFloat3(id.c_str(), matrixRotation), .001f, 0.f, 180.f) {
			transform.setLocalRotation(glm::radians(glm::vec3(matrixRotation[0], matrixRotation[1], matrixRotation[2])));
			isChanged = true;
		}
	});

	addTableRow("Scale", [&](std::string id) {
		if (ImGui::DragFloat3(id.c_str(), matrixScale, .1f)) {
			transform.setLocalScale(glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]));
			isChanged = true;
		}
	});

	END_IMGUI_TABLE();

}

void InspectorWindow::display()
{
	auto assets = Engine::get()->getSubSystem<Assets>();

	ImGui::Begin("Inspector", NULL, windowFlags);

	//displayWindowHeader("Inspector");

	float windowWidth = ImGui::GetContentRegionAvail().x;

	if (state.getSelectedEntity() != Entity::EmptyEntity)
	{
		displayComponent<Transformation>("Transformation", [](Transformation& transform) {
			bool isChanged = false;
			displayTransformation(transform, isChanged);
		});

		displayComponent<PhysicsComponent>("Physics", [](PhysicsComponent& rBody) {
			ImGui::Combo("##Type", (int*)&rBody.type, rigidyBodyTypesStrList, IM_ARRAYSIZE(rigidyBodyTypesStrList));
			ImGui::InputFloat("Mass", &rBody.mass);

			ImGui::LabelText("", "Linear Lock");
			ImGui::PushID("LinearX");
			ImGui::Checkbox("X", &rBody.isLockedLinearX);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("LinearY");
			ImGui::Checkbox("Y", &rBody.isLockedLinearY);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("LinearZ");
			ImGui::Checkbox("Z", &rBody.isLockedLinearZ);
			ImGui::PopID();

			ImGui::LabelText("", "Angular Lock");
			ImGui::PushID("AngularX");
			ImGui::Checkbox("X", &rBody.isLockedAngularX);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("AngularY");
			ImGui::Checkbox("Y", &rBody.isLockedAngularY);
			ImGui::PopID();
			ImGui::SameLine();
			ImGui::PushID("AngularZ");
			ImGui::Checkbox("Z", &rBody.isLockedAngularZ);
			ImGui::PopID();

			

			static const char* colliderTypeNames[] = {
				"None", "Box", "Sphere", "Terrain", "Mesh", "Capsule"
			};

			if (ImGui::Combo("Collider Type", (int*)&rBody.colliderType, colliderTypeNames, IM_ARRAYSIZE(colliderTypeNames))) {
				switch (rBody.colliderType) {
				case ColliderType::NONE: rBody.collider = 0; break;
				case ColliderType::BOX: rBody.collider = std::make_shared<CollisionBox>(); break;
				case ColliderType::SPHERE: rBody.collider = std::make_shared<CollisionSphere>(); break;
				case ColliderType::TERRAIN: rBody.collider = std::make_shared<CollisionTerrain>(); break;
				case ColliderType::MESH: rBody.collider = std::make_shared<CollisionMesh>(); break;
				case ColliderType::CAPSULE: /* when implemented */ break;
				}
			}

			ColliderType type = rBody.colliderType;

			if (type == ColliderType::NONE)
			{
				return;
			}

			ImGui::Combo("##LayerMask", (int*)&rBody.collider->layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));

			switch (type) {
			case ColliderType::BOX:
				if (auto* box = dynamic_cast<CollisionBox*>(rBody.collider.get())) {
					ImGui::InputFloat3("Extents", &box->extents.x);
				}
				break;
			case ColliderType::SPHERE:
				if (auto* sphere = dynamic_cast<CollisionSphere*>(rBody.collider.get())) {
					ImGui::InputFloat("Radius", &sphere->radius);
				}
				break;
			case ColliderType::TERRAIN:
				ImGui::TextDisabled("Terrain collider has no editable parameters.");
				break;
			case ColliderType::MESH:
				if (auto* mesh = dynamic_cast<CollisionMesh*>(rBody.collider.get())) {
					ImGui::Checkbox("Convex", &mesh->isConvex);
					// Optional: display mesh ResourceWrapper name, etc.
				}
				break;
			case ColliderType::CAPSULE:
				// Handle capsule here if you define its struct
				ImGui::Text("Capsule collider UI not implemented yet.");
				break;
			}
		});

		//displayComponent<CollisionBoxComponent>("Collision Box", [](CollisionBoxComponent& collisionBox) {
		//	ImGui::InputFloat("Half Extent", &collisionBox.halfExtent);
		//	ImGui::Combo("##LayerMask", (int*)&collisionBox.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
		//});

		//displayComponent<CollisionSphereComponent>("Collision Sphere", [](CollisionSphereComponent& collisionSphere) {
		//	ImGui::InputFloat("Radius", &collisionSphere.radius);
		//	ImGui::Combo("##LayerMask", (int*)&collisionSphere.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
		//	});

		//displayComponent<CollisionMeshComponent>("Collision Mesh", [](CollisionMeshComponent& collisionMesh) {
		//	ImGui::Combo("##LayerMask", (int*)&collisionMesh.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
		//	});

		displayComponent<PlayerController>("Player Controller", [](PlayerController& controller) {
			ImGui::TextDisabled("Controller has no editable parameters.");
			//ImGui::Combo("##LayerMask", (int*)&collisionMesh.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
			});

		displayComponent<MeshComponent>("Mesh", [](MeshComponent& meshComponent) {
			if (meshComponent.mesh.isEmpty()) return;

			BEGIN_IMGUI_TABLE("Mesh");

			addTableRow("Vertices count:", [&](std::string id) {
				rightAlignedText(std::to_string((int)meshComponent.mesh.get()->getNumOfVertices()).c_str());
			});

			END_IMGUI_TABLE();

			std::string meshName = "None";
			if (!meshComponent.mesh.isEmpty())
			{
				meshName = meshComponent.mesh.getUID();
			}

			addAssetSelectWidget(meshName, AssetType::MESH, [&meshComponent](UUID uid) {
				meshComponent.mesh = AssetWrapper<MeshCollection>(uid);
			});
			
			
		});

		displayComponent<CameraComponent>("Camera", [](CameraComponent& cameraComponent) {
			// TBD
			
			static const char* projectionMode[] = { "Perspective", "Orthographic" };
			static int currentProjection = 0; // Index of the selected item
			currentProjection = cameraComponent.type;

			ImGui::PushItemWidth(150.0f); // Set dropdown width to 150
			if (ImGui::BeginCombo("##ProjectionMode", projectionMode[currentProjection])) // Label for the combo box
			{
				for (int i = 0; i < IM_ARRAYSIZE(projectionMode); i++)
				{
					bool isSelected = (currentProjection == i);
					if (ImGui::Selectable(projectionMode[i], isSelected))
					{
						currentProjection = i; // Update selected index
						cameraComponent.type = (CameraComponent::CamType)currentProjection;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus(); // Set focus to the current item
				}
				ImGui::EndCombo();
			}

			ImGui::DragFloat("FOVY", &cameraComponent.fovy);
			ImGui::DragFloat("aspect", &cameraComponent.aspect);
			ImGui::DragFloat("z near", &cameraComponent.znear);
			ImGui::DragFloat("z far", &cameraComponent.zfar);
			});

		displayComponent<NativeScriptComponent>("Script", [](NativeScriptComponent& nsc) {
			// Button to trigger some action
			if (ImGui::Button("Select Script"))
			{
				EditorState::Instance().showScriptSelector = true;
			}

			std::string selectedScript;
			displaySelectScriptDialog(selectedScript);

			if (!selectedScript.empty())
			{
				nsc.script = std::shared_ptr<ScriptableEntity>(NativeScriptsLoader::instance->getScript(selectedScript));
			}

			

			// Text display field
			if (nsc.script)
			{
				ImGui::SameLine();
				ImGui::Text(nsc.script->name().c_str());
			}
			});

		displayComponent<MaterialComponent>("Materials", [](MaterialComponent& materials) {
			int index = 0;
			for (auto& [id, mat] : materials)
			{
				ImGui::PushID(&mat);
				
				std::string matName = "None";
				if (!mat.isEmpty())
				{
					matName = mat.resource()->getName();
					if (matName.empty())
					{
						matName = "Material " + std::to_string(index);
					}
				}

				addAssetSelectWidget(matName, AssetType::MATERIAL, [&mat](UUID uid) {
					mat = AssetWrapper<Material>(uid);
				});

				++index;
				ImGui::PopID();
			}
			});

		displayComponent<DirectionalLight>("Directional Light", [](DirectionalLight& dLight) {
			auto& color = dLight.getColor();
			if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
				dLight.SetColor(color);
			}
			});

		displayComponent<PointLight>("Point Light", [](PointLight& pLight) {
			auto& color = pLight.getColor();
			if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
				pLight.SetColor(color);
			}

			Attenuation& attenuation = pLight.getAttenuation();
			ImGui::LabelText("", "Attenuation");
			ImGui::DragFloat("constant", &attenuation.constant, 0.01f);
			ImGui::DragFloat("linear", &attenuation.linear, 0.01f);
			ImGui::DragFloat("quadratic", &attenuation.quadratic, 0.01f);
			pLight.SetAttenuation(attenuation);
		});

		displayComponent<InstanceBatch>("Instance Batch", [](InstanceBatch& instanceBatch) {
			auto& transformations = instanceBatch.getTransformations();

			if (ImGui::Button("Add Transformation")) {
				instanceBatch.addTransformation(std::make_shared<Transformation>(state.getSelectedEntity()));
			}

			if (ImGui::BeginChild("Transformations List", ImVec2(0, 200), true)) {
				bool isChanged = false;
				for (size_t i = 0; i < transformations.size(); ++i) {
					if (ImGui::TreeNode((void*)(intptr_t)i, "Transformation %zu", i)) {
						displayTransformation(*transformations[i], isChanged);
						
						ImGui::TreePop();
					}
				}
				if (isChanged)
				{
					instanceBatch.build();
				}
				ImGui::EndChild();
			}
			});

		displayComponent<SkyboxComponent>("Skybox", [](SkyboxComponent& skybox) {
			addTextureEditWidget(skybox.originalImage, { 50, 50 }, [&](UUID uuid) {
				skybox.setSkybox(AssetWrapper<Texture>(uuid));
			});

			// Compile Button
			if (ImGui::Button("Build"))
			{
				skybox.build();
			}
		});

		displayComponent<ImageComponent>("Image", [](ImageComponent& image) {
			addTextureEditWidget(image.image, { 50, 50 }, [&](UUID uuid) {
				image.image = AssetWrapper<Texture>(uuid);
				});
			ImGui::DragFloat("posX", &image.position.x);
			ImGui::DragFloat("posY", &image.position.y);
			ImGui::DragFloat("sizeX", &image.size.x);
			ImGui::DragFloat("sizeY", &image.size.y);
			});

		displayComponent<Animator>("Animator", [](Animator& animator) {

			ImGui::DragFloat("playback speed", &animator.m_playbackSpeed);

			static int activeAnimationIndex = 0;
			static std::string animName;
			static std::string selectedAnimUID;
			//static bool showAnimationSelector = false;
			//static std::vector<std::string> renameBuffer{};

			EntityState& eState = state.getCurrentEntityState();
			
			ImGui::LabelText("", "Animations");

			auto animations = animator.getAllAnimations();

			auto iter = animations.cbegin();

			// Display animation list
			int index = 0;
			while (iter != animations.cend())
			{
				auto& name = iter->first;
				auto& animation = iter->second;

				ImGui::PushID(index);

				if (ImGui::CollapsingHeader(name.c_str()))
				{

					ImGui::Indent();

					std::string oldName = name;

					float width = ImGui::GetContentRegionAvail().x;
					ImGui::SetNextItemWidth(width);

					ImGui::InputText("##Name", &eState.animationRenameBuffers[index]);

					if (ImGui::IsItemDeactivatedAfterEdit()) 
					{
						auto newAnimationName = std::string(eState.animationRenameBuffers[index]);
						// This runs when the user is done editing,
						// either by pressing Enter or unfocusing the input field
						if (newAnimationName != oldName) {
							auto anim = animator.getAnimation(oldName);
							animator.removeAnimation(oldName);
							animator.addAnimation(newAnimationName, anim);
							break;
						}
					}

					std::string animationName = "None";
					if (!animation.isEmpty())
					{
						animationName = animation.getUID();
					}

					addAssetSelectWidget(animationName, AssetType::ANIMATION, [&animator, name](UUID uid) {
						animator.addAnimation(name, AssetWrapper<Animation>(uid));
					});

					bool isSelected = (index == activeAnimationIndex);
					if (ImGui::Checkbox("Make Active Animation", &isSelected))
					{
						activeAnimationIndex = index;
						animator.playAnimation(name);
					}

					ImGui::Unindent();
				}

				ImGui::PopID();

				iter++;
				index++;
			}

			if (ImGui::Button("+")) {
				int animationsCount = animations.size();
				std::string newAnimationName = "New Animation_" + std::to_string(animationsCount);
				animator.addAnimation(newAnimationName, AssetWrapper<Animation>::empty);
				eState.animationRenameBuffers.push_back(newAnimationName);
			}
		});

		displayComponent<Terrain>("Terrain", [](Terrain& terrain) {
			addTextureEditWidget(terrain.m_heightmap, { 50, 50 }, [&](UUID uuid) {
				terrain = Terrain::generateTerrain(terrain.m_width, terrain.m_height, terrain.m_scale, AssetWrapper<Texture>(uuid));
			});
			ImGui::DragInt("height", &terrain.m_height);
			ImGui::DragInt("width", &terrain.m_width);
			ImGui::DragInt("scale", &terrain.m_scale);


			ImGui::Separator();
			ImGui::LabelText("Textures", "");

			ImGui::SliderInt("Textures", &terrain.m_textureCount, 0, MAX_TEXTURE_COUNT);
			for (int i = 0; i < terrain.m_textureCount; i++)
			{
				ImGui::PushID(i);
				auto texture = terrain.getTexture(i);
				addTextureEditWidget(texture, { 50, 50 }, [i, &terrain](UUID uuid) {
					terrain.setTexture(i, AssetWrapper<Texture>(uuid));
				});
				ImGui::DragFloat("Height blend", &terrain.m_textureBlends[i].blend, .01f);
				auto scale = terrain.getTextureScale(i);
				if (ImGui::DragFloat("scaleX", &scale.r, .01f))
				{
					terrain.setTextureScaleX(i, scale.r);
				}
				if (ImGui::DragFloat("scaleY", &scale.g, .01f))
				{
					terrain.setTextureScaleY(i, scale.g);
				}
				ImGui::PopID();
			}
		});

		displayComponent<ShaderComponent>("Shader Component", [](ShaderComponent& shaderComponent) {
			//auto& state = g_states[selectedEntity.handlerID()];

			if (ImGui::Button("Select Shader"))
			{
				EditorState::Instance().showShaderSelector = true;
			}

			UUID selectedShaderUID;
			displaySelectShaderDialog(selectedShaderUID);

			if (!selectedShaderUID.empty())
			{
				shaderComponent.setShader(AssetWrapper<Shader>(selectedShaderUID));
			}

			if (!shaderComponent.m_customShader.isEmpty())
			{
				auto name = Engine::get()->getSubSystem<Assets>()->getAlias(shaderComponent.m_customShader.getUID());
				ImGui::Text(name.c_str());
			}
			
			// Compile Button
			if (ImGui::Button("recompile"))
			{
				shaderComponent.m_customShader.resource()->recompile();
				shaderComponent.update();
			}

			ImGui::Separator();

			// Projection Type Drop-down
			const char* projectionTypes[] = { "Default", "Texture2D" };
			ImGui::Combo("Projection Type", (int*)&shaderComponent.projection, projectionTypes, IM_ARRAYSIZE(projectionTypes));

			if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
			{
				// Projection Texture
				ImGui::Text("Projection Texture:");
				addTextureEditWidget(shaderComponent.projectionTexture, { 100,100}, [&](UUID uuid) {
					shaderComponent.setProjectionTexture(AssetWrapper<Texture>(uuid));
				});
			}

			// Custom Textures Array
			if (ImGui::CollapsingHeader("Textures"))
			{
				for (auto& [name, texture] : shaderComponent.customTextures)
				{
					ImGui::PushID(name.c_str());
					ImGui::Text(name.c_str());
					addTextureEditWidget(texture, { 100,100 }, [&](UUID uuid) {
						texture = AssetWrapper<Texture>(uuid);
						});
					ImGui::PopID();
				}
				
			}

			// Display Uniforms and Update Shader
			if (ImGui::CollapsingHeader("Uniforms"))
			{
				for (auto& [name, value] : shaderComponent.m_uniformProperties)
				{
					ImGui::PushID(name.c_str());
					bool updated = false; // Track if the value was changed
					std::visit([&](auto& v)
						{
							using T = std::decay_t<decltype(v)>;
							ImGui::Text("%s:", name.c_str());

							if constexpr (std::is_same_v<T, float>)
							{
								updated = ImGui::DragFloat(("##" + name).c_str(), &v, 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::vec2>)
							{
								updated = ImGui::DragFloat2(("##" + name).c_str(), &v[0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::vec3>)
							{
								updated = ImGui::DragFloat3(("##" + name).c_str(), &v[0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::vec4>)
							{
								updated = ImGui::DragFloat4(("##" + name).c_str(), &v[0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, int>)
							{
								updated = ImGui::InputInt(("##" + name).c_str(), &v);
							}
							else if constexpr (std::is_same_v<T, unsigned int>)
							{
								updated = ImGui::InputScalar(("##" + name).c_str(), ImGuiDataType_U32, &v);
							}
							else if constexpr (std::is_same_v<T, glm::mat3>)
							{
								for (int i = 0; i < 3; ++i)
									updated |= ImGui::DragFloat3((name + "##row" + std::to_string(i)).c_str(), &v[i][0], 0.1f);
							}
							else if constexpr (std::is_same_v<T, glm::mat4>)
							{
								for (int i = 0; i < 4; ++i)
									updated |= ImGui::DragFloat4((name + "##row" + std::to_string(i)).c_str(), &v[i][0], 0.1f);
							}
						}, value);

					// If the value changed, update the shader
					if (updated)
					{
						shaderComponent.m_customShader.resource()->setUniformValue(name, value);
					}

					ImGui::PopID();
				}
			}
		});

		displayComponent<FoliageComponent>("Foliage Component", [](FoliageComponent& foliage) {
			// Compile Button
			addTextureEditWidget(foliage.m_foliageSpreadMap, { 100,100 }, [&](UUID uuid) {
				foliage.m_foliageSpreadMap = AssetWrapper<Texture>(uuid);
				});

			ImGui::ColorEdit3("Bottom Color", (float*)&foliage.colorA);
			ImGui::ColorEdit3("Top Color", (float*)&foliage.colorB);
			ImGui::DragFloat("Density", &foliage.globalDensity, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Width", &foliage.width, 0.01f, 1.0f);
			ImGui::DragFloat("Height", &foliage.height, 0.01f, 1.0f);
			ImGui::DragInt("Patch Width", &foliage.patchWidth);
			ImGui::DragInt("Patch Height", &foliage.patchHeight);
			//ImGui::DragInt("Pixel Per Patch", &foliage.pixelPerPatch);

			if (ImGui::Button("Select Terrain"))
			{
				logError("Not yet implemented.");
			}

			//displayEntitySelectDialog();

			if (ImGui::Button("build"))
			{
				foliage.build();
			}
			});

		displayComponent<ScriptComponent>("Lua Script Component", [](ScriptComponent& script) {
			std::string scriptName = "None";
			if (!script.getScript().isEmpty())
			{
				scriptName = script.getScript().getUID();
			}

			addAssetSelectWidget(scriptName, AssetType::LUA_SCRIPT, [&script](UUID uid) {
				script.script = AssetWrapper<LuaScript>(uid);
				});
			});

		displayComponent<TestComp>("Test Component", [](TestComp& testComp) {
			});

		if (ImGui::Button("Add Component", ImVec2(windowWidth, 0)))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			if (ImGui::MenuItem("Transformation"))
			{
				state.getSelectedEntity().addComponent<Transformation>(state.getSelectedEntity());
			}

			if (ImGui::MenuItem("Physics"))
			{
				state.getSelectedEntity().addComponent<PhysicsComponent>();
			}

			if (ImGui::MenuItem("Player Controller"))
			{
				state.getSelectedEntity().addComponent<PlayerController>();
			}

			if (ImGui::MenuItem("Mesh"))
			{
				state.getSelectedEntity().addComponent<MeshComponent>();
			}

			if (ImGui::MenuItem("Camera"))
			{
				state.getSelectedEntity().addComponent<CameraComponent>();
			}

			if (ImGui::MenuItem("C++ Script"))
			{
				state.getSelectedEntity().addComponent<NativeScriptComponent>();
			}

			if (ImGui::MenuItem("InstanceBatch"))
			{
				auto meshComponent = state.getSelectedEntity().tryGetComponent<MeshComponent>();
				if (meshComponent)
				{
					throw std::runtime_error("Not implemented");
					// TODO fix
					//selectedEntity.addComponent<InstanceBatch>(std::vector<std::shared_ptr<Transformation>>{}, meshComponent->mesh);
				}
			}

			if (ImGui::MenuItem("Image"))
			{
				auto& img = state.getSelectedEntity().addComponent<ImageComponent>(BuiltInAssets::getByName<Texture>(SGE_TEXTURE_WHITE));
				img.size = { 50, 50 };
			}

			if (ImGui::MenuItem("Animator"))
			{
				auto& animator = state.getSelectedEntity().addComponent<Animator>();
			}

			if (ImGui::MenuItem("Terrain"))
			{
				auto& terrain = state.getSelectedEntity().addComponent<Terrain>();
			}

			if (ImGui::MenuItem("Skybox"))
			{
				auto& skybox = state.getSelectedEntity().addComponent<SkyboxComponent>();
			}

			if (ImGui::MenuItem("Shader"))
			{
				auto& shader = state.getSelectedEntity().addComponent<ShaderComponent>();
			}

			if (ImGui::MenuItem("Foliage"))
			{
				auto& foliage = state.getSelectedEntity().addComponent<FoliageComponent>();
			}

			if (ImGui::MenuItem("Lua Script"))
			{
				auto& script = state.getSelectedEntity().addComponent<ScriptComponent>();
				script.entity = state.getSelectedEntity();
			}

			//Todo REMOVE
			if (ImGui::MenuItem("TestComponent"))
			{
				auto& testComp = state.getSelectedEntity().addComponent<TestComp>();
			}

			ImGui::EndPopup();
		}
		
	}

	// Render inspector content here
	ImGui::End();
}