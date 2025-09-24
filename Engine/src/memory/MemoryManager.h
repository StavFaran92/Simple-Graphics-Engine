#pragma once

#include "Asset.h"
#include "MemoryPool.h"

class MemoryManager
{
    MemoryPool<ResourceBase>& getMemoryPool() { return m_memoryPool; }

private:
    MemoryPool<ResourceBase> m_memoryPool;
};