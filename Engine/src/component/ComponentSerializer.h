#pragma once

#include "runtime/Entity.h"
#include "component/Component.h"

class ComponentSerializer
{
public:
	using SerializeFn = std::function<std::shared_ptr<Component>(const Entity& e)>;
	using DeserializeFn = std::function<Entity(std::shared_ptr<const Component>)>;

	static void registerSerializeFunc(const SerializeFn& fn);
	static void serializeComponents(const Entity& e, std::vector<std::shared_ptr<Component>>& components);

	static void registerDeserializeFunc(const DeserializeFn& fn);
	static void deserializeComponents(const std::vector<std::shared_ptr<Component>>& components, Entity& e);

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

template<typename T>
class ComponentFnRegister
{
public:
	static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

	ComponentFnRegister(const ComponentSerializer::SerializeFn& fn)
	{
		ComponentSerializer::registerSerializeFunc(fn);
	}

	static ComponentFnRegister<T> staticRegister;
};

template<typename T>
ComponentFnRegister<T> ComponentFnRegister<T>::staticRegister;