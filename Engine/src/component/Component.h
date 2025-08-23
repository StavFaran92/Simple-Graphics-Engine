#pragma once

#include <memory>

#include "core/Core.h"

#include "component/ScriptableEntity.h"
#include "core/Configurations.h"
#include "render/RenderView.h"
#include "geometry/Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include "physics/Physics.h"
#include "cereal/types/optional.hpp"
#include "physics/Colliders.h"
#include "serialize/CerealHelpers.h"
#include "geometry/MeshCollection.h"

/**
HOW TO ADD A NEW SERIALIZED COMPONENT GUIDE
	- Add component that inherit from component
	- Add component GUI display
	- Add component GUI add option
	- Add component to Serialized Entity (in Archiver.h)
	- Add component serialize in Archiver::serializeEntity
	- Add component deseralize in Archiver::deserializeEntity
*/

class Scene;
class Mesh;
class Entity;



struct EngineAPI Component
{
public:
	virtual ~Component() = default;

	template <class Archive>
	void serialize(Archive& archive) {
	}

	
};

template<typename T>
std::shared_ptr<Component> getComponentIfExists(const Entity& e)
{
	std::shared_ptr<Component> c;
	if (e.HasComponent<T>())
	{
		c = std::make_shared<T>(e.getComponent<T>());
	}
	return c;
}

template<typename T>
static void attachSimple(std::shared_ptr<Component> c, Entity entityHandler)
{
	if (auto tc = std::dynamic_pointer_cast<T>(c))
	{
		entityHandler.addComponent<T>(*tc);
	}
}

#define REGISTER_COMPONENT(TYPE) \
	CEREAL_REGISTER_TYPE(TYPE); \
	CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, TYPE); \
	inline ComponentSerializeFnRegister<TYPE> TYPE##_serializeRegister(getComponentIfExists<TYPE>); \
	inline ComponentDeserializeFnRegister<TYPE> TYPE##_deserializeRegister(TYPE::attachToEntity);

struct EngineAPI TagComponent : public Component
{
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
        std::string tag;
};




struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(Resource<Texture> skyboxImage);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(originalImage);

	}

	void setSkybox(Resource<Texture> image);

	void build();
	

        Resource<Texture> originalImage;
        Resource<Texture> cubemap;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};


struct EngineAPI RenderableComponent : public Component
{
	RenderableComponent() = default;

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(renderTechnique);
	}

	enum class RenderTechnique : int
	{
		Forward,
		Deferred
	};

    RenderTechnique renderTechnique = RenderTechnique::Deferred;
};


struct EngineAPI NativeScriptComponent : public Component
{
	NativeScriptComponent() = default;
	std::shared_ptr<ScriptableEntity> script = nullptr;
	Entity entity = Entity::EmptyEntity;

        template<typename T, typename... Args>
        T* bind(Args&&... args)
        {
                static_assert(std::is_base_of<ScriptableEntity, T>::value, "T must inherit from ScriptableEntity");

		script = std::make_shared< T>(std::forward<Args>(args)...);
		script->entity = entity;
		//script->onCreate();

		return static_cast<T*>(script.get());
	}

	void unbind()
	{
		if (script)
		{
			//delete script;
			script = nullptr;
		}
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity);
		SERIALIZED_MEMBER(script);
	}
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};


struct EngineAPI ObjectComponent : public Component
{
	ObjectComponent() = default;
	ObjectComponent(Entity e, const std::string& name) : name(name), e(e) {};

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(e);
	}

    std::string name;
    Entity e = Entity::EmptyEntity;
    
};


struct EngineAPI ShaderComponent : public Component
{
	enum ProjectionType : int
	{
		DefaultProjection = 0,
		Texture2D = 1
	};

	ShaderComponent();
	
	ShaderComponent(Shader* vertexShader, Shader* fragmentShader);

	void addTexture(const std::string& name, Resource<Texture> texture)
	{
		customTextures[name] = texture;
	}

	void setProjectionTexture(Resource<Texture> texture);

	void update();

	void parseUniforms(const std::string& sourceCode);

	void setShader(Resource<Shader> shader);

	template <class Archive>
	void serialize(Archive& archive) {
			SERIALIZED_MEMBER(m_customShader); 
			SERIALIZED_MEMBER(shaderOverride); 
			SERIALIZED_MEMBER(customTextures); 
			SERIALIZED_MEMBER(projection); 
			SERIALIZED_MEMBER(projectionTexture);
			SERIALIZED_MEMBER(m_shaderFilePath); 
			SERIALIZED_MEMBER(isValid);
	}

	// This will only be used by forward renderer, ignored by deffered
	Resource<Shader> m_customShader;

	std::map<std::string, Resource<Texture>> customTextures;

	ProjectionType projection = ProjectionType::DefaultProjection;
	Resource<Texture> projectionTexture;
	std::shared_ptr<RenderView> renderViewProjection;

	std::unordered_map<std::string, Value> m_uniformProperties;

	std::string m_shaderFilePath;
	ShaderOverride shaderOverride;

        bool isValid = false;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};


struct EngineAPI InstanceBatch : public Component
{
public:
	InstanceBatch();

	InstanceBatch(const std::vector< std::shared_ptr<Transformation>>& transformations, Resource<Mesh> mesh);
	

	void addTransformation(const std::shared_ptr<Transformation>& transformation);

	const std::vector<glm::mat4> getMatrices() const;
	std::vector<std::shared_ptr<Transformation>>& getTransformations();
	
	size_t getCount() const
	{
		return transformations.size();
	}

	void build();

	std::vector<std::shared_ptr<Transformation>> transformations;
	Resource<Mesh> mesh;
	unsigned int m_id = 0;
	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
private:
	

private:

};


struct EngineAPI ImageComponent : public Component
{
	ImageComponent() = default;

	ImageComponent(Resource<Texture> image) : image(image) {}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(image);
		SERIALIZED_MEMBER(size);
		SERIALIZED_MEMBER(position);
		SERIALIZED_MEMBER(rotate);
	}

	glm::vec2 size;
	glm::vec2 position;
	float rotate = 0;

        Resource<Texture> image;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};


struct CharacterController : public Component
{
	CharacterController() = default;


};

struct TestComp : public Component
{
	TestComp() = default;


	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(test);
		SERIALIZED_MEMBER_OPTIONAL(test2, 0);
	}

	int test = 0;
	int test2 = 0;

	
};

struct VolumeComponent : public Component
{
	VolumeComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
	}

	Resource<Shader> shader;
};

struct EngineAPI PlayerController : public Component
{
	PlayerController() = default;

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
	}

	void move(glm::vec3 disp)
	{
		m_disp = disp;
	}

	void reset()
	{
		m_disp = glm::vec3(0.f);
	}

    glm::vec3 m_disp{};
    int controllerIndex = 0;
    
};

//REGISTER_COMPONENT(TagComponent)
//REGISTER_COMPONENT(SkyboxComponent)
//REGISTER_COMPONENT(RenderableComponent)
//REGISTER_COMPONENT(NativeScriptComponent)
//REGISTER_COMPONENT(PhysicsComponent)
//REGISTER_COMPONENT(CameraComponent)
//REGISTER_COMPONENT(MeshComponent)
//REGISTER_COMPONENT(MaterialComponent)
//REGISTER_COMPONENT(ObjectComponent)
//REGISTER_COMPONENT(ShaderComponent)
//REGISTER_COMPONENT(InstanceBatch)
//REGISTER_COMPONENT(ImageComponent)
//REGISTER_COMPONENT(PlayerController)
//REGISTER_COMPONENT(TestComp)