#include "EntryPoint.h"
#include "sge.h"

#include "FlyCamera.h"

class CameraScript : public ScriptableEntity
{
	virtual void onCreate() override
	{
		Engine::get()->getWindow()->lockMouse();

		m_flyCamera = std::make_shared<FlyCamera>(entity, 0, 0, 3, 1);
	}

	virtual void onUpdate(float deltaTime) {
		m_flyCamera->onUpdate(deltaTime);
	};

	std::shared_ptr<FlyCamera> m_flyCamera;
};

class Sandbox : public Application
{
public:

	void start() override
	{
		// set Editor camera as active camera
		auto editorCamera = Engine::get()->getContext()->getActiveScene()->createEntity("Editor Camera");
		editorCamera.addComponent<CameraComponent>();
		editorCamera.addComponent<NativeScriptComponent>().bind<EditorCamera>();
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);

		Entity terrainEnt = Engine::get()->getContext()->getActiveScene()->createEntity("Terrain");
		//auto terrain = Terrain::generateTerrain(100, 100, 64, "C:/Users/Stav/Downloads/HeightMap.png");
		auto& terrain = terrainEnt.addComponent<Terrain>(Terrain::generateTerrain(100, 100, 10, "C:/Users/Stav/Pictures/PerlinExample.png"));

		terrain.m_textureCount = 3;

		{
			auto texture = Texture::importTexture2D("C:/Users/Stav/Downloads/tex/Grass003_1K-JPG_Color.jpg");
			terrain.setTexture(0, texture);
			terrain.setTextureBlend(0, .10f);
			terrain.setTextureScaleX(0, 10.f);
			terrain.setTextureScaleY(0, 10.f);
		}

		{
			auto texture = Texture::importTexture2D("C:/Users/Stav/Downloads/tex/Grass004_1K-JPG_Color.jpg");
			terrain.setTexture(1, texture);
			terrain.setTextureBlend(1, .20f);
			terrain.setTextureScaleX(1, 15.f);
			terrain.setTextureScaleY(1, 15.f);
		}

		{
			auto texture = Texture::importTexture2D("C:/Users/Stav/Downloads/tex/Ground037_1K-JPG_Color.jpg");
			terrain.setTexture(2, texture);
			terrain.setTextureBlend(2, .50f);
			terrain.setTextureScaleX(2, 13.f);
			terrain.setTextureScaleY(2, 13.f);
		}

		//terrain.getHeightAtPoint(1, 1);

		//auto textureSnow = Engine::get()->getSubSystem<Assets>()->importTexture2D("C:/Users/Stav/Downloads/tex/Snow008A_1K-JPG_Color.jpg");
		//terrain.setTexture(3, textureSnow);
		//terrain.setTextureBlend(3, .80f);




		//Entity zombieGirl = Engine::get()->getContext()->getActiveScene()->createEntity("Zombie girl");
		//auto mesh = Engine::get()->getSubSystem<ModelImporter>()->import("C:/Users/Stav/Downloads/FPS Zombie/Scary Zombie Pack/zombiegirl.fbx");
		//zombieGirl.addComponent<RenderableComponent>();
		//zombieGirl.addComponent<MaterialComponent>();
		//zombieGirl.addComponent<MeshComponent>().mesh = mesh;

		////zombieGirl.getComponent<Transformation>().translate({ 0, 0, 10 });
		//zombieGirl.getComponent<Transformation>().scale({ 0.05, 0.05, 0.05 });

		//auto animation = Engine::get()->getSubSystem<AnimationLoader>()->import("C:/Users/Stav/Downloads/FPS Zombie/Scary Zombie Pack/Zombie Walk.fbx");

		//zombieGirl.addComponent<Animator>(animation);

		//Engine::get()->getContext()->getActiveScene()->startSimulation();

		//camTransform.setLocalPosition(playerTransform.getLocalPosition());
		//camTransform.translate({0,2,0});

		//camera.setParent(player);

		//auto box1 = ShapeFactory::createBox(Engine::get()->getContext()->getActiveScene().get());
		//auto& nsc = box1.addComponent<NativeScriptComponent>();
		//nsc.bind<CustomBoxBehaviour>();
		//box1->getComponent<Transformation>().translate({1, 0,0});
		//auto box2 = ShapeFactory::createBox(Engine::get()->getContext()->getActiveScene().get());


		//auto importer = getContext()->getModelImporter();
		//auto guitar_deferred = importer->loadModelFromFile("C:/Users/Stav/Downloads/backpack/backpack.obj", getContext()->getActiveScene().get());
		//auto guitar_forward = importer->loadModelFromFile("C:/Users/Stav/Downloads/backpack/backpack.obj", getContext()->getActiveScene().get());
		//guitar_forward.getComponent<Transformation>().setLocalPosition({ 0,0,3 });
		//auto& renderable = guitar_forward.getComponent<RenderableComponent>();
		//renderable.renderTechnique = RenderableComponent::Forward;

		//auto tHandler = Texture::loadTextureFromFile("C:/Users/Stav/Downloads/gear_store_8k.hdr", Texture::Type::None);
		//Skybox::CreateSkybox(Skybox::TexType::EQUIRECTANGULAR, tHandler);


		//auto importer = getContext()->getModelImporter();

		//auto guitar = importer->loadModelFromFile("C:/Users/Stav/Downloads/backpack/backpack.obj", getContext()->getActiveScene().get());;

		//auto gun = importer->loadModelFromFile("C:/Users/Stav/Downloads/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX", getContext()->getActiveScene().get());

		//auto& trans = gun.getComponent<Transformation>();
		//trans.scale({ .1f, .1f, .1f });
		//trans.rotate({ 1,0,0 }, -90);
		//trans.translate({0, 0,-5});
		//trans.rotate({ 0,1,0 }, 180);
	
		//auto box = ShapeFactory::createSphere(getContext()->getActiveScene().get());
		//auto& mat = gun.addComponent<Material>();
		//auto albedoMap = Engine::get()->getSubSystem<Assets>()->importTexture2D("C:/Users/Stav/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", false);
		//mat.setTexture(Texture::Type::Albedo, Resource<Texture>(albedoMap));
		//auto roughnessMap = Engine::get()->getSubSystem<Assets>()->importTexture2D("C:/Users/Stav/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", false);
		//mat.setTexture(Texture::Type::Roughness, Resource<Texture>(roughnessMap));
		//auto normalMap = Engine::get()->getSubSystem<Assets>()->importTexture2D("C:/Users/Stav/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", false);
		//mat.setTexture(Texture::Type::Normal, Resource<Texture>(normalMap));
		//auto metallicMap = Engine::get()->getSubSystem<Assets>()->importTexture2D("C:/Users/Stav/Downloads/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", false);
		//mat.setTexture(Texture::Type::Metallic, Resource<Texture>(metallicMap));
		//auto aoMap = Engine::get()->getSubSystem<Assets>()->importTexture2D("C:/Users/Stav/Downloads/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga", false);
		//mat.setTexture(Texture::Type::AmbientOcclusion, Resource<Texture>(aoMap));
		//box.getComponent<Transformation>().setLocalPosition({ i * 3,0,j * 3 });

		//auto& transform = guitar->getComponent<Transformation>();
		//transform.translate({ 10, 0,0 });

		//transformations.push_back(&trans1);
		//transformations.push_back(&trans2);

		//const int gridLength = 100;

		//for (int i = 0; i < gridLength; i++)
		//{
		//	for (int j = 0; j < gridLength; j++)
		//	{
		//		Transformation trans({(i - gridLength / 2) * 2, 0, (j - gridLength / 2) * 2});
		//		transformations.push_back(trans);
		//	}
		//}

		//box1->addComponent<InstanceBatch>( transformations );
		//guitar->RemoveComponent<RenderableComponent>( );

		//sphere1 = (Sphere*)ModelBuilder::builder<Sphere>(1, 36, 18)
		//	.build();

		//sphere2 = (Sphere*)ModelBuilder::builder<Sphere>(1, 36, 18)
		//	.build();

		//sphere1->translate(3, 0, 0);
		//sphere2->translate(6, 0, 0);

		//box->addChildren(sphere1);
		//sphere1->addChildren(sphere2);

		//postProcess(PostProcess::grayscale());
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}