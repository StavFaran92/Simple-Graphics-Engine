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
