#include "ShaderCreateDialog.h"

#include "EditorState.h"
#include "memory/Assets.h"

ShaderCreateDialog::ShaderCreateDialog()
	: DialogBase("ShaderCreateDialog")
{
}

void ShaderCreateDialog::appearContent()
{
	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Shader", EditorState::Instance().getWorkingDir().path());
}

void ShaderCreateDialog::drawContent()
{
	m_uniqueName.draw();

	int currentIndex = 0;
	int index = 0;
	std::vector<const char*> comboItems;

	for (const auto& [key, value] : shaderOverrideToString)
	{
		if (key == m_shaderOverrideType)
			currentIndex = index;
		comboItems.push_back(value.c_str());
		++index;
	}

	ImGui::Text("Override Type");
	if (ImGui::Combo("##ShaderOverrideType", &currentIndex, comboItems.data(), static_cast<int>(comboItems.size())))
	{
		auto it = shaderOverrideToString.begin();
		std::advance(it, currentIndex);
		m_shaderOverrideType = it->first;
	}
	ImGui::Separator();
}

bool ShaderCreateDialog::acceptContent()
{
	if (!m_uniqueName.isValid())
	{
		return false;
	}

	AssetBuildDescriptor desc;
	desc.aType = AssetType::SHADER;
	desc.name = m_uniqueName.name;
	desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
	ShaderCreateDescriptor shaderCreateDesc;
	shaderCreateDesc.shaderOverride = m_shaderOverrideType;
	Engine::get()->getSubSystem<Assets>()->createAsset(desc, shaderCreateDesc);
	return true;
}

void ShaderCreateDialog::cancelContent()
{
}
