#pragma once

#include <memory>

#include "core/Core.h"
#include "physics/Colliders.h"
#include "serialize/CerealHelpers.h"

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
class Transformation;
class Asset;
template<typename T> class AssetHandle;



struct EngineAPI Component
{
public:
	virtual ~Component() = default;

	virtual std::vector<AssetHandle<Asset>*> gatherDependencies() const;

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

#ifndef COMPONENT_SERDES
	#ifdef ENGINE_BUILD_DLL
		#define COMPONENT_SERDES(TYPE) \
			inline ComponentSerializeFnRegister<TYPE> TYPE##_serializeRegister(getComponentIfExists<TYPE>); \
			inline ComponentDeserializeFnRegister<TYPE> TYPE##_deserializeRegister(TYPE::attachToEntity);
	#else
		#define COMPONENT_SERDES(TYPE)
	#endif
#endif

#define REGISTER_COMPONENT(TYPE) \
	CEREAL_REGISTER_TYPE(TYPE); \
	CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, TYPE); \
	COMPONENT_SERDES(TYPE)

struct EngineAPI TagComponent : public Component
{
        static void attachToEntity(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&);
        std::string tag;
};

struct EngineAPI InstanceBatch : public Component
{
public:
	InstanceBatch();

	InstanceBatch(const std::vector< std::shared_ptr<Transformation>>& transformations, ResourceWrapper<Mesh> mesh);
	

	void addTransformation(const std::shared_ptr<Transformation>& transformation);

	const std::vector<glm::mat4> getMatrices() const;
	std::vector<std::shared_ptr<Transformation>>& getTransformations();
	
	size_t getCount() const
	{
		return transformations.size();
	}

	void build();

	std::vector<std::shared_ptr<Transformation>> transformations;
	ResourceWrapper<Mesh> mesh;
	unsigned int m_id = 0;
	static void attachToEntity(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&);
private:
	

private:

};

struct TestComp : public Component
{
	TestComp() = default;


	static void attachToEntity(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(test);
		SERIALIZED_MEMBER_OPTIONAL(test2, 0);
	}

	int test = 0;
	int test2 = 0;

	
};