#include "EntryPoint.h"
#include "sge.h"

void createGuitar()
{
	//auto importer = Engine::get()->getContext()->getModelImporter();
	//auto guitar = importer->loadModelFromFile("C:/Users/Stav/Downloads/backpack/backpack.obj", Engine::get()->getContext()->getActiveScene().get());

	//auto random = Engine::get()->getRandomSystem();
	//auto x = random->rand() * 10 - 5;
	//auto z = random->rand() * 10 - 5;

	//auto& guitarTransform = guitar.getComponent<Transformation>();
	//guitarTransform.setLocalPosition({ x, 10, z });
	//guitarTransform.rotate({ 0, 1, 0 }, 45);

	//auto& rb = guitar.addComponent<RigidBodyComponent>(RigidbodyType::Dynamic, 1.f);

	//guitar.addComponent<CollisionMeshComponent>();

	//	for (auto& child : guitar.getChildren())
	//		{
	//			child.second.addComponent<CollisionMeshComponent>();
	//			//auto& rb = child.second.addComponent<RigidBodyComponent>();
	//			//rb.mass = 1;
	//			//rb.type = RigidbodyType::Kinematic;

	//			//auto& rb = child.second.addComponent<RigidBodyComponent>();
	//			//rb.mass = 1;
	//			//rb.type = RigidbodyType::Dynamic;
	//		}
	//	

}

void createSphere()
{
	auto sphere = ShapeFactory::createSphere(&Engine::get()->getContext()->getActiveScene()->getRegistry());
	{
		auto random = Engine::get()->getRandomSystem();
		auto x = random->rand() * 10 - 5;
		auto z = random->rand() * 10 - 5;

		auto& sphereTransform = sphere.getComponent<Transformation>();
		sphereTransform.setLocalPosition({ x, 10, z });


		//auto& mat = sphere.addComponent<Material>();
		//auto tex = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/checkers.jpg");
		//mat.setTexture(Texture::Type::Diffuse, Resource<Texture>(tex));

		sphere.addComponent<RigidBodyComponent>(RigidbodyType::Dynamic, 1.f);
		sphere.addComponent<CollisionSphereComponent>(1.f);
	}
}

void createBox()
{
	auto box = ShapeFactory::createBox(&Engine::get()->getContext()->getActiveScene()->getRegistry());
	{
		auto random = Engine::get()->getRandomSystem();
		auto x = random->rand() * 10 - 5;
		auto z = random->rand() * 10 - 5;

		auto& boxTransform = box.getComponent<Transformation>();
		boxTransform.setLocalPosition({ x, 10, z });
		boxTransform.rotate({ 0, 1, 0 }, 45);

		//auto& mat = box.addComponent<Material>();
		//auto tex = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/checkers.jpg");
		//mat.setTexture(Texture::Type::Diffuse, Resource<Texture>(tex));

		box.addComponent<RigidBodyComponent>(RigidbodyType::Dynamic, 1.f);
		box.addComponent<CollisionBoxComponent>(.5f);
	}
}

class GUI_Helper : public GuiMenu
{
	// Inherited via GuiMenu
	virtual void display() override
	{
		ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 300, 250 }, ImGuiCond_Once);
		ImGui::Begin("Physx sample", 0, ImGuiWindowFlags_NoResize);

		ImGui::PushItemWidth(100);

		if (ImGui::Button("Create Box"))
		{
			createBox();
		}

		if (ImGui::Button("Create Sphere"))
		{
			createSphere();
		}

		if (ImGui::Button("Create Guitar"))
		{
			createGuitar();
		}

		if (ImGui::Button("Start Sim"))
		{
			Engine::get()->getContext()->getActiveScene()->startSimulation();
		}

		if (ImGui::Button("Stop Sim"))
		{
			Engine::get()->getContext()->getActiveScene()->stopSimulation();
		}

		

		ImGui::PopItemWidth();


		ImGui::End();
	}
};

class Sandbox : public Application
{
public:

	void start() override
	{
		ImGui::SetCurrentContext((ImGuiContext*)Engine::get()->getImguiHandler()->getCurrentContext());


		// set Editor camera as active camera
		auto editorCamera = Engine::get()->getContext()->getActiveScene()->createEntity("Editor Camera");
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<EditorCamera>();
		Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(editorCamera);

		Skybox::CreateSkyboxFromCubemap({ SGE_ROOT_DIR "Resources/Engine/Textures/Skybox/right.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/left.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/top.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/bottom.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/front.jpg",
		SGE_ROOT_DIR"Resources/Engine/Textures/Skybox/back.jpg" }, Engine::get()->getContext()->getActiveScene().get());

		//auto camera = Engine::get()->getContext()->getActiveScene()->getActiveCamera();
		//camera->lookAt(0, 5, 0);
		//camera->setPosition(25, 225, 35);

		auto gui = new GUI_Helper();
		Engine::get()->getImguiHandler()->addGUI(gui);

		//{
		//	auto importer = getContext()->getModelImporter();
		//	auto guitar = importer->loadModelFromFile("C:/Users/Stav/Downloads/backpack/backpack.obj", getContext()->getActiveScene().get());

		//	auto& boxTransform = guitar.getComponent<Transformation>();
		//	boxTransform.setLocalPosition({ 0, 10, .5f });
		//	boxTransform.rotate({ 0, 1, 0 }, 45);
		//	//boxTransform.rotate({ 0, 0, 1 }, 180);

		//	auto& rb = guitar.addComponent<RigidBodyComponent>();
		//	rb.mass = 1;
		//	rb.type = RigidbodyType::Dynamic;

		//	guitar.addComponent<CollisionMeshComponent>();

		//	for (auto& child : guitar.getChildren())
		//	{
		//		child.second.addComponent<CollisionMeshComponent>();
		//		//auto& rb = child.second.addComponent<RigidBodyComponent>();
		//		//rb.mass = 1;
		//		//rb.type = RigidbodyType::Kinematic;

		//		//auto& rb = child.second.addComponent<RigidBodyComponent>();
		//		//rb.mass = 1;
		//		//rb.type = RigidbodyType::Dynamic;
		//	}
		//}

		


		auto box = ShapeFactory::createBox(&Engine::get()->getContext()->getActiveScene()->getRegistry());
		{

			auto& boxTransform = box.getComponent<Transformation>();
			boxTransform.setLocalPosition({ 0, 14, .5f });
			boxTransform.rotate({ 0, 1, 0 }, 45);
			//boxTransform.rotate({ 0, 0, 1 }, 180);

			auto& rb = box.addComponent<RigidBodyComponent>(RigidbodyType::Dynamic, 1.f);
			auto& collisionBox = box.addComponent<CollisionBoxComponent>(.5f);
		}

		//for(int i=0; i<100; i++)
		//	for (int j = 0; j < 100; j++)
		//		{
		//			auto box = ShapeFactory::createBox(Engine::get()->getContext()->getActiveScene().get());

		//			auto& boxTransform = box.getComponent<Transformation>();
		//			boxTransform.setPosition({ i, 12, j });
		//			//boxTransform.rotate({ 0, 1, 0 }, 90);
		//			boxTransform.rotate({ 0, 0, 1 }, 50);

		//			//auto& rb = box.addComponent<RigidBodyComponent>();
		//			//rb.mass = 1;
		//			//rb.type = RigidbodyType::Dynamic;

		//			//auto& collisionBox = box.addComponent<CollisionBoxComponent>();
		//			//collisionBox.halfExtent = .5f;
		//		}

		auto sphere = ShapeFactory::createSphere(&Engine::get()->getContext()->getActiveScene()->getRegistry());
		{
			
			auto& sphereTransform = sphere.getComponent<Transformation>();
			sphereTransform.setLocalPosition({ 0, 10, 2 });
			//boxTransform.rotate({ 0, 1, 0 }, 90);
			//boxTransform.rotate({ 0, 0, 1 }, 180);

			//auto& rb = sphere.addComponent<RigidBodyComponent>();
			//rb.mass = 1;
			//rb.type = RigidbodyType::Dynamic;


			auto& collisionBox = sphere.addComponent<CollisionSphereComponent>(1.f);
		}

		sphere.setParent(box);


		{
			auto ground = ShapeFactory::createBox(&Engine::get()->getContext()->getActiveScene()->getRegistry());
			auto& groundTransfrom = ground.getComponent<Transformation>();
			groundTransfrom.setLocalScale({ 50, .5f, 50 });
			//groundTransfrom.rotate({ 0, 0, 1 }, 20);
			//groundTransfrom.rotate({ 0, 0, 1 }, 90);

			//auto& mat = ground.addComponent<Material>();
			//auto tex = Engine::get()->getSubSystem<Assets>()->importTexture2D("Resources/Content/Textures/floor.jpg");
			//mat.setTexture(Texture::Type::Diffuse, Resource<Texture>(tex));
			auto& rb = ground.addComponent<RigidBodyComponent>(RigidbodyType::Static, 1.f);


			auto& collisionBox = ground.addComponent<CollisionBoxComponent>(.5f);
		}
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}