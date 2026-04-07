#include "MaterialEditDialog.h"

#include "EditorState.h"

MaterialEditDialog::MaterialEditDialog()
	: DialogBase("MaterialEditDialog")
{
}

void MaterialEditDialog::appearContent()
{
	if (!EditorState::Instance().selectedMaterialForEdit.isEmpty())
	{
		m_previousMaterialData = EditorState::Instance().selectedMaterialForEdit->data;
	}
}

void MaterialEditDialog::drawContent()
{
	auto& mat = EditorState::Instance().selectedMaterialForEdit;
	if (mat.isEmpty())
	{
		ImGui::Text("No material selected");
		return;
	}

	if (m_materialData.draw(mat->data))
	{
		mat.makeDirty();
	}
	ImGui::Separator();
}

bool MaterialEditDialog::acceptContent()
{
	return true;
}

void MaterialEditDialog::cancelContent()
{
	auto& mat = EditorState::Instance().selectedMaterialForEdit;
	if (!mat.isEmpty())
	{
		mat->data = m_previousMaterialData;
		mat.makeDirty();
	}
}
