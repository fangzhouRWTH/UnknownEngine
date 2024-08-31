#pragma once
#include <memory>

namespace unknown
{
    template <typename Impl>
    class Singleton
    {
    public:
        static void Initialize()
        {
            if (sInstance == nullptr)
            {
                sInstance = new Impl();
                sInstance->initialize();
            }
        }
        static Impl* Get() { return sInstance; }
        virtual void initialize() = 0;

    protected:
        bool sInitialized = false;
        static Impl* sInstance;
    };

    template <typename Impl>
    Impl* Singleton<Impl>::sInstance = nullptr;
}