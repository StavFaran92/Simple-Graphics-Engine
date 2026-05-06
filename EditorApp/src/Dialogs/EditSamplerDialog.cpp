#include "EditSamplerDialog.h"

#include "imgui.h"
#include "EditorState.h"
#include "Widgets.h"
#include "core/Logger.h"

EditSamplerDialog::EditSamplerDialog()
	: DialogBase("EditSamplerDialog")
{
}

void EditSamplerDialog::appearContent()
{
	if (sampler)
	{
		m_previousSampler = std::make_shared<TextureSamplerAsset>(*sampler);
	}
}

void EditSamplerDialog::drawContent()
{
	if (!sampler)
	{
		ImGui::Text("No sampler selected.");
		return;
	}

	ImGui::Text("Texture");
	addTextureEditWidget("Texture", sampler->texture, ImVec2{ 150, 150 }, [this](UUID uuid) {
		sampler->texture = TextureAssetRef(uuid);
	});

	ImGui::Spacing();

	int* currentChannelMask[4];
	currentChannelMask[0] = &sampler->state.channelMaskR;
	currentChannelMask[1] = &sampler->state.channelMaskG;
	currentChannelMask[2] = &sampler->state.channelMaskB;
	currentChannelMask[3] = &sampler->state.channelMaskA;

	for (int i = 0; i < sampler->state.channelCount; i++)
	{
		ImGui::PushID(&currentChannelMask[i]);
		displayChannelSelectWidget(currentChannelMask[i]);
		ImGui::PopID();
	}

	ImGui::Spacing();

	ImGui::DragFloat("xoffset", &sampler->state.xOffset, .1f);
	ImGui::DragFloat("yoffset", &sampler->state.yOffset, .1f);

	ImGui::Spacing();

	ImGui::DragFloat("xScale", &sampler->state.xScale, .1f);
	ImGui::DragFloat("yScale", &sampler->state.yScale, .1f);

	ImGui::Separator();
}

bool EditSamplerDialog::acceptContent()
{
	if (sampler && !sampler->texture.isEmpty() && onAcceptCB)
	{
		onAcceptCB(sampler->texture.getUID());
	}
	return true;
}

void EditSamplerDialog::cancelContent()
{
	if (sampler && m_previousSampler)
	{
		*sampler = *m_previousSampler;
	}
}
