/*
 * algorithm.hpp
 *
 * Created on: 12 Jul 2023
 * Author: hschmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2023 Holger Schmitz
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

#ifndef SCHNEK_COMPUTATION_ALGORITHM_HPP_
#define SCHNEK_COMPUTATION_ALGORITHM_HPP_

// Work in progress
// This file is brainstorming for a new way to implement algorithms in Schnek.
// The idea is to have a class that represents an algorithm. The algorithm is a list of steps that are executed in order.
// Each step is defined by a function that is executed on a specific architecture.
// The algorithm class also allows factories for fields to be registered that can be used on multiple architectures.
// An algorithm step informs the algorithm class which fields it needs and on which architecture
// and the algorithm class creates them using the registered factories.
namespace schnek {
    namespace computation {
        /**
         * Meta-type definition for an architecture:
         * 
         * struct Architecture {
         *   typedef ... GridStorageType;
         *   
         * };
         * 
         * We need compile-time checks for template parameters to check that they are architectures.
         */

        /**
         * 
         */
        template<typename ...Architecture>
        class ArchitectureCollection {
            public:
                static constexpr size_t size = sizeof...(Architecture);
        };

        template<typename FieldType>
        class FieldFactory {
            public:
                FieldType create(
                    const typename FieldType::RangeType &size,
                    const typename FieldTye::DomainType &domain,
                    const typename FieldTye::StaggerType &stagger, 
                    int ghostCells
                );
        };

        /**
         * A factory for fields that can be used on multiple architectures
         * 
         * The class defines a factory method for each architecture that can be used to create a field on that architecture.
         * This is used by the Algorithm class to create fields on the correct architecture.
         */
        template<typename FieldType, typename Architectures>
        class MultiArchitectureFieldFactory {
            public:
                
        };

        template<typename Architectures>
        class Algorithm {
            public:
                /**
                 * Register a field factory for all the di
                 */
                Registration registerFieldFactory(MultiArchitectureFieldFactory<FieldType> factory);

                /**
                 * Add a step to the algorithm
                 * 
                 * The step is added to the end of the algorithm.
                 * The AlgorithmStep also defines the architecture that the step is to be run on.
                 */
                void addStep(AlgorithmStep step);
        };
    } // namespace computation
} // namespace schnek

#endif // SCHNEK_COMPUTATION_ALGORITHM_HPP_ 