#pragma once

#include "sge.h"

class TextureDataWidget
{
public:
		TextureDataWidget() = default;
	void draw();

	TextureFilter m_filterMode = TextureFilter::Linear;
	TextureWrap   m_wrapMode = TextureWrap::Clamp;
	float         m_anisotropy = 1.0f;
	TextureSemantic m_semantic = TextureSemantic::Color;
};