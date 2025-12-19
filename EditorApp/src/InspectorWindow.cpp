#include "InspectorWindow.h"

#include <imgui_stdlib.h>

#include "Tables.h"
#include "EntityState.h"
#include "EditorState.h"
#include "NativeScriptsLoader.h"
#include "Dialogs.h"
#include "Widgets.h"
#include "dialogs/AssetSelectDialog.h"

bool g_testRay = false;
Terrain* g_activeTerrain = 0;

extern Entity g_editorCamera;

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

	ImGui::Begin("Inspector", NULL, windowFlags & 
		~ImGuiWindowFlags_NoScrollbar &
		~ImGuiWindowFlags_NoScrollWithMouse);

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

		displayComponent<PlayerController>("Player Controller", [](PlayerController& controller) {
			ImGui::TextDisabled("Controller has no editable parameters.");
			//ImGui::Combo("##LayerMask", (int*)&collisionMesh.layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));
			});

		displayComponent<MeshRendererComponent>("Mesh Renderer", [](MeshRendererComponent& meshComponent) {
			if (meshComponent.mesh.isEmpty()) return;

			BEGIN_IMGUI_TABLE("Mesh");

			addTableRow("Vertices count:", [&](std::string id) {
				rightAlignedText(std::to_string((int)meshComponent.mesh.get()->getNumOfVertices()));
			});

			END_IMGUI_TABLE();

			// Array of string names for the enum
			const char* renderTechniqueNames[] = { "Forward", "Deferred" };

			ImGui::Combo("Render Technique", (int*)&meshComponent.renderTechnique, renderTechniqueNames, IM_ARRAYSIZE(renderTechniqueNames));

			std::string meshName = "None";
			if (!meshComponent.mesh.isEmpty())
			{
				meshName = meshComponent.mesh.info().name;
			}

			addAssetSelectWidget(meshName, AssetType::MESH, [&meshComponent](UUID uuid) {
				meshComponent.mesh = AssetWrapper<MeshCollection>(uuid);
			});

			if (ImGui::CollapsingHeader("Materials"))
			{
				int index = 0;
				for (auto& [id, mat] : meshComponent.m_material)
				{
					ImGui::PushID(&mat);

					std::string matName = "None";
					if (!mat.isEmpty())
					{
						matName = mat.info().name;
						if (matName.empty())
						{
							matName = "Material " + std::to_string(index);
						}
					}

					addAssetSelectWidget(matName, AssetType::MATERIAL, [&mat](UUID uuid) {
						mat = AssetWrapper<Material>(uuid);
					});

					++index;
					ImGui::PopID();
				}
			}


			
			
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

			if (currentProjection == CameraComponent::CamType::PERSPECTIVE)
			{
				static float tempFOVY;
				tempFOVY = cameraComponent.getFOVYInDegrees();
				if (ImGui::DragFloat("FOVY", &tempFOVY, .1f))
				{
					cameraComponent.setFOVY(tempFOVY);
				}
				ImGui::DragFloat("aspect", &cameraComponent.aspect, .01f);
				ImGui::DragFloat("z near", &cameraComponent.znear);
				ImGui::DragFloat("z far", &cameraComponent.zfar);
			}
			else if (currentProjection == CameraComponent::CamType::ORTHOGRAPHIC)
			{
				ImGui::DragFloat("left", &cameraComponent.ortho_left, .1f);
				ImGui::DragFloat("right", &cameraComponent.ortho_right, .1f);
				ImGui::DragFloat("bottom", &cameraComponent.ortho_bottom, .1f);
				ImGui::DragFloat("top", &cameraComponent.ortho_top, .1f);
			}
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

		displayComponent<DirectionalLight>("Directional Light", [](DirectionalLight& dLight) {
			ImGui::ColorEdit3("Color", glm::value_ptr(dLight.color));
		});

		displayComponent<PointLight>("Point Light", [](PointLight& pLight) {
			//auto& color = pLight.getColor();
			ImGui::ColorEdit3("Color", glm::value_ptr(pLight.color));

			//Attenuation& attenuation = pLight.getAttenuation();
			ImGui::LabelText("", "Attenuation");
			ImGui::DragFloat("constant", &pLight.attenuation.constant, 0.01f);
			ImGui::DragFloat("linear", &pLight.attenuation.linear, 0.01f);
			ImGui::DragFloat("quadratic", &pLight.attenuation.quadratic, 0.01f);
			//pLight.SetAttenuation(attenuation);
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

					addAssetSelectWidget(animationName, AssetType::ANIMATION, [&animator, name](UUID uuid) {
						animator.addAnimation(name, AssetWrapper<Animation>(uuid));
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

			if (ImGui::BeginTabBar("TerrainTabs"))
			{
				if (ImGui::BeginTabItem("Layers"))
				{
					addAssetSelectWidget("Terrain_Heightmap", AssetType::TEXTURE, [&](UUID uuid) {
						terrain = Terrain::generateTerrain(terrain.getWidth(), terrain.getHeight(), terrain.m_scale, AssetWrapper<Texture>(uuid));
						});

					

					static int* newWidth = 0;
					newWidth = &state.getCurrentEntityState().terrinTempWidth;
					static int* newHeight = 0;
					newHeight = &state.getCurrentEntityState().terrinTempHeight;

					// UI
					ImGui::DragInt("Width", newWidth, 1, 1, 8192);
					ImGui::DragInt("Height", newHeight, 1, 1, 8192);
					ImGui::DragInt("scale", &terrain.m_scale);

					// Check if rebuild is needed
					bool needsRebuild =
						*newWidth != terrain.getWidth() ||
						*newHeight != terrain.getHeight();

					// Warning text
					if (needsRebuild)
					{
						ImGui::TextColored(
							ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
							"Terrain must be rebuilt"
						);
					}

					// Build button
					ImGui::BeginDisabled(!needsRebuild);
					if (ImGui::Button("Build"))
					{
						terrain.resize(*newWidth, *newHeight);
					}
					ImGui::EndDisabled();

					ImGui::Separator();

					if (ImGui::CollapsingHeader("Materials"))
					{
						std::string matName = "None";
						if (!terrain.m_material.isEmpty())
						{
							matName = terrain.m_material.info().name;
						}

						addAssetSelectWidget(matName, AssetType::MATERIAL, [&terrain](UUID uuid) {
							terrain.m_material = AssetWrapper<Material>(uuid);
							});
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Foliage"))
				{
					if (!terrain.m_foliageField.isActive)
					{
						if (ImGui::Button("build"))
						{
							terrain.buildFoliage();
						}
					}
					else
					{
						ImGui::ColorEdit3("Bottom Color", (float*)&terrain.m_foliageField.colorA);
						ImGui::ColorEdit3("Top Color", (float*)&terrain.m_foliageField.colorB);
						ImGui::DragFloat("Density", &terrain.m_foliageField.globalDensity, 0.01f, 0.0f, 1.0f);



						bool isActive = EditorState::Instance().getActiveToolType() == EditorTool::Type::FoliagePainter;

						if (isActive)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
						}

						if (ImGui::Button("Foliage Painter"))
						{
							g_activeTerrain = &terrain;
							EditorState::Instance().setActiveEditorTool(
								isActive ? EditorTool::Type::None : EditorTool::Type::FoliagePainter
							);
						}

						if (isActive)
						{
							ImGui::PopStyleColor();
						}
					}



					//if (ImGui::Button("test"))
					//{
					//	g_activeTerrain = &terrain;
					//	EditorState::Instance().setActiveEditorTool(EditorTool::Type::FoliagePainter);

					//}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		});

		displayComponent<ScriptComponent>("Lua Script Component", [](ScriptComponent& script) {
			std::string scriptName = "None";
			if (!script.getScript().isEmpty())
			{
				scriptName = script.getScript().getUID();
			}

			addAssetSelectWidget(scriptName, AssetType::LUA_SCRIPT, [&script](UUID uuid) {
				script.script = AssetWrapper<LuaScript>(uuid);
			});
		});

		displayComponent<PostProcessComponent>("Post Process Component", [](PostProcessComponent& postProcessComponent) {
			std::string shaderName = "None";
			if (!postProcessComponent.shader.isEmpty())
			{
				shaderName = postProcessComponent.shader.getUID();
			}

			addAssetSelectWidget(shaderName, AssetType::SHADER, [&postProcessComponent](UUID uuid) {
				postProcessComponent.shader = AssetWrapper<Shader>(uuid);
			});
				
			if (!postProcessComponent.shader.isEmpty())
			{
				// Compile Button
				if (ImGui::Button("recompile"))
				{
					postProcessComponent.shader.resource()->recompile();
				}
			}

			});

		displayComponent<WaterBodyComponent>("Water Body Component", [](WaterBodyComponent& waterBody) {

			addSamplerEditWidget(waterBody.waterBodyNormal, ImVec2(50, 50), "Water Normal");

			ImGui::DragFloat2("Wave 1 Speed", glm::value_ptr(waterBody.wave1Speed), 0.001f, -10.0f, 10.0f);
			ImGui::DragFloat2("Wave 2 Speed", glm::value_ptr(waterBody.wave2Speed), 0.001f, -10.0f, 10.0f);
			ImGui::DragFloat2("Wave 3 Speed", glm::value_ptr(waterBody.wave3Speed), 0.001f, -10.0f, 10.0f);

			ImGui::DragFloat("Wave 1 Amplitude", &waterBody.wave1Amp, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Wave 2 Amplitude", &waterBody.wave2Amp, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Wave 3 Amplitude", &waterBody.wave3Amp, 0.01f, 0.0f, 10.0f);

			ImGui::ColorEdit3("Color A", glm::value_ptr(waterBody.colorA));
			ImGui::ColorEdit3("Color B", glm::value_ptr(waterBody.colorB));

			ImGui::DragFloat("Opacity", &waterBody.opacity, 0.01f, 0.0f, 1.0f);

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
				state.getSelectedEntity().addComponent<MeshRendererComponent>();
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
				auto meshComponent = state.getSelectedEntity().tryGetComponent<MeshRendererComponent>();
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

			//if (ImGui::MenuItem("Foliage"))
			//{
			//	auto& foliage = state.getSelectedEntity().addComponent<FoliageComponent>();
			//}

			if (ImGui::MenuItem("Lua Script"))
			{
				auto& script = state.getSelectedEntity().addComponent<ScriptComponent>();
				script.entity = state.getSelectedEntity();
			}

			if (ImGui::MenuItem("Post Process Effect"))
			{
				state.getSelectedEntity().addComponent<PostProcessComponent>();
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