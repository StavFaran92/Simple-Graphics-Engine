#include "memory/UUID.h"

UUID UUID::generate_uuid_v4() 
{
#ifdef UUID_OPTION_A
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";  // UUID version 4
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);  // variant bits
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);

    return UUID(ss.str());
#else
    return UUID(std::to_string(UUID::getAndIncrement()));
#endif
}

// Constructor from std::string
UUID::UUID(const std::string& str) : m_value(str) {}

UUID::UUID(const char* str) : m_value(str) {}

void UUID::setCounter(size_t count) 
{
    counter = count;
}

size_t UUID::getCounter()
{
    return counter;
}

size_t UUID::getAndIncrement()
{
    return counter++;
}

    

// Implicit conversion to std::string
UUID::operator std::string() const {
    return m_value;
}

const std::string& UUID::str() const {
    return m_value;
}

// Comparison operators
bool UUID::operator==(const UUID& other) const {
    return m_value == other.m_value;
}

bool UUID::operator!=(const UUID& other) const {
    return !(*this == other);
}

bool UUID::operator<(const UUID& other) const {
    return m_value < other.m_value;
}

bool UUID::empty() const
{
    return m_value.empty();
}