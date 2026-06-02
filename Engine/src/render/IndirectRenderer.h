#pragma once

class Scene;

class IndirectRenderer
{
public:
	void init();

	void renderScene(Scene*);
};