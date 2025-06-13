//
// Created by balma on 6/12/25.
//
#pragma once
#include <cstddef>
#include <cstdlib>

class ArenaAllocator
{
  public:
    inline explicit ArenaAllocator(const size_t bytes) : m_size(bytes)
    {
        m_buffer = static_cast<std::byte*>(malloc(m_size));
        m_offset = m_buffer;
    }


    /**
         * This was a bit confusing, so I'll add this comment explaining it.
         * We create a temp var called offset that we will return.
         * Reason is so we can return the current location it is and increment the m_offset with the
         * new size
         */
    template<typename T>
    inline T* alloc()
    {
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    inline ArenaAllocator(const ArenaAllocator&) = delete;

    inline ArenaAllocator operator=(const ArenaAllocator&) = delete;

    inline ~ArenaAllocator()
    {
        free(m_buffer);
    }

  private:
    size_t m_size{};
    std::byte* m_buffer;
    std::byte* m_offset;
};