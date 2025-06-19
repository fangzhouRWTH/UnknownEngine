#pragma once

#include <random>
//#include <mutex>
//#include <thread>
//#include <chrono>

#include "core/math.hpp"

namespace unknown {
class BaseRandomGenerator {
public:
  BaseRandomGenerator() : rng(std::random_device{}()) {}

  int getInt(int min, int max) {
    // std::lock_guard<std::mutex> lock(mutex);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
  }

  float getFloat(float min, float max) {
    // std::lock_guard<std::mutex> lock(mutex);
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
  }

  double getDouble(double min, double max) {
    // std::lock_guard<std::mutex> lock(mutex);
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
  }

private:
  std::mt19937 rng;
  // std::mutex mutex;
};

class ColorGenerator {
public:
  Vec3f Generate() {
    float x = mRng.getFloat(0.0, 1.0);
    float y = mRng.getFloat(0.0, 1.0);
    float z = mRng.getFloat(0.0, 1.0);
    return {x, y, z};
  }

private:
  BaseRandomGenerator mRng;
};

class SimpleBallPositionGenerator {
public:
  SimpleBallPositionGenerator(float radius) : mRadius(radius) {}
  Vec3f Generate() { return generate() * mRadius; }

private:
  Vec3f generate() {
    float alpha = mRng.getFloat(0.0, 1.0) * 3.14159 * 2.0;
    float theta = mRng.getFloat(-0.5, 0.5) * 3.14159;
    float radius = mRng.getFloat(0.0, 1.0);

    float z = sin(theta) * radius;
    float xy = cos(theta) * radius;
    float x = xy * cos(alpha);
    float y = xy * sin(alpha);

    return {x, y, z};
  }
  float mRadius;
  BaseRandomGenerator mRng;
};
} // namespace unknown