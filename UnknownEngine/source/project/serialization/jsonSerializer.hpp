#pragma once
#include <string>
#include <memory>
#include <optional>

#include "platform/type.hpp"

namespace unknown
{
    class Serializer
    {
        public:
            struct Field
            {
                friend class Serializer;
                public:
                ~Field();
                Field(const Field & other);
                Field(const Field && other);
                Field& operator=(const Field & other);
                Field& operator=(const Field && other);

                bool Contains(const std::string & field);
                Field Access(const std::string & field);
                Field Access(const u64 & idx);
                u64 Size();
                template <typename T>
                std::optional<T> Value();

                bool isEmpty();

                private:
                Field();
                struct Impl;
                std::unique_ptr<Impl> impl;
            };

            Serializer();
            ~Serializer();
            Serializer(const Serializer & other) = delete;
            Serializer operator=(const Serializer& other)= delete;

            bool Load(const std::string & filePath);
            bool Contains(const std::string & field);
            Field Access(const std::string & field);
            Field Access(const u64 & idx);
            u64 Size();

        private:
            struct Impl;
            std::unique_ptr<Impl> impl;
            bool mbIsEmpty;
    };
}