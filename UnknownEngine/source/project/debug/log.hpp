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
    #define INFO_MAT4(mat4) INFO_LOG("Mat4:\n  {},{},{},{} \n  {},{},{},{} \n  {},{},{},{} \n  {},{},{},{} \n",\
    mat4.col(0).x(),mat4.col(1).x(),mat4.col(2).x(),mat4.col(3).x(),\
    mat4.col(0).y(),mat4.col(1).y(),mat4.col(2).y(),mat4.col(3).y(),\
    mat4.col(0).z(),mat4.col(1).z(),mat4.col(2).z(),mat4.col(3).z(),\
    mat4.col(0).w(),mat4.col(1).w(),mat4.col(2).w(),mat4.col(3).w())
}