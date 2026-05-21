#include "CharacterSlotStore.h"

namespace portfolio
{
CharacterSlotStore::CharacterSlotStore(std::size_t capacity)
    : capacity_(capacity)
{
}

Character* CharacterSlotStore::Acquire(CharacterKind kind)
{
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& slot = slots_[i];
        if (!slot.inUse && slot.object.get() != nullptr && slot.object->Kind() == kind)
        {
            slot.inUse = true;
            return slot.object.get();
        }
    }

    if (slots_.size() >= capacity_)
    {
        return nullptr;
    }

    Slot slot;
    slot.object = factory_.Create(kind);
    slot.inUse = true;
    slots_.push_back(std::move(slot));
    return slots_.back().object.get();
}

void CharacterSlotStore::Release(Character* character)
{
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        Slot& slot = slots_[i];
        if (slot.object.get() == character)
        {
            slot.inUse = false;
            return;
        }
    }
}

bool CharacterSlotStore::Contains(const Character* character) const
{
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        if (slots_[i].object.get() == character)
        {
            return true;
        }
    }
    return false;
}

std::size_t CharacterSlotStore::AvailableCount() const
{
    return capacity_ - InUseCount();
}

std::size_t CharacterSlotStore::Capacity() const
{
    return capacity_;
}

std::size_t CharacterSlotStore::AllocatedCount() const
{
    return slots_.size();
}

std::size_t CharacterSlotStore::InUseCount() const
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < slots_.size(); ++i)
    {
        if (slots_[i].inUse)
        {
            ++count;
        }
    }
    return count;
}
}
