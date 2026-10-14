#include "MaterialEditDialog.h"

#include "EditorState.h"
#include "Thumbnails/ThumbnailCache.h"

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

	static auto onChangedCB = [](const MaterialData&) {
		EditorState::Instance().selectedMaterialForEdit.makeDirty();
		ThumbnailCache::Instance().invalidate(EditorState::Instance().selectedMaterialForEdit.getUID());
	};

	m_materialData.draw(mat->data, onChangedCB);
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
		ThumbnailCache::Instance().invalidate(mat.getUID());
	}
}
