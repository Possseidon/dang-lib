#pragma once

#include <cstddef>
#include <cstdlib>
#include <set>

#include "dang-lua/Allocator.h"

#include "catch2/catch.hpp"

class CheckedAllocator {
public:
    CheckedAllocator() = default;

    CheckedAllocator(const CheckedAllocator&) = delete;
    CheckedAllocator(CheckedAllocator&&) = delete;
    CheckedAllocator& operator=(const CheckedAllocator&) = delete;
    CheckedAllocator& operator=(CheckedAllocator&&) = delete;

    void checkNotEmpty() const { CHECK(allocations_.size() > 0); }

    void checkEmpty() const { CHECK(allocations_.size() == 0); }

    dang::lua::Allocator allocator() { return dang::lua::Allocator(alloc, this); }

private:
    static void* alloc(void* ud, void* ptr, std::size_t osize, std::size_t nsize)
    {
        auto& self = *static_cast<CheckedAllocator*>(ud);
        INFO("realloc " << ptr << " from " << osize << " to " << nsize);
        if (nsize > 0) {
            if (ptr != nullptr)
                CHECK(self.allocations_.erase(ptr) == 1);
            ptr = std::realloc(ptr, nsize);
            auto [_, inserted] = self.allocations_.insert(ptr);
            CHECK(inserted);
            return ptr;
        }
        if (ptr != nullptr) {
            CHECK(self.allocations_.erase(ptr) == 1);
            std::free(ptr);
        }
        return nullptr;
    }

    std::set<void*> allocations_;
};
