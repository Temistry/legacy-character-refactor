#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace portfolio
{
template <typename T>
class SimpleMemoryPool
{
public:
    explicit SimpleMemoryPool(std::size_t capacity)
        : capacity_(capacity)
    {
    }

    template <typename... Args>
    T* Acquire(Args&&... args)
    {
        if (!freeList_.empty())
        {
            T* object = freeList_.back();
            freeList_.pop_back();
            *object = T(std::forward<Args>(args)...);
            return object;
        }

        if (objects_.size() >= capacity_)
        {
            return nullptr;
        }

        objects_.push_back(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
        return objects_.back().get();
    }

    void Release(T* object)
    {
        if (object != nullptr)
        {
            freeList_.push_back(object);
        }
    }

    std::size_t Capacity() const
    {
        return capacity_;
    }

    std::size_t AllocatedCount() const
    {
        return objects_.size();
    }

private:
    std::size_t capacity_;
    std::vector<std::unique_ptr<T> > objects_;
    std::vector<T*> freeList_;
};
}

