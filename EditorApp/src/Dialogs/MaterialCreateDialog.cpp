#include "MaterialCreateDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"
#include "render/MaterialDataParser.h"
#include "commands/CommandStack.h"
#include "commands/AssetCommands.h"

MaterialCreateDialog::MaterialCreateDialog()
	: DialogBase("MaterialCreateDialog")
{
}

void MaterialCreateDialog::appearContent()
{
	m_tempData = MaterialData();
	MaterialDataParser::parse(m_tempData);

	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Material", EditorState::Instance().getWorkingDir().path());
}

void MaterialCreateDialog::drawContent()
{
	m_uniqueName.draw();
	ImGui::Separator();

	m_matData.draw(m_tempData, nullptr);
	ImGui::Separator();
}

bool MaterialCreateDialog::acceptContent()
{
	if (m_uniqueName.isValid())
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.name = m_uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		// TODO: MaterialCreateDescriptor with render mode + material data
		MaterialCreateDescriptor matDesc;
		matDesc.data = m_tempData;
		AssetRef<Asset> material = Engine::get()->getSubSystem<Assets>()->createAsset(desc, matDesc);
		if (!material.isEmpty())
			CommandStack::Instance().record(std::make_unique<CreateAssetCmd>(material, std::make_unique<MaterialCreateDescriptor>(matDesc), "Create " + desc.name));
		return true;
	}
	return false;
}

void MaterialCreateDialog::cancelContent()
{
}
