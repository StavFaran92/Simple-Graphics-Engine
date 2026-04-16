#include "AssetInspectorWindow.h"

#include "EditorState.h"
#include "Widgets.h"

#include <fstream>
#include <sstream>
#include <thread>

#ifndef _WIN32
#include <unistd.h>
#endif

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
#ifndef _WIN32
	if (fork() == 0) {
		execlp("code", "code", path.c_str(), (char*)nullptr);
		_exit(0);
	}
#else
	std::thread([path]() {
		std::string command = "code \"" + path + "\"";
		std::system(command.c_str());
		}).detach();
#endif
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
		MaterialAssetRef material = currentAssetEdit.as<MaterialAsset>();
		if (material.isEmpty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Failed to load material.");
			break;
		}

		static MaterialDataWidget materialWidget;
		materialWidget.draw(material->data, [&material](const MaterialData&) {
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
	default:
		ImGui::TextDisabled("Asset type not supported yet.");
		break;
	}

	ImGui::End();
}
