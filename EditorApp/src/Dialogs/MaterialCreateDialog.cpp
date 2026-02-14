#include "MaterialCreateDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"

MaterialCreateDialog::MaterialCreateDialog()
	: DialogBase("MaterialCreateDialog")
{
}

void MaterialCreateDialog::appearContent()
{
	m_tempMaterial = Material::create(MaterialRenderMode::Opaque);
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
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.name = m_uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		// TODO: makeResourceCreateDescriptor<MaterialCreateDescriptor>() with render mode + material data
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
		return true;
	}
	return false;
}

void MaterialCreateDialog::cancelContent()
{
}
