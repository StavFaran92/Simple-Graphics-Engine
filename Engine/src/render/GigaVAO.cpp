#include "render/GigaVAO.h"

#include <GL/glew.h>
#include "core/Logger.h"
#include "core/Configurations.h"

GigaVAO::GigaVAO(const VertexLayout& layout)
    : m_layout(layout)
{

    glGenVertexArrays(1, &m_vaoID);
    glGenBuffers(1, &m_vboID);
    glGenBuffers(1, &m_eboID);

    glBindVertexArray(m_vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferData(GL_ARRAY_BUFFER, kInitialVertexBytes, nullptr, GL_DYNAMIC_DRAW);
    m_vertexCapacity = kInitialVertexBytes;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, kInitialIndexBytes, nullptr, GL_DYNAMIC_DRAW);
    m_indexCapacity = kInitialIndexBytes;

    setupVertexAttributes();

    glBindVertexArray(0);
}

GigaVAO::~GigaVAO()
{
    glDeleteBuffers(1, &m_vboID);
    glDeleteBuffers(1, &m_eboID);
    glDeleteVertexArrays(1, &m_vaoID);
}

bool GigaVAO::push(const void* vertexData, size_t vertexCount,
                   const std::vector<unsigned int>& indices,
                   unsigned int& outVertexOffset, unsigned int& outIndexOffset)
{
    const size_t vertexBytes = vertexCount * m_layout.getStride();
    const size_t indexBytes  = indices.size() * sizeof(unsigned int);

    // Grow vertex buffer if needed
    while (m_vertexUsed + vertexBytes > m_vertexCapacity)
    {
        if (!growVertexBuffer())
        {
            logError("GigaVAO vertex buffer hard limit reached, cannot push mesh.");
            return false;
        }
    }

    // Grow index buffer if needed
    while (m_indexUsed * sizeof(unsigned int) + indexBytes > m_indexCapacity)
    {
        if (!growIndexBuffer())
        {
            logError("GigaVAO index buffer hard limit reached, cannot push mesh.");
            return false;
        }
    }

    outVertexOffset = static_cast<unsigned int>(m_vertexUsed / m_layout.getStride());
    outIndexOffset  = static_cast<unsigned int>(m_indexUsed);

    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferSubData(GL_ARRAY_BUFFER, m_vertexUsed, vertexBytes, vertexData);
    m_vertexUsed += vertexBytes;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_indexUsed * sizeof(unsigned int), indexBytes, indices.data());
    m_indexUsed += indices.size();

    return true;
}

void GigaVAO::bind() const
{
    glBindVertexArray(m_vaoID);
}

void GigaVAO::unbind() const
{
    glBindVertexArray(0);
}

size_t GigaVAO::getVertexCount() const
{
    size_t stride = m_layout.getStride();
    return stride > 0 ? m_vertexUsed / stride : 0;
}

size_t GigaVAO::getIndexCount() const
{
    return m_indexUsed;
}

bool GigaVAO::growVertexBuffer()
{
    const size_t newCapacity = std::min(m_vertexCapacity * 2, kMaxVertexBytes);
    if (newCapacity <= m_vertexCapacity)
        return false;

    unsigned int newVBO = 0;
    glGenBuffers(1, &newVBO);
    glBindBuffer(GL_ARRAY_BUFFER, newVBO);
    glBufferData(GL_ARRAY_BUFFER, newCapacity, nullptr, GL_DYNAMIC_DRAW);

    // GPU-side copy — no CPU round-trip
    glBindBuffer(GL_COPY_READ_BUFFER, m_vboID);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, m_vertexUsed);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);

    glDeleteBuffers(1, &m_vboID);
    m_vboID = newVBO;
    m_vertexCapacity = newCapacity;

    // Re-point VAO attribute pointers at the new VBO
    glBindVertexArray(m_vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    setupVertexAttributes();
    glBindVertexArray(0);

    logDebug("GigaVAO vertex buffer grew to " + std::to_string(newCapacity / (1024 * 1024)) + " MB");
    return true;
}

bool GigaVAO::growIndexBuffer()
{
    const size_t newCapacity = std::min(m_indexCapacity * 2, kMaxIndexBytes);
    if (newCapacity <= m_indexCapacity)
        return false;

    unsigned int newEBO = 0;
    glGenBuffers(1, &newEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, newCapacity, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_COPY_READ_BUFFER, m_eboID);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0, m_indexUsed * sizeof(unsigned int));
    glBindBuffer(GL_COPY_READ_BUFFER, 0);

    glDeleteBuffers(1, &m_eboID);

    // Re-bind the new EBO into the VAO
    glBindVertexArray(m_vaoID);
    m_eboID = newEBO;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboID);
    glBindVertexArray(0);

    m_indexCapacity = newCapacity;

    logDebug("GigaVAO index buffer grew to " + std::to_string(newCapacity / (1024 * 1024)) + " MB");
    return true;
}

void GigaVAO::setupVertexAttributes()
{
    size_t offset = 0;
    for (const auto& entry : m_layout.attribs)
    {
        const AttributeData& attrib = getAttributeData(entry);
        glEnableVertexAttribArray(attrib.location);
        if (attrib.typeName == typeid(int).name())
        {
            glVertexAttribIPointer(attrib.location, (GLint)attrib.length, GL_INT,
                                   (GLsizei)m_layout.getStride(), (const void*)offset);
        }
        else
        {
            glVertexAttribPointer(attrib.location, (GLint)attrib.length, GL_FLOAT, GL_FALSE,
                                  (GLsizei)m_layout.getStride(), (const void*)offset);
        }
        offset += attrib.length * attrib.size;
    }
}
