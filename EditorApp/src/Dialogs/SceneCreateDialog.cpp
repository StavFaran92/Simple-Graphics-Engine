#include "SceneCreateDialog.h"
#include "EditorState.h"
#include "memory/Assets.h"

SceneCreateDialog::SceneCreateDialog()
	: DialogBase("SceneCreateDialog")
{
}

void SceneCreateDialog::appearContent()
{
	m_tempScene = Scene::create();
	m_uniqueName.name = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName("New Scene", EditorState::Instance().getWorkingDir().path());
}

void SceneCreateDialog::drawContent()
{
	m_uniqueName.draw();
	ImGui::Separator();
	//m_matData.draw(m_tempMaterial);
	ImGui::Separator();
}

bool SceneCreateDialog::acceptContent()
{
	if (m_uniqueName.isValid())
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::SCENE;
		desc.name = m_uniqueName.name;
		desc.targetDirectory = EditorState::Instance().getWorkingDir().path();
		SceneCreateDescriptor sceneDesc;
		auto scene = Engine::get()->getSubSystem<Assets>()->createAsset(desc, sceneDesc).as<SceneAsset>();
		Engine::get()->getContext()->addScene(scene);
		return true;
	}
	return false;
}

void SceneCreateDialog::cancelContent()
{
}
