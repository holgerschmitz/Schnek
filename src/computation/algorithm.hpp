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

#include "../grid/grid.hpp"
#include "../grid/field.hpp"

#include <cstddef>
#include <algorithm>
#include <list>

// Work in progress
// This file is brainstorming for a new way to implement algorithms in Schnek.
// The idea is to have a class that represents an algorithm. The algorithm is a list of steps that are executed in order.
// Each step is defined by a function that is executed on a specific architecture.
// The algorithm class also allows factories for fields to be registered that can be used on multiple architectures.
// An algorithm step informs the algorithm class which fields it needs and on which architecture
// and the algorithm class creates them using the registered factories.
namespace schnek {
    namespace computation {
        class Algorithm;

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
        struct ArchitectureCollection {
            static constexpr size_t size = sizeof...(Architecture);
        };

        /**
         * Used as the FieldType for Fields that can be used on multiple architectures
         */
        template<
            typename T,
            size_t rank,
            template<size_t> class CheckingPolicy = GridNoArgCheck
        >
        struct FieldTypeWrapper {
            template<template<typename, size_t> typename StorageType>
            using type = Field<T, rank, CheckingPolicy, StorageType>;
        };

        /**
         * Used as the FieldType for Grids that can be used on multiple architectures
         */
        template<
            typename T,
            size_t rank,
            template<size_t> class CheckingPolicy = GridNoArgCheck
        >
        struct GridTypeWrapper {
            template<template<typename, size_t> typename StorageType>
            using type = Grid<T, rank, CheckingPolicy, StorageType>;
        };
        
        struct SimpleHostArchitecture {
            template<typename T, size_t rank> 
            using GridStorageType = SingleArrayGridStorage<T, rank>;
        };

        /**
         * A factory for fields that can be used on a single architecture
         * 
         * @tparam FieldType The type of field that the factory creates. 
         */
        template<class FieldType>
        struct MultiArchitectureFieldFactory {
            template<typename Architecture>
            typename FieldType::type<Architecture::template GridStorageType> create(
                const typename FieldType::type<Architecture::template GridStorageType>::RangeType &size,
                const typename FieldType::type<Architecture::template GridStorageType>::DomainType &domain,
                const typename FieldType::type<Architecture::template GridStorageType>::StaggerType &stagger, 
                int ghostCells
            );
        };

        template<
            typename T,
            size_t rank,
            template<size_t> class CheckingPolicy
        >
        struct MultiArchitectureFieldFactory< FieldTypeWrapper<T, rank, CheckingPolicy> > {

            template<typename Architecture>
            typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType> create(
                const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>::RangeType &size,
                const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>::DomainType &domain,
                const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>::StaggerType &stagger, 
                int ghostCells
            )
            {
                typedef typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType> FieldType;
                return FieldType(size, domain, stagger, ghostCells);
            }
        };

        // template<
        //     typename T,
        //     size_t rank,
        //     template<size_t> class CheckingPolicy
        // >
        // struct MultiArchitectureFieldFactory<GridTypeWrapper<T, rank, CheckingPolicy>::type> {
        //     template<typename Architecture>
        //     static typename FieldType<Architecture>::type create(
        //         const typename FieldType::RangeType &size,
        //         const typename FieldType::DomainType &domain,
        //         const typename FieldType::StaggerType &stagger, 
        //         int ghostCells
        //     );
        // };

        /**
         * A Registration object is returned by the registerFieldFactory method of the Algorithm class.
         * 
         * The registration object is used to keep track of the fields that have been registered. They
         * are used to reference fields in the algorithm steps.
         */
        class Registration {
            public:
                class RegistrationRef {
                    virtual ~RegistrationRef() {}
                };
            private:
                
                std::shared_ptr<RegistrationRef> ref;
                Registration(std::shared_ptr<RegistrationRef> ref): ref(ref) {}
                friend class Algorithm;
            public:

                Registration(const Registration &other): ref(other.ref) {}
                Registration(Registration &&other): ref(std::move(other.ref)) {}
                Registration &operator=(const Registration &other) {
                    ref = other.ref;
                    return *this;
                }
                Registration &operator=(Registration &&other) {
                    ref = std::move(other.ref);
                    return *this;
                }
        };

        namespace internal {

            template<typename FieldType>
            class RegistrationImpl : public Registration::RegistrationRef {
                private:
                    MultiArchitectureFieldFactory<FieldType> &factory;
                public:
                    RegistrationImpl(MultiArchitectureFieldFactory<FieldType> &factory): factory(factory) {}
            };

        }

        template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
        class AlgorithmStep;

        template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
        class AlgorithmStepBuilder;


        class Algorithm {
            private:
                std::list<Registration> registrations;
            public:
                /**
                 * Register a field factory for all the architectures in the collection
                 */
                template<typename FieldType>
                Registration registerFieldFactory(MultiArchitectureFieldFactory<FieldType> &factory);

                /**
                 * Add a step to the algorithm
                 * 
                 * The step is added to the end of the algorithm.
                 * The AlgorithmStep also defines the architecture that the step is to be run on.
                 */
                template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
                void addStep(AlgorithmStep<rank, Architecture, InputOutputDefinitions...> step);

                /**
                 * Get a builder to create an AlgorithmStep
                 */
                template<size_t rank, typename Architecture>
                AlgorithmStepBuilder<rank, Architecture> stepBuilder();
        };

        namespace internal {
            template<size_t tRank, typename tGhostCells>
            struct InputDefinition {
                static constexpr bool isInput = true;
                static constexpr bool isOutput = false;
                static constexpr size_t rank = tRank;
                typedef tGhostCells GhostCells;
            };

            template<size_t tRank, typename tGhostCells>
            struct OutputDefinition {
                static constexpr bool isInput = false;
                static constexpr bool isOutput = true;
                static constexpr size_t rank = tRank;
                typedef tGhostCells GhostCells;
            };
        }

        template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
        class AlgorithmStepBuilder {
            private:
                std::list<Registration> inputRegistrations;
                std::list<Registration> outputRegistrations;
            public:
                AlgorithmStepBuilder(
                    std::list<Registration> &inputRegistrations, 
                    std::list<Registration> &outputRegistrations
                ): inputRegistrations(inputRegistrations), outputRegistrations(outputRegistrations)
                {}

                template<typename ghostCells>
                AlgorithmStepBuilder<
                    rank,
                    Architecture, 
                    InputOutputDefinitions..., 
                    internal::InputDefinition<rank, ghostCells>
                > input(Registration registration) {
                    inputRegistrations.push_back(registration);
                    return AlgorithmStepBuilder<
                        rank,
                        Architecture, 
                        InputOutputDefinitions..., 
                        internal::InputDefinition<rank, ghostCells>
                    >(inputRegistrations, outputRegistrations);
                }

                template<typename ghostCells>
                AlgorithmStepBuilder<
                    rank,
                    Architecture, 
                    InputOutputDefinitions..., 
                    internal::OutputDefinition<rank, ghostCells>
                > output(Registration registration) {
                    inputRegistrations.push_back(registration);
                    return AlgorithmStepBuilder<
                        rank,
                        Architecture, 
                        InputOutputDefinitions..., 
                        internal::OutputDefinition<rank, ghostCells>
                    >(inputRegistrations, outputRegistrations);
                }

                AlgorithmStep<rank, Architecture, InputOutputDefinitions...> build();
        };
    //=================================================================
    //======================= FieldFactory ============================
    //=================================================================

        // template<
        //     typename T,
        //     size_t rank,
        //     template<size_t> class CheckingPolicy
        // >
        // template<typename Architecture>
        // typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>
        //     MultiArchitectureFieldFactory< FieldTypeWrapper<T, rank, CheckingPolicy> >::create<Architecture>(
        //         const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>::RangeType &size,
        //         const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>::DomainType &domain,
        //         const typename FieldTypeWrapper<T, rank, CheckingPolicy>::type<Architecture::template GridStorageType>::StaggerType &stagger, 
        //         int ghostCells
        //     )

        //=================================================================
        //========================= Algorithm =============================
        //=================================================================

        template<typename FieldType>
        Registration Algorithm::registerFieldFactory(MultiArchitectureFieldFactory<FieldType> &factory) {
            auto ref = std::make_shared<internal::RegistrationImpl<FieldType>>(factory);
            auto registration = Registration(ref);
            registrations.push_back(registration);
            return registration;
        }

    } // namespace computation
} // namespace schnek

#endif // SCHNEK_COMPUTATION_ALGORITHM_HPP_ 