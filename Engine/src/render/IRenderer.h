#pragma once

class Mesh;
class Entity;

class IRenderer {
public:
	IRenderer() = default;
	virtual ~IRenderer() = default;

	bool prepareMeshForRender(Mesh* mesh, const Entity& entityHandler);
	bool prepareEntityForRender(const Entity& entityHandler);
};