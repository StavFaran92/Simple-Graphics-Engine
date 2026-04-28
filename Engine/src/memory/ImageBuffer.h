#pragma once

#include "core/Core.h"

#include <vector>
#include <cstdint>

class EngineAPI ImageBuffer
{
public:
    enum class DataType
    {
        UINT8,
        FLOAT32
    };

public:
    ImageBuffer();
    ImageBuffer(const std::vector<uint8_t>& data);
    ImageBuffer(const std::vector<float>& data);
    ImageBuffer(const unsigned char* data, size_t size);

    ImageBuffer& operator=(const std::vector<uint8_t>& data);
    ImageBuffer& operator=(const std::vector<float>& data);
    ImageBuffer& operator=(std::vector<uint8_t>&& data);
    ImageBuffer& operator=(std::vector<float>&& data);

    void set(const std::vector<uint8_t>& data);
    void set(const std::vector<float>& data);
    void set(std::vector<uint8_t>&& data);
    void set(std::vector<float>&& data);

    void clear();

    void resize(const size_t newSize);
    const uint8_t* data() const;
    uint8_t* data();
    const std::vector<uint8_t>& getBytes() const;
    std::vector<uint8_t>& getBytes();

    const float* getFloatData() const;
    float* getFloatData();

    size_t size() const;
    size_t count() const;
    bool empty() const;

    DataType getType() const;
    bool isFloat() const;

private:
    std::vector<uint8_t> m_data;
    DataType m_type = DataType::UINT8;
};