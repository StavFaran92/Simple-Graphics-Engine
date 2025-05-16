#pragma once

#include <memory>

#include "Core.h"

#include "ScriptableEntity.h"
#include "Configurations.h"
#include "RenderView.h"
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Physics.h"
#include "cereal/types/optional.hpp"

#define SERIALIZED_MEMBER(name, member)	archive(cereal::make_nvp(name, member));

#define SERIALIZED_MEMBER_OPTIONAL(name, member, value)	\
	try {												\
		archive(cereal::make_nvp(name, member));		\
	}													\
	catch (const cereal::Exception&) {					\
		member = value;									\
	}

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


struct EngineAPI Component
{

};

struct EngineAPI TagComponent : public Component
{
	std::string tag;
};

struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(Resource<Texture> skyboxImage) : skyboxImage(skyboxImage) {}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("originalImage", originalImage);

	}

	Resource<Texture> skyboxImage;
	Resource<Texture> originalImage;
};

struct EngineAPI RenderableComponent : public Component
{
	RenderableComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("renderTechnique", renderTechnique);
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
		SERIALIZED_MEMBER("entity", entity);
		SERIALIZED_MEMBER("script", script);
	}
};

struct EngineAPI RigidBodyComponent : public Component
{

	RigidBodyComponent() = default;
	RigidBodyComponent(RigidbodyType type, float mass) : type(type), mass(mass) {};

	void addForce(glm::vec3 force);
	void setForce(glm::vec3 force);

	void move(glm::vec3 position);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("type", type);
		SERIALIZED_MEMBER("mass", mass);
	}

	bool isLockedLinearX = false;
	bool isLockedLinearY = false;
	bool isLockedLinearZ = false;
	bool isLockedAngularX = false;
	bool isLockedAngularY = false;
	bool isLockedAngularZ = false;
	RigidbodyType type = RigidbodyType::Static;
	float mass = 0;
	bool isChanged = false;
	glm::vec3 m_targetPisition{0};
	glm::vec3 m_force{ 0 };
	void* simulatedBody = nullptr;
};

struct EngineAPI CollisionBoxComponent : public Component
{
	CollisionBoxComponent() = default;
	CollisionBoxComponent(float halfExtent) : halfExtent(halfExtent) {};

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("halfExtent", halfExtent);
		SERIALIZED_MEMBER("layerMask", layerMask);
	}

	float halfExtent = 0;
	Physics::LayerMask layerMask = Physics::LayerMask::LAYER_0;
};

struct EngineAPI CollisionSphereComponent : public Component
{
	CollisionSphereComponent() = default;
	CollisionSphereComponent(float radius) : radius(radius) {};

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("radius", radius);
		SERIALIZED_MEMBER("layerMask", layerMask);
	}

	float radius = 0;
	Physics::LayerMask layerMask = Physics::LayerMask::LAYER_0;
};

struct EngineAPI CollisionMeshComponent : public Component
{
	CollisionMeshComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("isConvex", isConvex);
		SERIALIZED_MEMBER("layerMask", layerMask);
	}

	bool isConvex = false;
	Resource<Mesh> mesh = Resource<Mesh>::empty;
	Physics::LayerMask layerMask = Physics::LayerMask::LAYER_0;
};

struct EngineAPI CollisionTerrainComponent : Component
{
	CollisionTerrainComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("layerMask", layerMask);
	}

	Physics::LayerMask layerMask = Physics::LayerMask::LAYER_0;
};

struct EngineAPI CameraComponent : public Component
{
	CameraComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("center", center);
		SERIALIZED_MEMBER("up", up);
	}

	static CameraComponent createPerspectiveCamera(float fovy, float aspect, float znear, float zfar)
	{
		CameraComponent cam;
		cam.fovy = fovy;
		cam.aspect = aspect;
		cam.znear = znear;
		cam.zfar = zfar;
		cam.type = CamType::PERSPECTIVE;
		return cam;
	}

	glm::mat4 getProjection() const;

	float fovy = 0;
	float aspect = 0;
	float znear = 0;
	float zfar = 0;

	enum CamType
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	CamType type;

	glm::vec3 front{0,0,-1};
	glm::vec3 right;
	glm::vec3 center{ 0,0,0 };
	glm::vec3 up{ 0,1,0 };
};

struct EngineAPI MeshComponent : public Component
{
	MeshComponent(const Resource<MeshCollection>& mesh) : mesh(mesh) {};
	MeshComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("mesh", mesh);
		SERIALIZED_MEMBER("materialSlot", materialSlot);

	}

	float materialSlot = 0; // todo this will be used (probably as a list) to support multi material models
	Resource<MeshCollection> mesh = Resource<MeshCollection>::empty;
};

//struct EngineAPI MeshArrayRendererComponent : public Component
//{
//	MeshArrayRendererComponent() = default;
//	
//	void addMesh(const MeshRenderer& mesh)
//	{
//		meshes.push_back(mesh);
//	}
//
//	auto begin() { return meshes.begin(); }
//	auto end() { return meshes.end(); }
//	auto begin() const { return meshes.begin(); }
//	auto end() const { return meshes.end(); }
//
//	template <class Archive>
//	void serialize(Archive& archive) {
//		archive(meshes);
//	}
//
//	std::vector<MeshRenderer> meshes;
//};

struct EngineAPI MaterialComponent : public Component
{
	MaterialComponent();
	

	auto begin() { return materials.begin(); }
	auto end() { return materials.end(); }
	auto begin() const { return materials.begin(); }
	auto end() const { return materials.end(); }

	void addMaterial(const std::shared_ptr<Material>& mat)
	{
		materials[count++] = mat;
	}

	void setMaterial(int index, const std::shared_ptr<Material>& mat)
	{
		materials[index] = mat;
	}

	std::shared_ptr<Material> at(int index)
	{
		return materials.at(index);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("materials", materials);
	}

	std::map<int, std::shared_ptr<Material>> materials;
	int count = 0;
};

struct EngineAPI ObjectComponent : public Component
{
	ObjectComponent() = default;
	ObjectComponent(Entity e, const std::string& name) : name(name), e(e) {};

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("name", name);
		SERIALIZED_MEMBER("e", e);
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
		archive(m_customShader, shaderOverride, customTextures, projection, projectionTexture/*, renderViewProjection*/,
			/*m_uniformProperties,*/ m_shaderFilePath, isValid);
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
private:
	

private:
	std::vector<std::shared_ptr<Transformation>> transformations;
	Resource<Mesh> mesh;
	unsigned int m_id = 0;
};

struct EngineAPI ImageComponent : public Component
{
	ImageComponent() = default;

	ImageComponent(Resource<Texture> image) : image(image) {}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("image", image);
		SERIALIZED_MEMBER("size", size);
		SERIALIZED_MEMBER("position", position);
		SERIALIZED_MEMBER("rotate", rotate);
	}

	glm::vec2 size;
	glm::vec2 position;
	float rotate = 0;

	Resource<Texture> image;
};

struct CharacterController : public Component
{
	CharacterController() = default;


};

struct TestComp : public Component
{
	TestComp() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("test", test);
		SERIALIZED_MEMBER_OPTIONAL("test2", test2, 0);
	}

	int test = 0;
	int test2 = 0;
};

struct PostProcessComponent : public Component
{
	PostProcessComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
	}

	Resource<Shader> shader;
};