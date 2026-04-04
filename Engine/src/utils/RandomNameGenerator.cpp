#include "utils/RandomNameGenerator.h"
#include "memory/UUID.h"

size_t RandomNameGenerator::s_nameLength = 8;

std::string RandomNameGenerator::generateName()
{
    static const char charset[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(s_nameLength);

    for (int i = 0; i < s_nameLength; ++i)
        result += charset[dist(rng)];

    return result;
}