#pragma once
#include "platform/type.hpp"
#include "component.hpp"
#include <array>
#include <cassert>
namespace unknown::ecs
{
    struct ComponentsMask
    {
#define MASK_UNITS_COUNT 2u
    public:
        enum class MaskUnits
        {
            SINGLE,
            DOUBLE,
            // TRIPLE,
            // QUADRO,
        };

        std::array<u64, MASK_UNITS_COUNT> GetMasks() const { return _masks; }
        void Reset()
        {
            auto newMask = std::array<u64, MASK_UNITS_COUNT>({0U});
            _masks.swap(newMask);
        };

        void Set(ClassID componentClassId)
        {
            u32 index, slot;
            getLocal(componentClassId, index, slot);
            assert(index < MASK_UNITS_COUNT);
            u64 localMask = 1 << slot;
            _masks[index] |= localMask;
        }

        void Clear(ClassID componentClassId)
        {
            u32 index, slot;
            getLocal(componentClassId, index, slot);
            assert(index < MASK_UNITS_COUNT);
            u64 localMask = ~(1 << slot);
            _masks[index] &= localMask;
        }

        bool IsSet(ClassID componentClassId)
        {
            u32 index, slot;
            getLocal(componentClassId, index, slot);
            assert(index < MASK_UNITS_COUNT);
            u64 localMask = 1 << slot;
            return (_masks[index] & localMask) > 0u;
        }

        bool IsSet(ComponentsMask mask)
        {
            for (u64 i = 0; i < MASK_UNITS_COUNT; i++)
            {
                u64 otherMask = mask._masks[i];
                u64 res = _masks[i] & otherMask;
                if (res != otherMask)
                    return false;
            }
            return true;
        }
        
        bool IsEqual(ComponentsMask mask)
        {
            for (u64 i = 0; i < MASK_UNITS_COUNT; i++)
            {
                if(_masks[i] != mask._masks[i])
                    return false;
            }
            return true;
        }

        bool IsEmpty()
        {
            bool empty = true;
            for (u64 i = 0; i < MASK_UNITS_COUNT; i++)
            {
                if (_masks[i] > 0)
                    empty = false;
            }
            return empty;
        }

    private:
        void getLocal(ClassID componentClassId, u32 &index, u32 &slot)
        {
            index = componentClassId / 64u;
            slot = componentClassId % 64u;
        }

        std::array<u64, MASK_UNITS_COUNT> _masks{0u};
    };
}