#pragma once
#include "ecs/entity.hpp"
#include "platform/type.hpp"
#include <string>

namespace unknown::ecs
{
    class BaseActor
    {
    public:
        std::string name;
        u32 id;
    protected:
        BaseActor(const std::string &_name) : name(_name) {}
    };

    class EPlayer final : public Entity<EPlayer>, public BaseActor
    {
    public:
        EPlayer(const std::string &_name) : BaseActor(_name) {}
    };

    class EOpponent final : public Entity<EOpponent>, public BaseActor
    {
    public:
        EOpponent(const std::string &_name) : BaseActor(_name) {}
    };
}