#include "ModelImportDialog.h"
#include "EditorState.h"

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

		AssetCreateDescriptor desc;
		desc.name = uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		auto mesh = MeshGroupAsset::import(filepath.m_filepath, desc);

		auto& meshRenderer = entity.addComponent<MeshRendererComponent>(mesh);

		auto& materials = mesh->getImportedMaterials();

		// TODO fix
		//for (auto& [idx, m] : materials)
		//{
		//	meshRenderer.setMaterial(idx, m);

		//}

		ResourceWrapper<Prefab> prefab = Prefab::create(entity);

		AssetCreateDescriptor aInfo;
		aInfo.name = uniqueName.name + "_PREFAB";
		aInfo.targetDirectory = EditorState::Instance().getWorkingDir().path();
		PrefabAsset::create(prefab, aInfo);

		entity.remove();

		return true;
	}
	return false;
}

void ModelImportDialog::cancelContent()
{
}
