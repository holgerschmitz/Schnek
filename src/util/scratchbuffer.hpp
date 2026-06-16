
  #include <memory_resource>
#include <vector>
#include <optional>
#include <cstddef>
#include <algorithm>
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
            // Conservative padding for alignment inside the arena.
            const std::size_t bytes = n * sizeof(T) + alignof(T) - 1;

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