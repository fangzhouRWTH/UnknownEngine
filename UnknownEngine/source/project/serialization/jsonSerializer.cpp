#include "jsonSerializer.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace unknown
{
    using json = nlohmann::json;
    using jiter = json::iterator;

        struct Serializer::Field::Impl
    {
        jiter iter;
        bool empty = true;
    };

    struct Serializer::Impl
    {
        json data;
    };

    Serializer::Serializer() : impl(std::make_unique<Impl>())
    {

    }

    Serializer::~Serializer()
    {
    }

    bool Serializer::Load(const std::string & filePath)
    {
        auto index = filePath.rfind(".");
        auto postfix = filePath.substr(index+1,filePath.size());
        if(postfix != "json")
            return false;

        std::ifstream f(filePath);
        
        json data = json::parse(f);
        mbIsEmpty = data.empty();

        if(!mbIsEmpty)
        {
            impl->data = data;
            return true;
        }

        return false;
    }

    bool Serializer::Contains(const std::string &field)
    {
        return impl->data.contains(field);
    }

    Serializer::Field Serializer::Access(const std::string &field)
    {
        Field f;
        
        f.impl->iter = impl->data.find(field);

        if(f.impl->iter!=impl->data.end())
            f.impl->empty = false;
        return f;
    }

    Serializer::Field Serializer::Access(const u64 &idx)
    {
        Field f;
        if(impl->data.empty() || Size()<=idx)
            return f;

        f.impl->iter = impl->data.begin()+idx;
        f.impl->empty = false;

        return f;
    }

    u64 Serializer::Size()
    {
        u64 s = 0;
        if(!impl->data.empty())
            s = !impl->data.size();
        
        return s;
    }

    Serializer::Field::Field():impl(std::make_unique<Impl>())
    {
    }

    Serializer::Field::Field(const Field &other):impl(std::make_unique<Impl>())
    {
        impl->iter = other.impl->iter;
        impl->empty = other.impl->empty;
    }

    Serializer::Field::Field(const Field &&other):impl(std::make_unique<Impl>())
    {
        impl->iter = other.impl->iter;
        impl->empty = other.impl->empty;
    }

    Serializer::Field &Serializer::Field::operator=(const Field &other)
    {
        impl->iter = other.impl->iter;
        impl->empty = other.impl->empty;

        return *this;
    }

    Serializer::Field &Serializer::Field::operator=(const Field &&other)
    {
        impl->iter = other.impl->iter;
        impl->empty = other.impl->empty;

        return *this;
    }

    bool Serializer::Field::Contains(const std::string & field)
    {
        return impl->iter->contains(field);
    }

    Serializer::Field Serializer::Field::Access(const std::string & field)
    {
        Field f;

        if(impl->empty)
            return f;

        f.impl->iter = impl->iter->find(field);
        if(f.impl->iter!=impl->iter->end())
            f.impl->empty = false;
        
        return f;
    }

    Serializer::Field Serializer::Field::Access(const u64 &idx)
    {
        Field f;

        if(impl->empty || Size() <= idx)
            return f;

        f.impl->iter = impl->iter->begin()+idx;
        f.impl->empty = false;

        return f;
    }

    u64 Serializer::Field::Size()
    {
        u64 s = 0;
        if(!impl->empty)
            s = impl->iter->size();
        
        return s;
    }

    Serializer::Field::~Field()
    {
    }

    bool Serializer::Field::isEmpty()
    {
        return impl->empty;
    }

    template <typename T>
    std::optional<T> Serializer::Field::Value()
    {
        std::optional<T> res;
        if(impl->empty)
            return res;

        res = impl->iter->get<T>();
        return res;
    }

    #define DECL_VALUE_GETTER(TYPE) \
    template std::optional<TYPE> Serializer::Field::Value<TYPE>();

    DECL_VALUE_GETTER(std::string)
    DECL_VALUE_GETTER(bool)
}