#pragma once

#include <memory>
#include <string>

#include "core/Core.h"

class Entity;
class SGE_Regsitry;
class Model;
template<typename>class ResourceWrapper;

class EngineAPI ShapeFactory
{
public:
	static Entity createBoxEntity(SGE_Regsitry* registry);
	static Entity createQuad(SGE_Regsitry* registry);
	static Entity createSphere(SGE_Regsitry* registry);

private:
	static Entity createEntity(SGE_Regsitry* registry, const std::string& name);
};