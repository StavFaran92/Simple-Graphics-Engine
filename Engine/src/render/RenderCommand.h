#pragma once

#include <memory>

#include "core/Core.h"

#include "gl/glew.h"

class VertexArrayObject;

class EngineAPI RenderCommand
{
public:
	enum BufferBit : int
	{
		DEPTH_BUFFER_BIT = 0x00000100,
		COLOR_BUFFER_BIT = 0x00004000
	};

	static void clear();

	static void draw(const VertexArrayObject& vao);

	static void drawPatches(const VertexArrayObject& vao);

	static void drawInstanced(const VertexArrayObject& vao, int count);

	static void setViewport(int x, int y, int w, int h);

	static void copyFrameBufferData(unsigned int src, unsigned int dst, int bufferBit);
};