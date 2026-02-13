#pragma once

#include "Asset.h"
#include "MemoryPool.h"

class MemoryManager
{
    MemoryPool<Resource>& getMemoryPool() { return m_memoryPool; }

private:
    MemoryPool<Resource> m_memoryPool;
};