#include "ModelImportDialog.h"

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
		uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(filename);
	}
	ImGui::Separator();
}

bool ModelImportDialog::acceptContent()
{
	if (uniqueName.isValid())
	{
		auto entity = Engine::get()->getContext()->getActiveScene()->createEntity(uniqueName.name);
		entity.addComponent<RenderableComponent>();

		ModelImportSettings desc;
		desc.name = uniqueName.name;
		auto mesh = MeshCollection::import(filepath.m_filepath, desc);

		entity.addComponent<MeshComponent>().mesh = mesh;

		auto& materials = MeshCollection::getLastLoadedMaterials();

		auto& materialComponent = entity.addComponent<MaterialComponent>();
		for (auto& [idx, m] : materials)
		{
			materialComponent.setMaterial(idx, m);
		}

		ResourceWrapper<Prefab> prefab = Prefab::create(entity);

		AssetCreateDescriptor aInfo;
		aInfo.name = uniqueName.name + "_PREFAB";
		aInfo.aType = AssetType::PREFAB;
		Engine::get()->getSubSystem<Assets>()->createAsset(prefab, aInfo);

		entity.remove();

		return true;
	}
	return false;
}

void ModelImportDialog::cancelContent()
{
}
