/*
 * architecture.hpp
 *
 * Created on: 19 Jul 2024
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2024 Holger Schmitz
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

#ifndef SCHNEK_COMPUTATION_ARCHITECTURE_HPP_
#define SCHNEK_COMPUTATION_ARCHITECTURE_HPP_

#include "concepts/architecture-concept.hpp"
#include "internal/algorithm-action.hpp"

#include <vector>
#include <string>

namespace schnek::computation {
  /**
   * Meta-type definition for an architecture:
   *
   * struct ArchitectureTraits {
   *   typedef ... GridStorageType;
   *   static const std::string id;  // Unique identifier for the architecture
   *   typedef ... PreferredCopySource;  // Architecture to copy from if possible
   *   typedef TypeList<...> AllowedCopySources;  // List of architectures that can be copied from
   * };
   *
   */

  /**
   * @brief Abstract base class for architectures
   */
  class ArchitectureBase {
    public:
      virtual ~ArchitectureBase() = default;
      virtual std::string getId() const = 0;
      virtual std::string getPreferredCopySource() const = 0;
      virtual std::vector<std::string> getAllowedCopySources() const = 0;
      // virtual internal::pAlgorithmAction createCopyAction(std::string source) const = 0;
  };

  namespace internal {

    template<typename ...Architectures>
    struct ArchitectureIdListBuilder {
      static std::vector<std::string> build() {
        return {Architectures::id...};
      }
    }; 

  } // namespace internal

  /**
   * @brief Concrete architecture class
   *
   * @tparam Architecture Architecture type
   */
  template<typename ArchitectureTraits>
  class Architecture : public ArchitectureBase, public ArchitectureTraits {
    static_assert(
        concepts::ArchitectureConcept<ArchitectureTraits>::value,
        "ArchitectureTraits must define compatible architecture traits."
    );
    public:
      std::string getId() const override {
        return Architecture::id;
      }

      std::string getPreferredCopySource() const override {
        return Architecture::PreferredCopySource::id;
      }

      std::vector<std::string> getAllowedCopySources() const override {
        typedef typename ArchitectureTraits::AllowedCopySources::apply<
          internal::ArchitectureIdListBuilder
        > ArchitectureIdListBuilder;
        return ArchitectureIdListBuilder::build();
      }

      // internal::pAlgorithmAction createCopyAction() const override {
      // }
  };

  /**
   *
   */
  template<typename... Architecture>
  struct ArchitectureCollection {
      // static_assert(
      //     concepts::check_all_grid_storage_concepts<Architecture...>(),
      //     "All Architectures must define compatible GridStorageType."
      // );

      static constexpr size_t size = sizeof...(Architecture);
  };
}  // namespace schnek::computation

#endif  // SCHNEK_COMPUTATION_ARCHITECTURE_HPP_