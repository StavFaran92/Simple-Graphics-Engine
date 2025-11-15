#pragma once

class TextureDataWidget
{
public:
	enum class TextureFilter
	{
		Nearest,
		Linear,
		NearestMipNearest,
		LinearMipNearest,
		NearestMipLinear,
		LinearMipLinear
	};

	enum class TextureWrap
	{
		Repeat,
		ClampToEdge,
		ClampToBorder,
		MirroredRepeat
	};

	TextureDataWidget() = default;
	void draw();

	TextureFilter m_filterMode = TextureFilter::LinearMipLinear;
	TextureWrap   m_wrapMode = TextureWrap::Repeat;
	float         m_anisotropy = 1.0f;
};