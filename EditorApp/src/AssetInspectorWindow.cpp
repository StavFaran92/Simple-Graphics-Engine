#include "AssetInspectorWindow.h"

#include "EditorState.h"
#include "Widgets.h"

#include <fstream>
#include <sstream>

namespace
{
const char* textureFilterToString(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::Nearest: return "Nearest";
	case TextureFilter::Linear: return "Linear";
	default: return "Unknown";
	}
}

const char* textureWrapToString(TextureWrap wrap)
{
	switch (wrap)
	{
	case TextureWrap::Repeat: return "Repeat";
	case TextureWrap::Clamp: return "Clamp";
	case TextureWrap::Mirror: return "Mirror";
	default: return "Unknown";
	}
}

const char* textureTargetToString(TextureTarget target)
{
	switch (target)
	{
	case TextureTarget::TEXTURE_2D: return "2D";
	case TextureTarget::TEXTURE_3D: return "3D";
	case TextureTarget::TEXTURE_CUBE_MAP: return "CubeMap";
	default: return "Unknown";
	}
}

void openInVSCode(const std::string& path)
{
	// Windows-focused (this project currently targets Windows).
	std::string command = "code \"" + path + "\"";
	std::system(command.c_str());
}

std::string getShaderStagesLabel(const ShadersInfo& shadersInfo)
{
	std::vector<std::string> stages;
	if (!shadersInfo.vertexCode.empty()) stages.push_back("Vertex");
	if (!shadersInfo.fragmentCode.empty()) stages.push_back("Fragment");
	if (!shadersInfo.geometryCode.empty()) stages.push_back("Geometry");
	if (!shadersInfo.computeCode.empty()) stages.push_back("Compute");
	if (!shadersInfo.tessControlCode.empty()) stages.push_back("TessControl");
	if (!shadersInfo.tessEvaluationCode.empty()) stages.push_back("TessEvaluation");

	if (stages.empty())
		return "Unknown";

	std::ostringstream oss;
	for (size_t i = 0; i < stages.size(); ++i)
	{
		if (i > 0)
			oss << " + ";
		oss << stages[i];
	}
	return oss.str();
}

std::string getSourceSnippet(const std::string& source, int maxLines = 50)
{
	if (source.empty())
		return "Source code is empty.";

	std::istringstream input(source);
	std::ostringstream output;
	std::string line;
	int linesCount = 0;

	while (linesCount < maxLines && std::getline(input, line))
	{
		output << line << '\n';
		++linesCount;
	}

	if (input.good())
		output << "\n... (truncated)";

	return output.str();
}

std::string getFileSnippet(const std::string& filePath, int maxLines = 50)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		return "Failed to open script file.";

	std::ostringstream output;
	std::string line;
	int linesCount = 0;

	while (linesCount < maxLines && std::getline(file, line))
	{
		output << line << '\n';
		++linesCount;
	}

	if (!file.eof())
		output << "\n... (truncated)";

	return output.str();
}

void displayAnimationNodeTree(const AnimationData& animData, int nodeIndex)
{
	if (nodeIndex < 0 || nodeIndex >= (int)animData.nodes.size())
		return;

	const MeshNodeData& node = animData.nodes[nodeIndex];

	ImGuiTreeNodeFlags flags = node.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
	bool isOpen = ImGui::TreeNodeEx((void*)(intptr_t)nodeIndex, flags, "%s", node.name.c_str());

	if (isOpen)
	{
		for (int childIndex : node.children)
		{
			displayAnimationNodeTree(animData, childIndex);
		}
		ImGui::TreePop();
	}
}
}

void AssetInspectorWindow::display()
{
	ImGui::Begin("AssetInspector", nullptr, windowFlags &
		~ImGuiWindowFlags_NoScrollbar &
		~ImGuiWindowFlags_NoScrollWithMouse);

	auto& currentAssetEdit = EditorState::Instance().currentAssetEdit;
	if (currentAssetEdit.isEmpty())
	{
		ImGui::TextDisabled("No asset selected");
		ImGui::End();
		return;
	}

	const AssetRecord& info = currentAssetEdit.info();
	ImGui::Text("Name: %s", info.name.c_str());
	ImGui::Text("Type: %s", getAssetTypeAsStr(info.aType).c_str());
	ImGui::Separator();

	switch (info.aType)
	{
	case AssetType::TEXTURE:
	{
		TextureAssetRef texture = currentAssetEdit.as<TextureAsset>();
		if (texture.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load texture.");
			break;
		}

		TextureResourceRef texRes = texture.resource();
		if (!texRes.isEmpty())
		{
			const TextureData& data = texRes->getData();
			ImTextureID texID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(texRes->getID()));
			ImGui::Image(texID, ImVec2(128, 128));
			ImGui::Separator();
			ImGui::Text("Size: %d x %d", data.width, data.height);
			ImGui::Text("Channels: %d", data.channels);
			ImGui::Text("Target: %s", textureTargetToString(data.target));
			ImGui::Text("Filter: %s", textureFilterToString(data.filter));
			ImGui::Text("Wrap: %s", textureWrapToString(data.wrap));
			ImGui::Text("Mipmaps: %s", data.genMipMap ? "Enabled" : "Disabled");
			ImGui::Text("Internal Format: 0x%X", static_cast<int>(data.internalFormat));
			ImGui::Text("Format: 0x%X", static_cast<int>(data.format));
			ImGui::Text("Type: 0x%X", static_cast<int>(data.type));
		}
		break;
	}
	case AssetType::MATERIAL:
	{
		static MaterialAssetRef material;
		material = currentAssetEdit.as<MaterialAsset>();
		if (material.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load material.");
			break;
		}

		static MaterialDataWidget materialWidget;
		materialWidget.draw(material->data, [](const MaterialData&) {
			material.makeDirty();
		});
		break;
	}
	case AssetType::SHADER:
	{
		ShaderAssetRef shaderAsset = currentAssetEdit.as<ShaderAsset>();
		if (shaderAsset.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load shader.");
			break;
		}

		ShaderResourceRef shader = shaderAsset.resource();
		if (shader.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load shader resource.");
			break;
		}

		ImGui::Text("Shader Type: %s", getShaderStagesLabel(shader->getShadersInfo()).c_str());
		ImGui::Text("Override: %s", Shader::getShaderOverrideAsStr(shader->getShaderOverride()).c_str());

		if (ImGui::Button("Edit in VS Code"))
		{
			openInVSCode(info.getAbsolutePath());
		}
		ImGui::SameLine();
		if (ImGui::Button("Recompile"))
		{
			// Recompile from disk and refreshes the internal source/stage info.
			shader->recompile();
		}

		ImGui::Separator();
		ImGui::Text("Source snippet:");
		std::string snippet = getSourceSnippet(shader->getSourceCode(), 50);
		ImGui::BeginChild("ShaderSourceSnippet", ImVec2(0, 220), true);
		ImGui::PushTextWrapPos();
		ImGui::TextUnformatted(snippet.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndChild();
		break;
	}
	case AssetType::LUA_SCRIPT:
	{
		LuaScriptAssetRef scriptAsset = currentAssetEdit.as<LuaScriptAsset>();
		if (scriptAsset.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load Lua script.");
			break;
		}

		if (ImGui::Button("Edit in VS Code"))
		{
			openInVSCode(info.getAbsolutePath());
		}

		ImGui::Separator();
		ImGui::Text("Script snippet:");
		std::string snippet = getFileSnippet(info.getAbsolutePath(), 50);
		ImGui::BeginChild("LuaScriptSnippet", ImVec2(0, 220), true);
		ImGui::PushTextWrapPos();
		ImGui::TextUnformatted(snippet.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndChild();
		break;
	}
	case AssetType::MODEL:
	{
		ModelAssetRef modelAsset = currentAssetEdit.as<ModelAsset>();
		ModelResourceRef modelRes = modelAsset.resource();
		if (modelAsset.isEmpty() || modelRes.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load model resource.");
			break;
		}

		ImGui::Text("Meshes: %zu", modelRes->getMeshes().size());
		ImGui::Text("Vertices: %zu", modelRes->getNumOfVertices());
		ImGui::Text("Materials (slots): %d", modelRes->getMaterialCount());
		ImGui::Text("Bone offsets: %zu", modelRes->getBoneOffsets().size());
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Materials (UIDs, readonly)"))
		{
			for (const auto& [slot, matAsset] : modelAsset->m_materials)
			{
				if (matAsset.isEmpty())
					continue;

				ImGui::PushID(slot);
				std::string matUID = matAsset.info().uuid.str();
				std::string matName = matAsset->data.name;
				ImGui::Text("Slot %d: \n\tName: %s, UID: %s", slot, matName.c_str(), matUID.c_str());

				ImGui::PopID();
			}
		}
		break;
	}
	case AssetType::ANIMATION:
	{
		AnimationAssetRef animAsset = currentAssetEdit.as<AnimationAsset>();
		AnimationResourceRef animRes = animAsset.resource();
		if (animAsset.isEmpty() || animRes.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load animation resource.");
			break;
		}

		const float durationTicks = animRes->getDuration();
		const float ticksPerSecond = animRes->getTicksPerSecond();
		const float durationSeconds = ticksPerSecond > 0.f ? (durationTicks / ticksPerSecond) : 0.f;
		const AnimationData& animData = animRes->getData();

		ImGui::Text("Timeline: 0 .. %.3f ticks (%.3f sec)", durationTicks, durationSeconds);
		ImGui::Text("Ticks/sec: %.3f", ticksPerSecond);
		ImGui::Text("Nodes: %zu", animData.nodes.size());
		ImGui::Text("Bones: %zu", animData.bones.size());
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Node tree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (animData.nodes.empty())
			{
				ImGui::TextDisabled("No nodes.");
			}
			else
			{
				displayAnimationNodeTree(animData, 0);
			}
		}

		break;
	}
	default:
		ImGui::TextDisabled("Asset type not supported yet.");
		break;
	}

	ImGui::End();
}
