#include "MaterialCreateDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"

MaterialCreateDialog::MaterialCreateDialog()
	: DialogBase("MaterialCreateDialog")
{
}

void MaterialCreateDialog::appearContent()
{
	AssetBuildDescriptor desc;
	desc.aType = AssetType::MATERIAL;
	desc.name = "SGE_MATERIAL_TEMP";
	MaterialCreateDescriptor matDesc;
	matDesc.aType = desc.aType;
	matDesc.data.renderMode = MaterialRenderMode::Opaque;
	m_tempMaterial = Engine::get()->getSubSystem<Assets>()->createAsset(desc, matDesc).as<MaterialAsset>();

	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Material", EditorState::Instance().getWorkingDir().path());
}

void MaterialCreateDialog::drawContent()
{
	m_uniqueName.draw();
	ImGui::Separator();
	m_matData.draw(m_tempMaterial);
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
		matDesc.aType = desc.aType;
		matDesc.data = m_tempMaterial->data;
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, matDesc);
		return true;
	}
	return false;
}

void MaterialCreateDialog::cancelContent()
{
}
