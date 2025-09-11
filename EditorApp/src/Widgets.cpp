#include "Widgets.h"

#include "Dialogs.h"

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(std::string uuid)> callback)
{
	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(textureID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		assetTextureSelectCB = callback;
	}

	displayTextureSelectDialog();
}

void addTextureEditWidget(Resource<Texture> texture, ImVec2 size, std::function<void(std::string uuid)> callback)
{
	int texID = 0;
	if (!texture.isEmpty())
	{
		texID = texture.get()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		ImGui::OpenPopup("EditTexturePopup");
		assetTextureSelectCB = callback;
	}

	displayTextureSelectDialog();
}

void addTextureEditWidget(Resource<Material> mat, const std::string& name, Texture::TextureType ttype)
{
	Resource<Texture> tex = Resource<Texture>::empty;
	if (mat->hasTexture(ttype))
	{
		tex = mat->getSampler(ttype)->texture;
	}

	addTextureEditWidget(tex, { 20, 20 }, [=](std::string uuid) {
		mat.get()->setTexture(ttype, Resource<Texture>(uuid));
		});

	ImGui::SameLine();

	ImGui::Text(name.c_str());
}

void addSamplerEditWidget(Resource<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype)
{
	ImGui::PushID(name.c_str());

	int texID = 0;
	auto sampler = mat->getSampler(ttype);

	if (!sampler->texture.isEmpty())
	{
		texID = sampler->texture.get()->getID();
	}

	if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texID), size))
	{
		ImGui::OpenPopup("EditSamplerPopup");
		g_selectedSampler = sampler;
		g_previousSampler = std::make_shared<TextureSampler>(*sampler.get());
	}

	if (ImGui::BeginPopup("EditSamplerPopup"))
	{
		if (!g_selectedSampler)
		{
			logError("Selected sampler cannot be null.");
			ImGui::EndPopup();
			return;
		}
		auto assets = Engine::get()->getSubSystem<Assets>();

		ImGui::Text("Texture");
		addTextureEditWidget(g_selectedSampler->texture, ImVec2{ 150, 150 }, [=](std::string uuid) {
			g_selectedSampler->texture = Resource<Texture>(uuid);
			});

		ImGui::Spacing();

		static int* currentChannelMask[4];

		currentChannelMask[0] = &g_selectedSampler->channelMaskR;
		currentChannelMask[1] = &g_selectedSampler->channelMaskG;
		currentChannelMask[2] = &g_selectedSampler->channelMaskB;
		currentChannelMask[3] = &g_selectedSampler->channelMaskA;

		for (int i = 0; i < g_selectedSampler->channelCount; i++)
		{
			ImGui::PushID(&currentChannelMask[i]);
			displayChannelSelectWidget(currentChannelMask[i]);
			ImGui::PopID();
		}

		ImGui::Spacing();

		ImGui::DragFloat("xoffset", &g_selectedSampler->xOffset, .1f);
		ImGui::DragFloat("yoffset", &g_selectedSampler->yOffset, .1f);

		ImGui::Spacing();

		ImGui::DragFloat("xScale", &g_selectedSampler->xScale, .1f);
		ImGui::DragFloat("yScale", &g_selectedSampler->yScale, .1f);

		ImGui::Separator();

		if (ImGui::Button("OK"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel"))
		{
			mat->setSampler(ttype, g_previousSampler);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::Text(name.c_str());

	ImGui::PopID();

}

void displayChannelSelectWidget(int*& currentChannel)
{
	static const char* channelMaskOptions[] = { "None", "R", "G", "B", "A" };
	if (ImGui::BeginCombo("##channelMask", channelMaskOptions[*currentChannel])) // Label for the combo box
	{
		for (int i = 0; i < IM_ARRAYSIZE(channelMaskOptions); i++)
		{
			bool isSelected = (*currentChannel == i);
			if (ImGui::Selectable(channelMaskOptions[i], isSelected))
			{
				*currentChannel = i; // Update selected index
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus(); // Set focus to the current item
		}
		ImGui::EndCombo();
	}
}