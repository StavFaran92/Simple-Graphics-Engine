#include "EntryPoint.h"
#include "sge.h"

class CustomBoxBehaviour : public ScriptableEntity
{
	virtual void onCreate() override
	{
		std::cout << "Box was created modafaka.\n";
	}
};

class Sandbox : public Application
{
public:

	void start() override
	{
		//Skybox::CreateSkybox("C:/Users/Stav/Downloads/black-white-starry-night2.jpg");

		
		{
			auto importer = getContext()->getModelImporter();
			auto rock = importer->loadModelFromFile("C:/Users/Stav/Downloads/rock1-ue/rock2.obj", getContext()->getActiveScene().get());

			auto rockMat = std::make_shared<Material>();

			auto albedoMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/rock1-ue/rock1-albedo.png", false);
			rockMat->setTexture(Texture::Type::Albedo, Resource<Texture>(albedoMap));
			auto roughnessMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/rock1-ue/rock1-roughness.png", false);
			rockMat->setTexture(Texture::Type::Roughness, Resource<Texture>(roughnessMap));
			auto normalMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/rock1-ue/rock1-normal_dx.png", false);
			rockMat->setTexture(Texture::Type::Normal, Resource<Texture>(normalMap));
			auto metallicMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/rock1-ue/rock1-metallic.png", false);
			rockMat->setTexture(Texture::Type::Metallic, Resource<Texture>(metallicMap));
			auto aoMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/rock1-ue/rock1-ao.png", false);
			rockMat->setTexture(Texture::Type::AmbientOcclusion, Resource<Texture>(aoMap));

			rock.getComponent<MaterialComponent>().addMaterial(rockMat);

			std::vector<Transformation> transformations;



			auto random = Engine::get()->getRandomSystem();

			{
				const int rocksCount = 100000;
				for (int i = 0; i < rocksCount; i++)
				{
					float angle = (float)i * 2 * Constants::PI / rocksCount;
					Transformation trans(rock);
					trans.setLocalPosition({ cos(angle) * 400 + random->rand() * 100, random->rand() * 10, sin(angle) * 400 + random->rand() * 100 });
					auto scaleFactor = random->rand() * .005f;
					trans.setLocalScale({ .001f + scaleFactor, .001f + scaleFactor, .001f + scaleFactor });
					trans.setLocalRotation(glm::vec3{ random->rand() * 2 * Constants::PI, random->rand() * 2 * Constants::PI, random->rand() * 2 * Constants::PI });
					transformations.push_back(trans);
				}

				auto& meshComponent = rock.getComponent<MeshComponent>();
				rock.addComponent<InstanceBatch>(transformations, meshComponent.mesh);
			}
		}

		{
			auto sphere = ShapeFactory::createSphere(&getContext()->getActiveScene()->getRegistry());
			auto& trans = sphere.getComponent<Transformation>();
			trans.setLocalScale({ 100, 100, 100 });

			auto sphereMat = std::make_shared<Material>();

			auto albedoMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/Ground048_2K-JPG/Ground048_2K-JPG_Color.jpg", false);
			sphereMat->setTexture(Texture::Type::Albedo, Resource<Texture>(albedoMap));
			auto roughnessMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/Ground048_2K-JPG/Ground048_2K-JPG_Roughness.jpg", false);
			sphereMat->setTexture(Texture::Type::Roughness, Resource<Texture>(roughnessMap));
			auto normalMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/Ground048_2K-JPG/Ground048_2K-JPG_NormalDX.jpg", false);
			sphereMat->setTexture(Texture::Type::Normal, Resource<Texture>(normalMap));
			auto aoMap = Texture::create2DTextureFromFile("C:/Users/Stav/Downloads/Ground048_2K-JPG/Ground048_2K-JPG_AmbientOcclusion.jpg", false);
			sphereMat->setTexture(Texture::Type::AmbientOcclusion, Resource<Texture>(aoMap));

			sphere.getComponent<MaterialComponent>().addMaterial(sphereMat);
		}
	}

};

Application* CreateApplication()
{
	return new Sandbox();
}