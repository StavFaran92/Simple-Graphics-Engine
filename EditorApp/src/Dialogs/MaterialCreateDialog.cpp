#include "MaterialCreateDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"
#include "render/MaterialDataParser.h"

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
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, matDesc);
		return true;
	}
	return false;
}

void MaterialCreateDialog::cancelContent()
{
}
