#pragma once
#include <memory>

#include "IProjector.h"

class FrameBufferObject;
class RenderBufferObject;
class TextureHandler;

class ObjectPicker : public IProjector
{
public:
	struct PixelInfo {
		unsigned int ObjectID = 0;
		unsigned int DrawID = 0;
		unsigned int PrimID = 0;
	};

	int pickObject(int x, int y);

	// Inherited via IProjector
	virtual bool init(int windowWidth, int windowHeight) override;
	virtual void enableWriting() override;
	virtual void disableWriting() override;

private:
	std::shared_ptr<FrameBufferObject> m_frameBuffer = nullptr;
	std::shared_ptr<RenderBufferObject> m_renderBuffer = nullptr;
	std::shared_ptr<TextureHandler> m_textureHandler = nullptr;

	int m_width = 0;
	int m_height = 0;
};

