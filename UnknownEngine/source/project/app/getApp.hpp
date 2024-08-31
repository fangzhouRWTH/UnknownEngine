#pragma once
#include <memory>

namespace unknown
{
    class IApplication;

    class Application
    {
        public:
            static std::shared_ptr<IApplication> GetApplication();
    };
}