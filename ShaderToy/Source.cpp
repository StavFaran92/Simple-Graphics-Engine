#include "EntryPoint.h"
#include "sge.h"

static Color getPixel(int i, int j)
{
	int width = Engine::get()->getWindow()->getWidth();
	int height = Engine::get()->getWindow()->getHeight();

	unsigned char r = static_cast<unsigned char>(((float)j / width) * 255);
	unsigned char g = static_cast<unsigned char>(((float)i / height) * 255);

	return { r, g, 0 };
}

//class ShaderToyScript : public ScriptableEntity
//{
//	virtual void onCreate() override
//	{
//		int width = Engine::get()->getWindow()->getWidth();
//		int height = Engine::get()->getWindow()->getHeight();
//
//		auto& image = entity.getComponent<ImageComponent>();
//		image.image = Texture::createTexture(width, height);
//		image.size = glm::vec2(width, height);
//		image.position = glm::vec2(0, 0);
//
//	}
//
//	virtual void onUpdate(float deltaTime) {
//	};
//
//};



class ShaderToy : public Application
{
public:

	void start() override
	{
		Resource<Texture> densityTexture = Texture::importTexture3D("C:/Users/Stav/Downloads/VolumeCloud.png");

		//if (settings.saveOnDisk)
		//{
		//	AssetInfo aInfo;
		//	aInfo.origFilePath = "C:/Users/Stav/Downloads/VolumeCloud.png";
		//	aInfo.uuid = texture.getUID();
		//	aInfo.aType = AssetType::TEXTURE;
		//	aInfo.name = settings.name;
		//	aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		//	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);
		//}

		auto projectionEnt = Engine::get()->getContext()->getActiveScene()->createEntity();
		auto displayEnt = Engine::get()->getContext()->getActiveScene()->createEntity();

		int width = Engine::get()->getWindow()->getWidth();
		int height = Engine::get()->getWindow()->getHeight();
		auto projectionTexture = Texture::createTexture(width, height);

		auto& shader = Shader::createOverrideShader("RayMarchSample", "../../ShaderToy/Resources/Content/Shaders/VolumetricClouds_v2.glsl", ShaderOverride::Pixel);
		auto& shaderComponent = projectionEnt.addComponent<ShaderComponent>();
		shaderComponent.setProjectionTexture(projectionTexture);
		shaderComponent.projection = ShaderComponent::ProjectionType::Texture2D;
		shaderComponent.setShader(shader);

		shaderComponent.addTexture("test", densityTexture);

		//Texture::TextureImportSettings settings;
		//settings.genMipMap = true;
		//auto& noiseTexture = Texture::importTexture2D("C:/Users/Stav/Downloads/noise2.png", settings);
		//shaderComponent.addTexture("uNoise", noiseTexture);
		projectionEnt.addComponent<RenderableComponent>(); // todo reevaluate

		auto& image = displayEnt.addComponent<ImageComponent>();
		image.image = projectionTexture;
		image.size = { width, height };


		// I need the shader to write into the texture I created,
		// si I need to have a FBO / RenderView, 
		// set it up
		// bind it 
		// render using the specified shader (maybe override the image pixel shader?)
		// 
		// basically what I want is to be able to draw onto a texture
		// and then use that texture to render 
		// what i can do is use 2 objects
		// one render into a texture
		// the other is using that texture
		// the second I have implemented
		// also I need types of shaders overrides, i dont need the base shader here
		// shader needs 2 more things: 
		// override type and projection target
		// think about override type more.. need to see current functionality.
		//shader component will have projection and allow to project to texture
		

		//int* pixels = new int[width * height];
		//for (int i = 0; i < height; i++)
		//{
		//	for (int j = 0; j < width; j++)
		//	{
		//		auto pixel = getPixel(i, j);
		//		pixels[i * width + j] = 0xFF << 24 | (pixel.b << 16) | (pixel.g << 8) | pixel.r;
		//	}
		//}

		//tex.get()->setData(0, 0, width, height, pixels);
	}

};

Application* CreateApplication()
{
	return new ShaderToy();
}