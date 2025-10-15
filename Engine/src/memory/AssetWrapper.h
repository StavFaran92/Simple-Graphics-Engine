#pragma once

#include "memory/ResourceWrapper.h"
#include "core/Core.h"

template<typename T>
class EngineAPI AssetWrapper : public ResourceWrapper<T>
{ };