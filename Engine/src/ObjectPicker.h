#pragma once
#include <memory>

#include "Core.h"

#include "Resource.h"
#include "FrameBufferObject.h"
#include "RenderBufferObject.h"

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

	int pickObject(int x, int y);

	int getSelectedObject() const;

private:
	FrameBufferObject m_frameBuffer;
	Resource<Texture> m_targetTexture;
	Resource<Shader> m_pickingShader;

	int m_selectedObject = -1;
};

