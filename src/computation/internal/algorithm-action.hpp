/*
 * algorithm_action.hpp
 *
 * Created on: 30 Aug 2024
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
 */

#ifndef SCHNEK_COMPUTATION_INTERNAL_ALGORITHM_ACTION_HPP_
#define SCHNEK_COMPUTATION_INTERNAL_ALGORITHM_ACTION_HPP_

#include "../concepts/field-type-concept.hpp"
#include "../concepts/architecture-concept.hpp"

#include <memory>

namespace schnek::computation::internal {

    class AlgorithmAction {
      public:
        virtual void execute() = 0;
    };

    typedef std::unique_ptr<AlgorithmAction> pAlgorithmAction;

    // template<typename FieldType, typename ArchitectureFrom, typename ArchitectureTo>
    // class AlgorithmCopyAction : public AlgorithmAction {
    //     static_assert(schnek::computation::concepts::FieldTypeConcept<FieldType>::value,
    //                   "FieldType must satisfy FieldTypeConcept");
    //     static_assert(schnek::computation::concepts::ArchitectureConcept<ArchitectureFrom>::value,
    //                   "ArchitectureFrom must satisfy ArchitectureConcept");
    //     static_assert(schnek::computation::concepts::ArchitectureConcept<ArchitectureTo>::value,
    //                   "ArchitectureTo must satisfy ArchitectureConcept");
    //   public:

    //     void execute() override {
    //       to_.copyFrom(from_, field_);
    //     }
    // }
}

#endif  // SCHNEK_COMPUTATION_INTERNAL_ALGORITHM_ACTION_HPP_