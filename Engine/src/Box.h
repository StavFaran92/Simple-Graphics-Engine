#pragma once

#include "Model.h"

class Box : public Model
{
public:
	static Box* generateBox();
private:
	static std::shared_ptr<Mesh> generateMesh();
};

