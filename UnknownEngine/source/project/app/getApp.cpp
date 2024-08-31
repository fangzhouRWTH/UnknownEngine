#include "getApp.hpp"
#include "application.hpp"

namespace unknown
{
    std::shared_ptr<IApplication> Application::GetApplication()
    {
        return std::make_shared<Application_Default>();
    }
}