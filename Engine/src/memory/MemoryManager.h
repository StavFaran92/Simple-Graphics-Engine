#pragma once

#include "Asset.h"
#include "MemoryPool.h"

class MemoryManager
{
    MemoryPool<Asset>& getMemoryPool() { return m_memoryPool; }

private:
    MemoryPool<Asset> m_memoryPool;
};