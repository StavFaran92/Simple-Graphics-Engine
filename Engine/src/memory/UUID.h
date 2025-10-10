#pragma once

#include <random>
#include <sstream>
#include <spdlog/fmt/fmt.h>
#include "serialize/CerealHelpers.h"
#include <nlohmann/json.hpp>
#include "core/Core.h"


class EngineAPI UUID {
    std::string m_value;

public:
    UUID() = default;

    // Constructor from std::string
    UUID(const std::string& str);

    UUID(const char* str);

    inline static size_t counter = 0;

    static void setCounter(size_t count);

    static size_t getCounter();

    static size_t getAndIncrement();

    static UUID generate_uuid_v4();

    // Implicit conversion to std::string
    operator std::string() const;

    const std::string& str() const;

    // Comparison operators
    bool operator==(const UUID& other) const;

    bool operator!=(const UUID& other) const;

    bool operator<(const UUID& other) const;

    bool empty() const;

    template <class Archive>
    void serialize(Archive& archive) {
        SERIALIZED_MEMBER(m_value);
    }

    
};

// Static empty UUID instance
inline const UUID EMPTY_UUID("");

namespace std {
    template <>
    struct hash<UUID> {
        std::size_t operator()(const UUID& uuid) const noexcept {
            return std::hash<std::string>{}(uuid.str());
        }
    };
}


template <>
struct fmt::formatter<UUID> {
    // Parses format specifications (we don't use any)
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    // Formats the UUID by converting it to string
    template <typename FormatContext>
    auto format(const UUID& uuid, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", uuid.str());
    }
};

// JSON serialization
inline void to_json(nlohmann::json& j, const UUID& uuid) {
    j = static_cast<std::string>(uuid);  // or: j = uuid.str();
}

// JSON deserialization
inline void from_json(const nlohmann::json& j, UUID& uuid) {
    uuid = UUID(j.get<std::string>());
}