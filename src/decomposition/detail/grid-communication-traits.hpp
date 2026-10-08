/*
 * grid-communication-traits.hpp
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SCHNEK_DECOMPOSITION_DETAIL_GRID_COMMUNICATION_TRAITS_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_GRID_COMMUNICATION_TRAITS_HPP_

#include <memory>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include "config.hpp"
#include "../../util/scratchbuffer.hpp"

#ifdef SCHNEK_HAVE_KOKKOS
#include <Kokkos_Core.hpp>
#endif

namespace schnek {
  namespace detail {

    inline std::size_t checkedCommunicationBufferBytes(std::size_t count, std::size_t elementSize) {
      if (elementSize != 0 && count > std::numeric_limits<std::size_t>::max() / elementSize) {
        throw std::length_error("communication buffer size overflow");
      }
      return count * elementSize;
    }

    /**
     * @brief Identifies host memory for communication-buffer customisation.
     *
     * This tag is independent of the host allocator and can be used alongside
     * backend-specific memory-space types as a registry key.
     */
    struct HostCommunicationMemorySpace {};

#ifdef SCHNEK_HAVE_KOKKOS
    /**
     * @brief Reusable byte allocation for a Kokkos communication memory space.
     *
     * The allocation is deliberately untyped. Each communication operation
     * creates an unmanaged typed view over the retained bytes, so one registry
     * entry can serve different grid value types without releasing capacity.
     */
    template<class MemorySpace>
    class KokkosCommunicationBuffer {
      public:
        using ByteView = Kokkos::View<std::byte *, MemorySpace>;

        explicit KokkosCommunicationBuffer(std::size_t initial_bytes = 0) {
          reserve_bytes(initial_bytes);
        }

        void reserve_bytes(std::size_t bytes) {
          if (bytes <= capacity_bytes_) {
            return;
          }

          storage_ = ByteView(
              Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:communication-buffer"), bytes
          );
          capacity_bytes_ = bytes;
        }

        std::size_t capacity_bytes() const { return capacity_bytes_; }

        void reset() {}

        template<class T>
        auto make_vector(std::size_t n) {
          static_assert(alignof(T) <= alignof(std::max_align_t), "communication buffer type is over-aligned");
          const std::size_t bytes = checkedCommunicationBufferBytes(n, sizeof(T));
          reserve_bytes(bytes);
          using View = Kokkos::View<T *, MemorySpace, Kokkos::MemoryTraits<Kokkos::Unmanaged>>;
          return View(reinterpret_cast<T *>(storage_.data()), n);
        }

      private:
        ByteView storage_;
        std::size_t capacity_bytes_ = 0;
    };
#endif

    /**
     * @brief Selects the reusable communication buffer for a memory space.
     *
     * This is the allocator customisation point for grid backends. The host
     * tag uses the existing PMR-backed allocation; Kokkos memory spaces use a
     * byte View when Kokkos support is enabled.
     */
    template<class MemorySpace>
    struct CommunicationBufferTraits {
#ifdef SCHNEK_HAVE_KOKKOS
        using buffer_type = KokkosCommunicationBuffer<MemorySpace>;
#else
        using buffer_type = schnek::ScratchBuffer;
#endif
    };

    template<>
    struct CommunicationBufferTraits<HostCommunicationMemorySpace> {
        using buffer_type = schnek::ScratchBuffer;
    };

    /**
     * @brief Owns reusable send and receive buffers keyed by memory-space type.
     *
     * The lookup API is compile-time typed while the registry stores entries
     * heterogeneously so one decomposition can serve multiple memory spaces.
     * Backend-specific buffer types are selected by
     * CommunicationBufferTraits<MemorySpace>.
     */
    class BufferRegistry {
      public:
        struct EntryBase {
            virtual ~EntryBase() = default;
        };

        template<class BufferType>
        struct Entry : EntryBase {
            BufferType sendBuffer;
            BufferType receiveBuffer;
            BufferType partitionBuffer;

            BufferType &send() { return sendBuffer; }
            BufferType &receive() { return receiveBuffer; }
            BufferType &partition() { return partitionBuffer; }
        };

        template<class MemorySpace>
        auto &get() {
          using BufferType = typename CommunicationBufferTraits<MemorySpace>::buffer_type;
          using EntryType = Entry<BufferType>;
          const std::type_index key{typeid(MemorySpace)};
          auto &entry = entries[key];
          if (!entry) {
            entry = std::make_unique<EntryType>();
          }
          return static_cast<EntryType &>(*entry);
        }

      private:
        std::unordered_map<std::type_index, std::unique_ptr<EntryBase>> entries;
    };

    /**
     * @brief Describes the communication memory space used by a grid.
     *
     * Storage policies that expose both `MemorySpace` and `host_accessible`
     * are treated as backend-managed storage. All other storage policies use
     * the host memory tag.
     */
    template<class GridType, typename Enable = void>
    struct GridCommunicationTraits {
        using memory_space = HostCommunicationMemorySpace;
        using buffer_type = typename CommunicationBufferTraits<memory_space>::buffer_type;

        static constexpr bool host_accessible = true;
        static constexpr bool device_resident = false;
    };

    template<class GridType>
    struct GridCommunicationTraits<
        GridType,
        std::void_t<typename GridType::storage_type::MemorySpace, decltype(GridType::storage_type::host_accessible)>> {
        using storage_type = typename GridType::storage_type;
        using memory_space = typename storage_type::MemorySpace;
        using buffer_type = typename CommunicationBufferTraits<memory_space>::buffer_type;

        static constexpr bool host_accessible = storage_type::host_accessible;
        static constexpr bool device_resident = !host_accessible;
    };

    template<class GridType>
    using GridBufferMemorySpace = typename GridCommunicationTraits<GridType>::memory_space;

    template<class GridType>
    using GridCommunicationBuffer = typename GridCommunicationTraits<GridType>::buffer_type;

    /**
     * @brief Describes the communication memory space used by a particle container.
     *
     * Containers that expose both `MemorySpace` and `host_accessible` (the same
     * convention as grid storage policies) are treated as backend-managed.
     * All other containers, e.g. `std::vector`, use the host memory tag.
     */
    template<class ContainerType, typename Enable = void>
    struct ParticleCommunicationTraits {
        using memory_space = HostCommunicationMemorySpace;
        using buffer_type = typename CommunicationBufferTraits<memory_space>::buffer_type;

        static constexpr bool host_accessible = true;
        static constexpr bool device_resident = false;
    };

    template<class ContainerType>
    struct ParticleCommunicationTraits<
        ContainerType,
        std::void_t<typename ContainerType::MemorySpace, decltype(ContainerType::host_accessible)>> {
        using memory_space = typename ContainerType::MemorySpace;
        using buffer_type = typename CommunicationBufferTraits<memory_space>::buffer_type;

        static constexpr bool host_accessible = ContainerType::host_accessible;
        static constexpr bool device_resident = !host_accessible;
    };

    template<class ContainerType>
    using ParticleBufferMemorySpace = typename ParticleCommunicationTraits<ContainerType>::memory_space;

    template<class ContainerType>
    using ParticleCommunicationBuffer = typename ParticleCommunicationTraits<ContainerType>::buffer_type;

  }  // namespace detail
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_GRID_COMMUNICATION_TRAITS_HPP
