#pragma once

#include "core/math.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <fmt/core.h>

#include <string>
// #define LOG_TEST_1 spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR,SPDLOG_VER_PATCH)
// #define LOG()

namespace unknown::debug
{
    class Logger
    {
    public:
        static void MatrixInfo(Mat4f mat);
        static void VectorInfo(Vec3f vec);
    };

    #define INFO_LOG(fmt, ...) spdlog::info(fmt, __VA_ARGS__);
    #define INFO_PRINT(fmt) spdlog::info(fmt);
}