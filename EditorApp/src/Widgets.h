#pragma once

#include "sge.h"

#include "imgui.h"

#include <functional>

void addTextureEditWidget(int textureID, ImVec2 size, std::function<void(UUID uuid)> callback);

void addTextureEditWidget(AssetWrapper<Texture> texture, ImVec2 size, std::function<void(UUID uuid)> callback);

void addTextureEditWidget(AssetWrapper<Material> mat, const std::string& name, Texture::TextureType ttype);

void addSamplerEditWidget(AssetWrapper<Material> mat, ImVec2 size, const std::string& name, Texture::TextureType ttype);

void displayChannelSelectWidget(int*& currentChannel);

void displayTextureWidget();

void displayColoredLabelWidget(const char* label);

void addAssetSelectWidget(const std::string& name, AssetType aType, const std::function<void(UUID)>& uuid);

class UniqueNameWidget
{
public:
	UniqueNameWidget(const std::string& label);
	bool isValid() const;
	void draw();
	void clear();

	std::string m_label;
	std::string name;
};

class FilepathWidget
{
public:
	FilepathWidget(const std::string& label, char const* const* formats, size_t formatCount);
	bool isValid() const;
	void draw();
	void clear();
	bool accept();

	std::string m_filepath;
	bool m_isPressed = false;
	std::string m_label;
	size_t m_formatCount = 0;
	const char* const *m_formats = nullptr;
};

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