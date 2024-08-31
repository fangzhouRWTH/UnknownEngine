#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace unknown
{
    using namespace nlohmann;
    using JsonS = nlohmann::json;
    // class Serializer
    // {
    //     public:
    //         Serializer(std::string filePath);
    //         ~Serializer();
    //         Serializer(const Serializer & other) = delete;
    //         Serializer operator=(const Serializer& other)= delete;
    //     private:
    //         struct Impl;
    //         std::unique_ptr<Impl> mPtr;
    //         bool mbIsEmpty;
    // };
}