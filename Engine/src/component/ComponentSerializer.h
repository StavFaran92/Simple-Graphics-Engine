#pragma once

#include "runtime/Entity.h"
#include "component/Component.h"

class EngineAPI ComponentSerializer
{
public:
	using SerializeFn = std::function<std::shared_ptr<Component>(const Entity& e)>;
	using DeserializeFn = std::function<void(std::shared_ptr<Component>, Entity, ResourceWrapper<Scene>&)>;

	static void registerSerializeFunc(const SerializeFn& fn);
	static void serializeComponents(const Entity& e, std::vector<std::shared_ptr<Component>>& components);

	static void registerDeserializeFunc(const DeserializeFn& fn);
	static void deserializeComponents(const std::vector<std::shared_ptr<Component>>& components, Entity& e, ResourceWrapper<Scene>& scene);

private:

	// Magic static
	static std::vector<SerializeFn>& getSerializeFunctionRegistry() {
		static std::vector<SerializeFn> serializeFunctionRegistry;
		return serializeFunctionRegistry;
	}

	static std::vector<DeserializeFn>& getDeserializeFunctionRegistry() {
		static std::vector<DeserializeFn> deserializeFunctionRegistry;
		return deserializeFunctionRegistry;
	}

private:
	
};


// The above 2 can be merged into one 
// (Even a more abstract tool could be made if combined with asset registerer [maybe too abstract to be useful])

// Serialize
template<typename T>
class ComponentSerializeFnRegister
{
public:
	static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

	ComponentSerializeFnRegister(const ComponentSerializer::SerializeFn& fn)
	{
		ComponentSerializer::registerSerializeFunc(fn);
	}

	static ComponentSerializeFnRegister<T> staticRegister;
};

template<typename T>
ComponentSerializeFnRegister<T> ComponentSerializeFnRegister<T>::staticRegister;

// Deserealize
template<typename T>
class ComponentDeserializeFnRegister
{
public:
	static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

	ComponentDeserializeFnRegister(const ComponentSerializer::DeserializeFn& fn)
	{
		ComponentSerializer::registerDeserializeFunc(fn);
	}

	static ComponentDeserializeFnRegister<T> staticRegister;
};

template<typename T>
ComponentDeserializeFnRegister<T> ComponentDeserializeFnRegister<T>::staticRegister;
