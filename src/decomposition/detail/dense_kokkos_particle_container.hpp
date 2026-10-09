/*
 * dense_kokkos_particle_container.hpp
 *
 * Copyright 2026 Holger Schmitz
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
 *
 */
#ifndef SCHNEK_DECOMPOSITION_DETAIL_DENSE_KOKKOS_PARTICLE_CONTAINER_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_DENSE_KOKKOS_PARTICLE_CONTAINER_HPP_

#include <Kokkos_Core.hpp>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace schnek {

  /**
   * @brief Dense, contiguous array-of-structs particle container backed by a Kokkos view.
   *
   * This is the reference `ParticleContainerTraits` storage for Kokkos memory spaces. The container keeps its
   * particles in `[0, size())` of a single allocation with `capacity()` slots. Growing within capacity never
   * reallocates or moves particles. Growing beyond capacity allocates geometrically and copies the stored particles.
   * `reallocationCount()` reports how often that happened.
   *
   * Copies are deep, like `std::vector`. Element access is only available when `host_accessible` is true.
   *
   * @tparam Particle trivially copyable particle type
   * @tparam ExecSpace execution space used by the Kokkos dispatch paths
   * @tparam MemSpace memory space that stores the particles; must be accessible from `ExecSpace`
   */
  template<
      class Particle,
      class ExecSpace = Kokkos::DefaultExecutionSpace,
      class MemSpace = typename ExecSpace::memory_space>
  class DenseKokkosParticleContainer {
      static_assert(
          std::is_trivially_copyable_v<Particle>, "particles stored in a Kokkos view must be trivially copyable"
      );
      static_assert(
          Kokkos::SpaceAccessibility<ExecSpace, MemSpace>::accessible,
          "execution space cannot access the particle memory space"
      );

      using StorageView = Kokkos::View<Particle *, MemSpace>;

    public:
      using value_type = Particle;
      using ExecutionSpace = ExecSpace;
      using MemorySpace = MemSpace;
      static constexpr bool host_accessible = Kokkos::SpaceAccessibility<Kokkos::HostSpace, MemSpace>::accessible;

      DenseKokkosParticleContainer() = default;

      DenseKokkosParticleContainer(const DenseKokkosParticleContainer &other) { copyFrom(other); }

      DenseKokkosParticleContainer &operator=(const DenseKokkosParticleContainer &other) {
        if (this != &other) {
          DenseKokkosParticleContainer copy(other);
          swap(copy);
        }
        return *this;
      }

      DenseKokkosParticleContainer(DenseKokkosParticleContainer &&other) noexcept
          : storage_(std::exchange(other.storage_, StorageView{})),
            size_(std::exchange(other.size_, 0)),
            reallocations_(std::exchange(other.reallocations_, 0)) {}

      DenseKokkosParticleContainer &operator=(DenseKokkosParticleContainer &&other) noexcept {
        DenseKokkosParticleContainer moved(std::move(other));
        swap(moved);
        return *this;
      }

      ~DenseKokkosParticleContainer() = default;

      /// Number of stored particles.
      std::size_t size() const noexcept { return size_; }

      /// Number of particle slots allocated.
      std::size_t capacity() const noexcept { return storage_.extent(0); }

      bool empty() const noexcept { return size_ == 0; }

      /// Number of allocations that replaced the storage of this container since it was constructed.
      std::size_t reallocationCount() const noexcept { return reallocations_; }

      /**
       * Set the number of stored particles. Particles below `min(n, old size)` are kept. Particles added by growing are
       * not initialised. Storage grows geometrically when `n` exceeds the capacity.
       */
      void resize(std::size_t n) {
        if (n > capacity()) {
          reallocate(std::max(n, 2 * capacity()));
        }
        size_ = n;
      }

      /// Ensure capacity for at least `n` particles without changing the size.
      void reserve(std::size_t n) {
        if (n > capacity()) {
          reallocate(n);
        }
      }

      /// Append one particle. Copies from host memory, so it works for every memory space.
      void push_back(const Particle &value) {
        resize(size_ + 1);
        using HostView = Kokkos::View<const Particle *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged>;
        Kokkos::deep_copy(
            Kokkos::subview(storage_, Kokkos::pair<std::size_t, std::size_t>(size_ - 1, size_)), HostView(&value, 1)
        );
      }

      /// Pointer to the first particle in the memory space of the container. Empty containers may return null.
      Particle *data() noexcept { return storage_.data(); }
      const Particle *data() const noexcept { return storage_.data(); }

      Particle *begin() {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        return storage_.data();
      }
      Particle *end() {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        return storage_.data() + size_;
      }
      const Particle *begin() const {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        return storage_.data();
      }
      const Particle *end() const {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        return storage_.data() + size_;
      }

      Particle &operator[](std::size_t i) {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        return storage_(i);
      }
      const Particle &operator[](std::size_t i) const {
        static_assert(host_accessible, "element access requires host-accessible particle memory");
        return storage_(i);
      }

      /// Copy the stored particles into a host vector.
      std::vector<Particle> copyToHost() const {
        std::vector<Particle> result(size_);
        if (size_ > 0) {
          Kokkos::View<Particle *, Kokkos::HostSpace, Kokkos::MemoryUnmanaged> dst(result.data(), size_);
          Kokkos::deep_copy(dst, Kokkos::subview(storage_, Kokkos::pair<std::size_t, std::size_t>(0, size_)));
        }
        return result;
      }

      void swap(DenseKokkosParticleContainer &other) noexcept {
        std::swap(storage_, other.storage_);
        std::swap(size_, other.size_);
        std::swap(reallocations_, other.reallocations_);
      }

    private:
      void reallocate(std::size_t newCapacity) {
        StorageView next(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:dense-particles"), newCapacity);
        if (size_ > 0) {
          Kokkos::deep_copy(
              Kokkos::subview(next, Kokkos::pair<std::size_t, std::size_t>(0, size_)),
              Kokkos::subview(storage_, Kokkos::pair<std::size_t, std::size_t>(0, size_))
          );
        }
        storage_ = next;
        ++reallocations_;
      }

      void copyFrom(const DenseKokkosParticleContainer &other) {
        storage_ = StorageView(Kokkos::view_alloc(Kokkos::WithoutInitializing, "schnek:dense-particles"), other.size_);
        size_ = other.size_;
        if (size_ > 0) {
          Kokkos::deep_copy(
              Kokkos::subview(storage_, Kokkos::pair<std::size_t, std::size_t>(0, size_)),
              Kokkos::subview(other.storage_, Kokkos::pair<std::size_t, std::size_t>(0, size_))
          );
        }
      }

      StorageView storage_{};
      std::size_t size_ = 0;
      std::size_t reallocations_ = 0;
  };

}  // namespace schnek

#endif /* SCHNEK_DECOMPOSITION_DETAIL_DENSE_KOKKOS_PARTICLE_CONTAINER_HPP_ */
