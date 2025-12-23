#pragma once

#include "sge.h"

class TextureDataWidget
{
public:
		TextureDataWidget() = default;
	void draw();

	Texture::TextureFilter m_filterMode = Texture::TextureFilter::LinearMipLinear;
	Texture::TextureWrap   m_wrapMode = Texture::TextureWrap::Repeat;
	float         m_anisotropy = 1.0f;
	Texture::TextureSemantic m_semantic = Texture::TextureSemantic::Color;
};