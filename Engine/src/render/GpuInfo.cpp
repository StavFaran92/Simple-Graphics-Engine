#include "render/GpuInfo.h"

namespace
{
	std::string getGLString(GLenum name)
	{
		const GLubyte* value = glGetString(name);
		return value ? reinterpret_cast<const char*>(value) : "";
	}
}

void GpuInfo::init()
{
	m_vendor = getGLString(GL_VENDOR);
	m_renderer = getGLString(GL_RENDERER);
	m_glVersion = getGLString(GL_VERSION);
	m_glslVersion = getGLString(GL_SHADING_LANGUAGE_VERSION);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits);

	m_hasMemoryInfo = m_vendor.find("NVIDIA") != std::string::npos && GLEW_NVX_gpu_memory_info;

	update();
}

void GpuInfo::update()
{
	if (!m_hasMemoryInfo)
		return;

	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &m_totalMemoryKB);
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &m_availableMemoryKB);
	glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &m_evictionCount);
	glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &m_evictedMemoryKB);
}
