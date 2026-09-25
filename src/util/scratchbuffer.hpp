
#ifndef SCHNEK_UTIL_SCRATCHBUFFER_HPP_
#define SCHNEK_UTIL_SCRATCHBUFFER_HPP_

  #include <memory_resource>
#include <vector>
#include <optional>
#include <cstddef>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace schnek {

    class ScratchBuffer {
      public:
        explicit ScratchBuffer(std::size_t initial_bytes = 0) {
            reserve_bytes(initial_bytes);
        }

        void reserve_bytes(std::size_t bytes) {
            if (bytes <= storage_.size()) {
                return;
            }

            // Precondition: no pmr::vector using this arena may be alive here.
            arena_.reset();
            storage_.resize(bytes);
            arena_.emplace(
                storage_.data(),
                storage_.size(),
                std::pmr::null_memory_resource()
            );
        }
        std::size_t capacity_bytes() const { return storage_.size(); }

        void reset() {
            // Precondition: no live pmr::vector using this arena.
            if (arena_) {
                arena_->release();
            }
        }

        std::pmr::memory_resource* resource() {
            if (!arena_) {
                reserve_bytes(1);
            }
            return &*arena_;
        }

        template<class T>
        std::pmr::vector<T> make_vector(std::size_t n) {
            static_assert(alignof(T) <= alignof(std::max_align_t), "scratch buffer type is over-aligned");
            if (sizeof(T) != 0 && n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
                throw std::length_error("scratch buffer size overflow");
            }
            const std::size_t bytes = n * sizeof(T);

            reserve_bytes(bytes);

            std::pmr::vector<T> v{resource()};
            v.resize(n);
            return v;
        }

      private:
        std::vector<std::byte> storage_;
        std::optional<std::pmr::monotonic_buffer_resource> arena_;
    };
}

#endif  // SCHNEK_UTIL_SCRATCHBUFFER_HPP_