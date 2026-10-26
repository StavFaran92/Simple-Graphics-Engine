#pragma once

#include <string>

#include "core/Core.h"

// Needs a current GL context for init() and update()
class EngineAPI GpuInfo
{
public:
	void init();

	void update();

	const std::string& getVendor() const { return m_vendor; }
	const std::string& getRenderer() const { return m_renderer; }
	const std::string& getGLVersion() const { return m_glVersion; }
	const std::string& getGLSLVersion() const { return m_glslVersion; }
	int getMaxTextureSize() const { return m_maxTextureSize; }
	int getMaxTextureUnits() const { return m_maxTextureUnits; }

	// NVIDIA only (GL_NVX_gpu_memory_info); the memory getters are 0 when this is false
	bool hasMemoryInfo() const { return m_hasMemoryInfo; }
	int getTotalMemoryMB() const { return m_totalMemoryKB / 1024; }
	int getAvailableMemoryMB() const { return m_availableMemoryKB / 1024; }
	int getUsedMemoryMB() const { return (m_totalMemoryKB - m_availableMemoryKB) / 1024; }
	int getEvictionCount() const { return m_evictionCount; }
	int getEvictedMemoryMB() const { return m_evictedMemoryKB / 1024; }

private:
	std::string m_vendor;
	std::string m_renderer;
	std::string m_glVersion;
	std::string m_glslVersion;
	int m_maxTextureSize = 0;
	int m_maxTextureUnits = 0;

	bool m_hasMemoryInfo = false;
	int m_totalMemoryKB = 0;
	int m_availableMemoryKB = 0;
	int m_evictionCount = 0;
	int m_evictedMemoryKB = 0;
};
