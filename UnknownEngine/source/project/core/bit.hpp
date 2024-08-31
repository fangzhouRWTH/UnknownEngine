#pragma once
#include "platform/type.hpp"
#include <cassert>

namespace unknown
{
    template <typename T>
    class BitMask
    {
        static_assert(u32(T::ENUM_MAX) <= 32u);

    public:
        bool IsEmpty() const { return mask == 0u; }
        bool IsSet(T e) const
        {
            assert(e != T::ENUM_MAX);
            return (mask & (1 << u32(e))) != 0u;
        }
        void Set(T e)
        {
            assert(e != T::ENUM_MAX);
            mask |= (1 << u32(e));
        }
        void Set(BitMask<T> m)
        {
            mask |= m.mask;
        }
        void Set(u32 n)
        {
            mask |= n;
        }
        void Copy(BitMask<T> m)
        {
            mask = m.mask;
        }
        void Copy(u32 n)
        {
            mask = n;
        }
        void Clear(T e)
        {
            assert(e != T::ENUM_MAX);
            mask &= ~(1 << u32(e));
        }
        void Reset()
        {
            mask = 0u;
        }
        bool IsSet(BitMask<T> m) const
        {
            return mask == m.mask;
        }

        u32 Get() const { return mask; }

    private:
        u32 mask = 0u;
    };
}