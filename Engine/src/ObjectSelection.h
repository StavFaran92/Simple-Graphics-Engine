#pragma once

#include "IProjector.h"

#include <memory>

class FrameBufferObject;
class RenderBufferObject;
class TextureHandler;


class ObjectSelection : public IProjector
{
public:
	struct PixelInfo {
		unsigned int ObjectID = 0;
		unsigned int DrawID = 0;
		unsigned int PrimID = 0;
	};

	ObjectSelection();
	~ObjectSelection();

	bool selectedObject(int x, int y);
	int getSelectedObject() const;

	// Inherited via IProjector
	virtual bool init() override;
	virtual void enableWriting() override;
	virtual void disableWriting() override;

private:
	int m_selectedObject = -1;

	std::shared_ptr<FrameBufferObject> m_frameBuffer = nullptr;
	std::shared_ptr<RenderBufferObject> m_renderBuffer = nullptr;
	std::shared_ptr<TextureHandler> m_textureHandler = nullptr;

};

