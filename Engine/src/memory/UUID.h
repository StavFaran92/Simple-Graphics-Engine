#pragma once

#include <random>
#include <sstream>
#include <spdlog/fmt/fmt.h>
#include "serialize/CerealHelpers.h"
#include <nlohmann/json.hpp>
#include "core/Core.h"

typedef uint64_t uuid_type;

class EngineAPI UUID {
    uuid_type m_value;

public:
    UUID() = default;

    UUID(uuid_type uuid);

    static uuid_type counter;

    static void setCounter(uuid_type count);

    static uuid_type getCounter();

    static uuid_type getAndIncrement();

    static UUID generate_uuid_v4();

    // Implicit conversion to std::string
    operator std::string() const;

    operator uuid_type() const;

    std::string str() const;

    uuid_type value() const;

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
inline const UUID EMPTY_UUID(0);

namespace std {
    template <>
    struct hash<UUID> {
        std::size_t operator()(const UUID& uuid) const noexcept {
            return std::hash<uuid_type>{}(uuid.value());
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
    j = uuid.value();
}

// JSON deserialization
inline void from_json(const nlohmann::json& j, UUID& uuid) {
    uuid_type count = j.get<uuid_type>();
    uuid = UUID(count);
}