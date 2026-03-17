#include "ModelImportDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"
#include "geometry/Model.h"

ModelImportDialog::ModelImportDialog()
	: DialogBase("ModelImportDialog")
{
}

void ModelImportDialog::appearContent()
{
	uniqueName.clear();
	filepath.clear();
}

void ModelImportDialog::drawContent()
{
	uniqueName.draw();

	filepath.draw();
	if (filepath.accept())
	{
		std::filesystem::path path(filepath.m_filepath);
		std::string filename = path.filename().stem().string();
		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(filename, EditorState::Instance().getWorkingDir().path());
	}
	ImGui::Separator();
}

bool ModelImportDialog::acceptContent()
{
	if (uniqueName.isValid())
	{
		auto entity = Engine::get()->getContext()->getActiveScene()->createEntity(uniqueName.name);
		entity.addComponent<RenderableComponent>();

		AssetBuildDescriptor desc;
		desc.aType = AssetType::MESH;
		desc.name = uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		ModelLoadDescriptor modelLoadDesc;
		modelLoadDesc.sourcePath = filepath.m_filepath;
		auto mesh = Engine::get()->getSubSystem<Assets>()->importAsset(desc, modelLoadDesc).as<ModelAsset>();

		auto& meshRenderer = entity.addComponent<MeshRendererComponent>(mesh);

		for (auto&[slot, mat] : mesh->m_materials)
		{
			meshRenderer.setMaterial(slot, mat);
		}

		AssetBuildDescriptor aInfo;
		aInfo.aType = AssetType::PREFAB;
		aInfo.name = uniqueName.name + "_PREFAB";
		aInfo.targetDirectory = EditorState::Instance().getWorkingDir().path();
		PrefabCreateDescriptor prefabDesc;
		prefabDesc.data = Prefab::serializeEntityToPrefabData(entity); // todo fix, should be inner
		Engine::get()->getSubSystem<Assets>()->createAsset(aInfo, prefabDesc);

		entity.remove();

		return true;
	}
	return false;
}

void ModelImportDialog::cancelContent()
{
}
