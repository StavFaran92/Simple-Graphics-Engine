#include "TextureThumbnailGenerator.h"

#include "geometry/Model.h"
#include "geometry/Mesh.h"
#include "render/RenderCommand.h"
#include "memory/BuiltInAssets.h"
#include "core/Logger.h"

#include <GL/glew.h>
#include <algorithm>
#include <cmath>

namespace {

constexpr int kMaxTaps = 16;

} // namespace

TextureThumbnailGenerator::TextureThumbnailGenerator()
{
	m_shader = Shader::load(SGE_ROOT_DIR "../EditorApp/Resources/Content/Shaders/ThumbnailDownsampleShader.glsl");
}

bool TextureThumbnailGenerator::generate(const AssetRecord& aInfo, const TextureResourceRef& target)
{
	if (!m_shader) return false;

	TextureResourceRef source = TextureAssetRef(aInfo.uuid).resource();
	if (source.isEmpty()) return false;

	// Cubemaps and 3D textures fall back to the type icon
	const TextureData& srcData = source->getData();
	if (srcData.target != TextureTarget::TEXTURE_2D) return false;

	const int srcWidth  = source->getWidth();
	const int srcHeight = source->getHeight();
	const int dstWidth  = target->getWidth();
	const int dstHeight = target->getHeight();
	if (srcWidth <= 0 || srcHeight <= 0 || dstWidth <= 0 || dstHeight <= 0) return false;

	// Fit inside the target keeping the aspect ratio
	const float scale = std::min((float)dstWidth / srcWidth, (float)dstHeight / srcHeight);
	const int fitWidth  = std::max(1, (int)(srcWidth * scale));
	const int fitHeight = std::max(1, (int)(srcHeight * scale));

	// Enough samples per output pixel to cover the source texels it spans
	const float ratio = std::max((float)srcWidth / fitWidth, (float)srcHeight / fitHeight);
	const int taps = std::clamp((int)std::ceil(ratio), 1, kMaxTaps);

	m_fbo.bind();
	m_fbo.attachTexture(target->getID());

	if (!m_fbo.isComplete())
	{
		logError("TextureThumbnailGenerator: FBO is not complete");
		m_fbo.unbind();
		return false;
	}

	std::string captionGPU = "Texture Thumbnail";
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Letterbox background
	glViewport(0, 0, dstWidth, dstHeight);
	glClearColor(g_thumbnailBackground.r, g_thumbnailBackground.g, g_thumbnailBackground.b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport((dstWidth - fitWidth) / 2, (dstHeight - fitHeight) / 2, fitWidth, fitHeight);

	m_shader->use();
	m_shader->setUniformValue("source", 0);
	m_shader->setUniformValue("footprint", glm::vec2(1.f / fitWidth, 1.f / fitHeight));
	m_shader->setUniformValue("taps", taps);
	m_shader->setUniformValue("flipY", srcData.flip);
	m_shader->setUniformValue("background", g_thumbnailBackground);

	source->setSlot(0);
	source->bind();

	auto quad = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD);
	RenderCommand::draw(quad.resource()->getPrimaryMesh()->getVAO());

	source->unbind();

	glPopDebugGroup();

	m_fbo.unbind();
	return true;
}
