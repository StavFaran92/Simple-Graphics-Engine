#pragma once
#include <memory>

#include "core/Core.h"

#include "memory/ResourceWrapper.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"

class FrameBufferObject;
class RenderBufferObject;
class Texture;
class Context;
class Shader;

class EngineAPI ObjectPicker : public SubSystem
{
public:
	struct PixelInfo {
		unsigned int ObjectID = 0;
		unsigned int DrawID = 0;
		unsigned int PrimID = 0;
	};

	ObjectPicker();

	bool init();

	int pickObject(int x, int y, Entity camera);

	int getSelectedObject() const;

	void setSelectedObject(int selectedID);

private:
	std::shared_ptr<FrameBufferObject> m_frameBuffer;
	ResourceWrapper<Texture> m_targetTexture;
	ResourceWrapper<Shader> m_pickingShader;

	int m_selectedObject = -1;
};

