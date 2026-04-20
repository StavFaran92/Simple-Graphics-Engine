#pragma once

#include <memory>

#include "core/Core.h"
#include "physics/Colliders.h"
#include "serialize/CerealHelpers.h"
#include "memory/AssetAliases.h"

/**
HOW TO ADD A NEW SERIALIZED COMPONENT GUIDE
	- Add component that inherit from component
	- Add component GUI display
	- Add component GUI add option
	- Add component to Serialized Entity (in Archiver.h)
	- Add component serialize in Archiver::serializeEntity
	- Add component deseralize in Archiver::deserializeEntity
*/

class Mesh;
class Entity;
class Transformation;
class Asset;
class UUID;



struct EngineAPI Component
{
public:
	virtual ~Component() = default;


	template <class Archive>
	void serialize(Archive& archive) {
	}

	void registerSceneDependency(SceneResourceRef& scene) const;
	void removeSceneDependency(SceneResourceRef& scene) const;

	std::vector<AssetRef<Asset>> gatherDependencies() const;


	virtual std::string getName() = 0;
	virtual void resolve(SceneResourceRef& scene) {};
	virtual void postLoad(SceneResourceRef& scene) {};

protected:
	virtual std::vector<AssetRef<Asset>*> gatherDependenciesInternal() const;
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

#ifndef COMPONENT_SERDES
	#ifdef ENGINE_BUILD_DLL
		#define COMPONENT_SERDES(TYPE) \
			inline ComponentSerializeFnRegister<TYPE> TYPE##_serializeRegister;
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
	std::string getName() override { return "TagComponent"; }
	std::string tag;
};

struct EngineAPI InstanceBatch : public Component
{
public:
	InstanceBatch();

	InstanceBatch(const std::vector< std::shared_ptr<Transformation>>& transformations, ResourceRef<Mesh> mesh);
	

	void addTransformation(const std::shared_ptr<Transformation>& transformation);

	const std::vector<glm::mat4> getMatrices() const;
	std::vector<std::shared_ptr<Transformation>>& getTransformations();
	
	size_t getCount() const
	{
		return transformations.size();
	}

	void build();

	std::string getName() override;

	std::vector<std::shared_ptr<Transformation>> transformations;
	ResourceRef<Mesh> mesh;
	unsigned int m_id = 0;
private:
	

private:

};

struct TestComp : public Component
{
	TestComp() = default;

	std::string getName() override { return "TestComp"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(test);
		SERIALIZED_MEMBER_OPTIONAL(test2, 0);
	}

	int test = 0;
	int test2 = 0;

	
};