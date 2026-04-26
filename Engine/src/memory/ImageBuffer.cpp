#include "ImageBuffer.h"
#include <cstring>

ImageBuffer::ImageBuffer() = default;

ImageBuffer::ImageBuffer(const std::vector<uint8_t>& data)
{
    set(data);
}

ImageBuffer::ImageBuffer(const std::vector<float>& data)
{
    set(data);
}

ImageBuffer::ImageBuffer(const unsigned char* data, size_t size)
{
    m_data.resize(size);
    std::memcpy(m_data.data(), data, size);
    m_type = DataType::UINT8;
}

ImageBuffer& ImageBuffer::operator=(const std::vector<uint8_t>& data)
{
    set(data);
    return *this;
}

ImageBuffer& ImageBuffer::operator=(const std::vector<float>& data)
{
    set(data);
    return *this;
}

ImageBuffer& ImageBuffer::operator=(std::vector<uint8_t>&& data)
{
    set(std::move(data));
    return *this;
}

ImageBuffer& ImageBuffer::operator=(std::vector<float>&& data)
{
    set(std::move(data));
    return *this;
}

void ImageBuffer::set(const std::vector<uint8_t>& data)
{
    m_data = data;
    m_type = DataType::UINT8;
}

void ImageBuffer::set(const std::vector<float>& data)
{
    m_type = DataType::FLOAT32;
    m_data.resize(data.size() * sizeof(float));
    std::memcpy(m_data.data(), data.data(), m_data.size());
}

void ImageBuffer::set(std::vector<uint8_t>&& data)
{
    m_data = std::move(data);
    m_type = DataType::UINT8;
}

void ImageBuffer::set(std::vector<float>&& data)
{
    m_type = DataType::FLOAT32;
    m_data.resize(data.size() * sizeof(float));
    std::memcpy(m_data.data(), data.data(), m_data.size());
}

void ImageBuffer::clear()
{
    m_data.clear();
}

void ImageBuffer::resize(const size_t newSize)
{
    getBytes().resize(newSize);
}

const uint8_t* ImageBuffer::data() const
{
    return getBytes().data();
}

uint8_t* ImageBuffer::data()
{
    return getBytes().data();
}

const std::vector<uint8_t>& ImageBuffer::getBytes() const
{
    return m_data;
}

std::vector<uint8_t>& ImageBuffer::getBytes()
{
    return m_data;
}

const float* ImageBuffer::getFloatData() const
{
    return reinterpret_cast<const float*>(m_data.data());
}

float* ImageBuffer::getFloatData()
{
    return reinterpret_cast<float*>(m_data.data());
}

size_t ImageBuffer::size() const
{
    if (m_type == DataType::FLOAT32)
        return m_data.size() / sizeof(float);
    return m_data.size();
}

bool ImageBuffer::empty() const
{
    return size() == 0;
}

ImageBuffer::DataType ImageBuffer::getType() const
{
    return m_type;
}

bool ImageBuffer::isFloat() const
{
    return m_type == DataType::FLOAT32;
}