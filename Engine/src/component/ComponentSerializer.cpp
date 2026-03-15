#include "ComponentSerializer.h"

void ComponentSerializer::registerSerializeFunc(const SerializeFn& fn)
{
	getSerializeFunctionRegistry().push_back(fn);
}

void ComponentSerializer::serializeComponents(const Entity& e, std::vector<std::shared_ptr<Component>>& components)
{
	auto& funcs = getSerializeFunctionRegistry();
	for (const auto& f : funcs)
	{
		if (auto c = f(e)) 
		{
			components.push_back(c);
		}
	}
}

void ComponentSerializer::registerDeserializeFunc(const DeserializeFn& fn)
{
	getDeserializeFunctionRegistry().push_back(fn);
}

void ComponentSerializer::deserializeComponents(const std::vector<std::shared_ptr<Component>>& components, Entity& e, ResourceWrapper<Scene>& scene)
{
	auto& funcs = getDeserializeFunctionRegistry();
	for (const auto& f : funcs)
	{
		for (auto& c : components)
		{
			f(c, e, scene);
		}
	}
}
