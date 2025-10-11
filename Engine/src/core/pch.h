#pragma once

// pch.h
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <cstdint>
#include <atomic>
#include <filesystem>
#include <functional>
#include <mutex>
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include "cereal/cereal.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>