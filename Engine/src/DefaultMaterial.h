#pragma once

#include <memory>
#include "Core.h"
#include "Material.h"

class EngineAPI DefaultMaterial : public Material
{
public:
	DefaultMaterial();
	DefaultMaterial(float shine);

	void UseMaterial(Shader& shader) override;

	~DefaultMaterial();


};