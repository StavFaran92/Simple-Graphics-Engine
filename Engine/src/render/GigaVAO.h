#pragma once

#include <vector>
#include "core/Core.h"
#include "render/VertexLayout.h"

class EngineAPI GigaVAO
{
public:
    static constexpr size_t kInitialVertexBytes = 5  * 1024 * 1024;   //   5 MB
    static constexpr size_t kInitialIndexBytes  = 1  * 1024 * 1024;   //   1 MB  (~262K indices)
    static constexpr size_t kMaxVertexBytes     = 512 * 1024 * 1024;  // 512 MB
    static constexpr size_t kMaxIndexBytes      = 128 * 1024 * 1024;  // 128 MB  (~32M indices)

    GigaVAO() = default;
    GigaVAO(const VertexLayout& layout);
    ~GigaVAO();

    // Appends mesh data into the buffer. Grows up to the hard limit.
    // Returns false only when the hard limit has been reached.
    // outVertexOffset: base vertex index for this mesh (used in indirect draw baseVertex)
    // outIndexOffset:  first index element for this mesh  (used in indirect draw firstIndex)
    bool push(const void* vertexData, size_t vertexCount,
              const std::vector<unsigned int>& indices,
              unsigned int& outVertexOffset, unsigned int& outIndexOffset);

    void bind()   const;
    void unbind() const;

    size_t getVertexCount() const;
    size_t getIndexCount()  const;

private:
    bool growVertexBuffer();
    bool growIndexBuffer();
    void setupVertexAttributes();

private:
    unsigned int m_vaoID = 0;
    unsigned int m_vboID = 0;
    unsigned int m_eboID = 0;

    size_t m_vertexCapacity = 0;  // bytes
    size_t m_vertexUsed     = 0;  // bytes
    size_t m_indexCapacity  = 0;  // bytes
    size_t m_indexUsed      = 0;  // element count

    VertexLayout m_layout;        // stride must be valid before first push
};
