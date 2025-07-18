#pragma once
#include <memory>

#include "core/Core.h"

#include "memory/Resource.h"
#include "runtime/Entity.h"

class FrameBufferObject;
class RenderBufferObject;
class TextureHandler;
class Context;
class Shader;

class EngineAPI ObjectPicker
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
	Resource<Texture> m_targetTexture;
	Resource<Shader> m_pickingShader;

	int m_selectedObject = -1;
};

