#pragma once
#include "platform/type.hpp"
#include <cassert>

namespace unknown
{
    // Defined Enum Must ends with ENUM_MAX
    template <typename T>
    class BitMask
    {
        static_assert(u32(T::ENUM_MAX) <= 32u);

    public:
        BitMask<T>(){}
        BitMask<T>(u32 m):mask(m){}
        BitMask<T>(const BitMask<T> & other):mask(other.mask){}
        void operator = (const BitMask<T> & other){mask = other.mask;}

        //Default set to 0, as empty
        bool IsEmpty() const { return mask == 0u; }

        //Return true if bit mask contains the value
        bool IsSet(T e) const
        {
            assert(e != T::ENUM_MAX);
            return (mask & (1 << u32(e))) != 0u;
        }

        //Set the bit at the input position to 1
        BitMask<T> & Set(T e)
        {
            assert(e != T::ENUM_MAX);
            mask |= (1 << u32(e));
            return *this;
        }

        //Set the bits of input to 1
        BitMask<T> & Set(BitMask<T> m)
        {
            mask |= m.mask;
            return *this;
        }

        //Use integer to set bits directly
        BitMask<T> & Set(u32 n)
        {
            mask |= n;
            return *this;
        }

        
        void Copy(BitMask<T> m)
        {
            mask = m.mask;
        }

        void Copy(u32 n)
        {
            mask = n;
        }

        BitMask<T> operator|(const BitMask<T> & other)
        {
            return BitMask<T>(mask|other.mask);
        }

        BitMask<T> operator&(const BitMask<T> & other)
        {
            return BitMask<T>(mask&other.mask);
        }

        BitMask<T> operator~()
        {
            return BitMask<T>(~mask);
        }

        BitMask<T> operator^(const BitMask<T> & other)
        {
            return mask^other.mask;
        }

        //Set bit to 0
        void Clear(T e)
        {
            assert(e != T::ENUM_MAX);
            mask &= ~(1 << u32(e));
        }

        //Set bits to 0
        void Clear(BitMask<T> m)
        {
            mask &= ~m.mask;
        }

        
        void Reset()
        {
            mask = 0u;
        }

        //Return true if contains all the bits of 1 are matched
        bool IsSet(BitMask<T> m) const
        {
            return mask == m.mask;
        }

        u32 Get() const { return mask; }

    private:
        u32 mask = 0u;
    };
}