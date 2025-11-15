#include "MaterialCreateDialog.h"

void MaterialCreateDialog::appearContent()
{
	m_tempMaterial = Material::create();
	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Material");
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
		Engine::get()->getSubSystem<Assets>()->createAsset(m_tempMaterial, desc);
		return true;
	}
	return false;
}

void MaterialCreateDialog::cancelContent()
{
}
