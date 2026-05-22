#include "InspectorWindow.h"

#include <imgui_stdlib.h>

#include "Tables.h"
#include "EntityState.h"
#include "EditorState.h"
#include "NativeScriptsLoader.h"
#include "TerrainPaintTool.h"
#include "Widgets.h"
#include "TerrainPaintTool.h"
#include "AnimationGraphWindow.h"
#include "AnimationViewerWindow.h"

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
			ImGui::LabelText("", "Actor Type");
			ImGui::Combo("##ActorType", (int*)&rBody.rigidBodyType, rigidyBodyTypesStrList, IM_ARRAYSIZE(rigidyBodyTypesStrList));

			ImGui::LabelText("", "Collision Type");
			ImGui::Combo("##CollisionType", (int*)&rBody.collisionType, physicsCollisionTypesStrList, IM_ARRAYSIZE(physicsCollisionTypesStrList));

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

			ImGui::LabelText("", "Shape");
			if (ImGui::Combo("##Shape", (int*)&rBody.shapeType, colliderTypeNames, IM_ARRAYSIZE(colliderTypeNames))) {
				switch (rBody.shapeType) {
				case CollisionShape::NONE: rBody.collider = 0; break;
				case CollisionShape::BOX: rBody.collider = std::make_shared<CollisionBox>(); break;
				case CollisionShape::SPHERE: rBody.collider = std::make_shared<CollisionSphere>(); break;
				case CollisionShape::TERRAIN: rBody.collider = std::make_shared<CollisionTerrain>(); break;
				case CollisionShape::MESH: rBody.collider = std::make_shared<CollisionMesh>(); break;
				case CollisionShape::CAPSULE: rBody.collider = std::make_shared<CollisionCapsule>(); break;
				}
			}

			CollisionShape type = rBody.shapeType;

			if (type == CollisionShape::NONE)
			{
				return;
			}

			ImGui::LabelText("", "Layer");
			ImGui::Combo("##LayerMask", (int*)&rBody.collider->layerMask, layerMaskList, IM_ARRAYSIZE(layerMaskList));

			switch (type) {
			case CollisionShape::BOX:
				if (auto* box = dynamic_cast<CollisionBox*>(rBody.collider.get())) {
					ImGui::DragFloat3("Extents", &box->extents.x, .1f);
				}
				break;
			case CollisionShape::SPHERE:
				if (auto* sphere = dynamic_cast<CollisionSphere*>(rBody.collider.get())) {
					ImGui::DragFloat("Radius", &sphere->radius, .1f);
				}
				break;
			case CollisionShape::CAPSULE:
				if (auto* capsule = dynamic_cast<CollisionCapsule*>(rBody.collider.get())) {
					ImGui::DragFloat("Radius", &capsule->radius, .1f);
					ImGui::DragFloat("Height", &capsule->halfHeight, .1f);
				}
				break;
			case CollisionShape::TERRAIN:
				ImGui::TextDisabled("Terrain collider has no editable parameters.");
				break;
			case CollisionShape::MESH:
				if (auto* mesh = dynamic_cast<CollisionMesh*>(rBody.collider.get())) {
					ImGui::Checkbox("Convex", &mesh->isConvex);
					// Optional: display mesh ResourceWrapper name, etc.
				}
				break;
			}

			ImGui::DragFloat3("Offset", &rBody.offset.x, .1f);
		});

		displayComponent<PlayerController>("Player Controller", [](PlayerController& controller) {
				ImGui::DragFloat("Radius", &controller.radius, .1f);
				ImGui::DragFloat("Height", &controller.height, .1f);
				ImGui::DragFloat3("Offset", &controller.offset.x, .1f);
			});

		displayComponent<MeshRendererComponent>("Mesh Renderer", [](MeshRendererComponent& meshComponent) {
			if (meshComponent.mesh.isEmpty()) return;

			BEGIN_IMGUI_TABLE("Mesh");

			addTableRow("Vertices count:", [&](std::string id) {
				rightAlignedText(std::to_string((int)meshComponent.mesh.resource()->getNumOfVertices()));
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

			addAssetSelectWidget(meshName, AssetType::MODEL, [&meshComponent](UUID uuid) {
				meshComponent.mesh = ModelAssetRef(uuid);
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
						mat = MaterialAssetRef(uuid);
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
				EditorState::Instance().scriptSelectCB = [&nsc](const std::string& selectedScript) {
					nsc.script = std::shared_ptr<ScriptableEntity>(NativeScriptsLoader::instance->getScript(selectedScript));
				};
				EditorState::Instance().setState("ScriptSelectDialog", true);
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
			addTextureEditWidget("Skybox", skybox.originalImage, { 50, 50 }, [&](UUID uuid) {
				skybox.setSkybox(TextureAssetRef(uuid));
			});

			// Compile Button
			if (ImGui::Button("Build"))
			{
				skybox.build();
			}
		});

		displayComponent<ImageComponent>("Image", [](ImageComponent& image) {
			addTextureEditWidget("Image", image.image, { 50, 50 }, [&](UUID uuid) {
				image.image = TextureAssetRef(uuid);
				});
			ImGui::DragFloat("posX", &image.position.x);
			ImGui::DragFloat("posY", &image.position.y);
			ImGui::DragFloat("sizeX", &image.size.x);
			ImGui::DragFloat("sizeY", &image.size.y);
			});

		displayComponent<Animator>("Animator", [](Animator& animator) {

			EntityState& eState = state.getCurrentEntityState();

			if (ImGui::Button("Edit Graph"))
			{
				AnimationGraphWindow::open(&animator);
			}

			ImGui::Separator();
			ImGui::Text("Animations");

			const auto& animations = animator.getAllAnimations();

			// Sync rename buffers size
			while (eState.animationRenameBuffers.size() < animations.size())
				eState.animationRenameBuffers.push_back(animations[eState.animationRenameBuffers.size()].name);

			for (int index = 0; index < (int)animations.size(); ++index)
			{
				const AnimationEntry& entry = animations[index];

				ImGui::PushID(index);

				bool isActive = (index == animator.getCurrentAnimationID());
				if (isActive)
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));

				if (ImGui::CollapsingHeader(entry.name.c_str()))
				{
					ImGui::Indent();

					// Rename
					float width = ImGui::GetContentRegionAvail().x;
					ImGui::SetNextItemWidth(width);
					ImGui::InputText("##Name", &eState.animationRenameBuffers[index]);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						const std::string newName = eState.animationRenameBuffers[index];
						if (newName != entry.name)
							animator.getAnimation(entry.name)->name = newName;
					}

					// Asset picker
					AnimationEntry* entryPtr = animator.getAnimation(entry.name);
					std::string animUID = entry.animation.isEmpty() ? EMPTY_UUID : entry.animation.getUID();
					addAssetSelectWidget(animUID, AssetType::ANIMATION, [entryPtr](UUID uuid) {
						entryPtr->animation = AnimationAssetRef(uuid);
					});

					// Playback speed
					ImGui::DragFloat("Playback Speed", &entryPtr->playbackSpeed, 0.01f, 0.0f, 10.0f);

					// Set active
					if (!isActive && ImGui::Button("Play"))
						animator.playAnimation(entry.name);

					ImGui::SameLine();
					if (ImGui::Button("View"))
						AnimationViewerWindow::open(state.getSelectedEntity(), entry.animation);

					ImGui::Unindent();
				}

				if (isActive)
					ImGui::PopStyleColor();

				ImGui::PopID();
			}

			if (ImGui::Button("+ Add Animation"))
			{
				AnimationEntry newEntry;
				newEntry.name = "Animation_" + std::to_string(animations.size());
				newEntry.playbackSpeed = 1.0f;
				animator.addAnimation(newEntry);
				eState.animationRenameBuffers.push_back(newEntry.name);
			}

		});

		displayComponent<Terrain>("Terrain", [](Terrain& terrain) {

			if (ImGui::BeginTabBar("TerrainTabs"))
			{
				if (ImGui::BeginTabItem("Properties"))
				{
					

					

					static int* newWidth = 0;
					newWidth = &state.getCurrentEntityState().terrinTempWidth;
					static int* newHeight = 0;
					newHeight = &state.getCurrentEntityState().terrinTempHeight;

					// UI
					ImGui::DragInt("Width", newWidth, 1, 1, 8192);
					ImGui::DragInt("Height", newHeight, 1, 1, 8192);
					ImGui::DragFloat("scale", &terrain.m_scale);

					static bool useCustomHeightmap = false;
					ImGui::Checkbox("Custom Heightmap", &useCustomHeightmap);
					if (useCustomHeightmap)
					{
						addAssetSelectWidget("Terrain_Heightmap", AssetType::TEXTURE, [&](UUID uuid) {
							terrain.setHeightmap(TextureAssetRef(uuid));
						});
					}

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

							if (terrain.m_material->getMaterialRenderMode() != MaterialRenderMode::Terrain)
							{
								ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Material type is not Terrain compatible!");
							}
						}

						addAssetSelectWidget(matName, AssetType::MATERIAL, [&terrain](UUID uuid) {
							terrain.m_material = MaterialAssetRef(uuid);
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

				if (ImGui::BeginTabItem("Painter"))
				{
					static const char* toolNames[] = { "None", "Deformer", "Painter" };
					static const EditorTool::Type toolTypes[] = { EditorTool::Type::None, EditorTool::Type::TerrainDeformer, EditorTool::Type::TerrainPainter };

					EditorTool::Type activeTool = EditorState::Instance().getActiveToolType();
					int currentIndex = 0;
					for (int i = 0; i < IM_ARRAYSIZE(toolTypes); i++)
					{
						if (toolTypes[i] == activeTool)
						{
							currentIndex = i;
							break;
						}
					}

					if (ImGui::BeginCombo("Tool", toolNames[currentIndex]))
					{
						for (int i = 0; i < IM_ARRAYSIZE(toolNames); i++)
						{
							bool isSelected = (i == currentIndex);
							if (ImGui::Selectable(toolNames[i], isSelected))
							{
								g_activeTerrain = &terrain;
								EditorState::Instance().setActiveEditorTool(toolTypes[i]);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if (EditorState::Instance().getActiveToolType() == EditorTool::Type::TerrainPainter)
					{
						auto terrainPainter = std::static_pointer_cast<TerrainPaintTool>(
							EditorState::Instance().getEditorTool(EditorTool::Type::TerrainPainter));
						
						

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Layers");
						ImGui::Spacing();

						int layerCount = terrain.getLayerCount();

						// Clamp selection in case layers changed
						terrainPainter->clampSelection(layerCount);

						for (int i = 0; i < layerCount; i++)
						{
							ImGui::PushID(i);

							bool isSelected = (i == terrainPainter->getSelectedLayer());
							ImGui::Bullet();
							ImGui::SameLine();

							const LayerMask& layer = terrain.getLayer(i);
							std::string label = layer.name;

							if (ImGui::Selectable(label.c_str(), isSelected))
								terrainPainter->selectLayer(i);

							ImGui::PopID();
						}

						ImGui::Spacing();

						// Add button
						if (ImGui::Button("+"))
						{
							terrain.addLayer();
						}

						ImGui::SameLine();

						// Remove button — disabled when only 1 layer remains
						ImGui::BeginDisabled(layerCount <= 1);
						if (ImGui::Button("-"))
						{
							terrain.removeLayer(terrainPainter->getSelectedLayer());
							terrainPainter->clampSelection(terrain.getLayerCount());
						}
						ImGui::EndDisabled();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Brush");
						ImGui::Spacing();

						static const char* drawModeNames[] = { "Draw", "Erase" };
						int drawMode = (int)terrainPainter->getDrawMode();
						if (ImGui::Combo("Mode", &drawMode, drawModeNames, IM_ARRAYSIZE(drawModeNames)))
							terrainPainter->setDrawMode((TerrainPaintTool::DrawMode)drawMode);

						float radius = terrainPainter->getBrushRadius();
						if (ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f))
							terrainPainter->setBrushRadius(radius);

						float strength = terrainPainter->getBrushStrength();
						if (ImGui::SliderFloat("Strength", &strength, 0.0f, 1.0f))
							terrainPainter->setBrushStrength(strength);

						

					} // TerrainPainter active

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
				script.script = LuaScriptAssetRef(uuid);
			});
		});

		displayComponent<PostProcessComponent>("Post Process Component", [](PostProcessComponent& postProcessComponent) {
			std::string shaderName = "None";
			if (!postProcessComponent.shader.isEmpty())
			{
				shaderName = postProcessComponent.shader.getUID();
			}

			addAssetSelectWidget(shaderName, AssetType::SHADER, [&postProcessComponent](UUID uuid) {
				postProcessComponent.shader = ShaderAssetRef(uuid);
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

		displayComponent<VolumetricCloudsComponent>("Volumetric Clouds Component", [](VolumetricCloudsComponent& clouds) {
			ImGui::DragFloat("March size", &clouds.marchSize, 0.001f, 0.01f, 1.0f);
			});

		displayComponent<VolumeComponent>("Volume Component", [](VolumeComponent& volume) {

			std::string meshName = "None";
			if (!volume.mesh.isEmpty())
			{
				meshName = volume.mesh.info().name;
			}

			addAssetSelectWidget(meshName, AssetType::MODEL, [&volume](UUID uuid) {
				volume.mesh = ModelAssetRef(uuid);
				});

			std::string matName = "None";
			if (!volume.material.isEmpty())
			{
				matName = volume.material.info().name;
				if (matName.empty())
				{
					matName = "Material";
				}
			}

			addAssetSelectWidget(matName, AssetType::MATERIAL, [&volume](UUID uuid) {
				volume.material = MaterialAssetRef(uuid);
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
				auto& img = state.getSelectedEntity().addComponent<ImageComponent>(BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE));
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

			if (ImGui::MenuItem("Volume"))
			{
				state.getSelectedEntity().addComponent<VolumeComponent>();
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