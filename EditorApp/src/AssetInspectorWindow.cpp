#include "AssetInspectorWindow.h"

#include "EditorState.h"
#include "Widgets.h"

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
	default:
		ImGui::TextDisabled("Asset type not supported yet.");
		break;
	}

	ImGui::End();
}
