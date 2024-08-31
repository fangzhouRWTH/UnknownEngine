#include "log.hpp"

namespace unknown::debug
{
    void Logger::MatrixInfo(Mat4f mat)
    {
        spdlog::info("Matrix 4x4 ======\n  [{}],[{}],[{}],[{}]\n  [{}],[{}],[{}],[{}]\n  [{}],[{}],[{}],[{}]\n  [{}],[{}],[{}],[{}]\n \n",
                mat.col(0)(0),mat.col(1)(0),mat.col(2)(0),mat.col(3)(0),
                mat.col(0)(1),mat.col(1)(1),mat.col(2)(1),mat.col(3)(1),
                mat.col(0)(2),mat.col(1)(2),mat.col(2)(2),mat.col(3)(2),
                mat.col(0)(3),mat.col(1)(3),mat.col(2)(3),mat.col(3)(3));
    }
    void Logger::VectorInfo(Vec3f vec)
    {
        spdlog::info("Vector 3 ==> X: [{}], Y: [{}], Z: [{}] ====", vec.x(), vec.y(), vec.z());
    }
}