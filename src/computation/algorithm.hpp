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

#include <algorithm>
#include <array>
#include <cstddef>
#include <list>
#include <map>

#include "../generic/type-util.hpp"
#include "../generic/typelist.hpp"
#include "../grid/field.hpp"
#include "../grid/grid.hpp"
#include "../util/unique.hpp"
#include "architecture.hpp"
#include "concepts/architecture-concept.hpp"

// Work in progress
// This file is brainstorming for a new way to implement algorithms in Schnek.
// The idea is to have a class that represents an algorithm. The algorithm is a list of steps that are executed in
// order. Each step is defined by a function that is executed on a specific architecture. The algorithm class also
// allows factories for fields to be registered that can be used on multiple architectures. An algorithm step informs
// the algorithm class which fields it needs and on which architecture and the algorithm class creates them using the
// registered factories.
namespace schnek::computation {

  template<typename... Architectures>
  class Algorithm;
  namespace internal {
    class RegistrationWrapper : public schnek::Unique<RegistrationWrapper> {
      public:
        virtual ~RegistrationWrapper() {}
    };
    typedef std::shared_ptr<RegistrationWrapper> pRegistrationWrapper;

    class AlgorithmStepWrapper : public schnek::Unique<AlgorithmStepWrapper> {
      public:
        virtual ~AlgorithmStepWrapper() {}
    };
    typedef std::shared_ptr<AlgorithmStepWrapper> pAlgorithmStepWrapper;

  }  // namespace internal

  /**
   * A Registration object is returned by the registerFieldFactory method of the Algorithm class.
   *
   * The registration object is used to keep track of the fields that have been registered. They
   * are used to reference fields in the algorithm steps.
   *
   * Registration objects can be copied and passed around, preserving their identity.
   */
  template<typename FieldType>
  class Registration {
    private:
      template<typename... Architectures>
      friend class Algorithm;

      template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
      friend class AlgorithmStepBuilder;

      MultiArchitectureFieldFactory<FieldType> &factory;
      internal::RegistrationWrapper *wrapper;

      std::list<std::pair<internal::pAlgorithmStepWrapper, bool>> steps;

      Registration(MultiArchitectureFieldFactory<FieldType> &factory) : factory(factory) {}

      void addAlgorithmStep(internal::pAlgorithmStepWrapper step, bool isInput) {
        steps.push_back(std::make_pair(step, isInput));
      }

    public:
      Registration(const Registration &other) : factory(other.factory), wrapper(other.wrapper) {}
      Registration &operator=(const Registration &other) {
        factory = other.factory;
        wrapper = other.wrapper;
        return *this;
      }

      long getId() const { return wrapper->getId(); }
  };

  namespace internal {
    template<typename FieldType>
    class RegistrationWrapperImpl : public RegistrationWrapper {
      public:
        Registration<FieldType> registration;
        RegistrationWrapperImpl(Registration<FieldType> registration) : registration(registration) {}
    };

    class AlgorithmAction {
      public:
        virtual void execute() = 0;
    };

    typedef std::unique_ptr<AlgorithmAction> pAlgorithmAction;
  }  // namespace internal

  template<size_t rank, typename FuncType, typename Architecture, typename... InputOutputDefinitions>
  class AlgorithmStep;

  template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
  class AlgorithmStepBuilder;

  template<typename... Architectures>
  class Algorithm {
    private:
      std::map<long, internal::pRegistrationWrapper> registrations;
      std::list<internal::pAlgorithmStepWrapper> steps;
      static_assert(
          (concepts::ArchitectureConcept<Architectures>::value && ...),
          "Architectures must meet ArchitecturesConcept requirements"
      );

    public:
      /**
       * Register a field factory for all the architectures in the collection
       */
      template<typename FieldType>
      Registration<FieldType> registerFieldFactory(MultiArchitectureFieldFactory<FieldType> &factory);

      /**
       * Add a step to the algorithm
       *
       * The step is added to the end of the algorithm.
       * The AlgorithmStep also defines the architecture that the step is to be run on.
       */
      template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
      void addStep(AlgorithmStep<rank, Architecture, InputOutputDefinitions...> &step);

      /**
       * Get a builder to create an AlgorithmStep
       */
      template<size_t rank, typename Architecture>
      AlgorithmStepBuilder<rank, Architecture> stepBuilder();

      /**
       * @brief Create a list of actions that represent the algorithm
       *
       * This is public for now to allow testing. It will be private in the final version.
       *
       * @return std::list<internal::pAlgorithmAction>
       */
      std::list<internal::pAlgorithmAction> makeActions();
  };

  namespace internal {
    template<size_t tRank, typename tGhostCells, typename tFieldType>
    struct InputDefinition {
        static constexpr bool isInput = true;
        static constexpr bool isOutput = false;
        static constexpr size_t rank = tRank;
        typedef tGhostCells GhostCells;
        typedef tFieldType FieldType;
    };

    template<size_t tRank, typename tGhostCells, typename tFieldType>
    struct OutputDefinition {
        static constexpr bool isInput = false;
        static constexpr bool isOutput = true;
        static constexpr size_t rank = tRank;
        typedef tGhostCells GhostCells;
        typedef tFieldType FieldType;
    };

    template<typename InputOutputDefinition>
    struct IsInputDefinition {
        static constexpr bool value = InputOutputDefinition::isInput;
    };

    template<typename InputOutputDefinition>
    struct IsOutputDefinition {
        static constexpr bool value = InputOutputDefinition::isOutput;
    };

    template<typename InputOutputDefinition>
    struct IODefinitionToRegistration {
        typedef Registration<typename InputOutputDefinition::FieldType> *type;
    };

    template<typename... InputOutputDefinitions>
    using InputRegistrationsTuple = typename generic::TypeList<InputOutputDefinitions...>::filter<
        internal::IsInputDefinition>::map<internal::IODefinitionToRegistration>::apply<std::tuple>;

    template<typename... InputOutputDefinitions>
    using OutputRegistrationsTuple = typename generic::TypeList<InputOutputDefinitions...>::filter<
        internal::IsOutputDefinition>::map<internal::IODefinitionToRegistration>::apply<std::tuple>;

  }  // namespace internal

  template<size_t rank, typename FuncType, typename Architecture, typename... InputOutputDefinitions>
  class AlgorithmStep {
    private:
      template<typename... Architectures>
      friend class Algorithm;

    public:
      using InputRegistrationsTuple = internal::InputRegistrationsTuple<InputOutputDefinitions...>;
      using OutputRegistrationsTuple = internal::OutputRegistrationsTuple<InputOutputDefinitions...>;

    private:
      InputRegistrationsTuple inputRegistrations;
      OutputRegistrationsTuple outputRegistrations;
      FuncType func;

    public:
      AlgorithmStep(
          InputRegistrationsTuple &inputRegistrations, OutputRegistrationsTuple &outputRegistrations, FuncType func
      )
          : inputRegistrations(inputRegistrations), outputRegistrations(outputRegistrations), func(func) {}

      AlgorithmStep(const AlgorithmStep &other) = default;
  };

  namespace internal {
    template<size_t rank, typename FuncType, typename Architecture, typename... InputOutputDefinitions>
    class AlgorithmStepWrapperImpl : public AlgorithmStepWrapper {
      public:
        AlgorithmStep<rank, FuncType, Architecture, InputOutputDefinitions...> step;
        AlgorithmStepWrapperImpl(AlgorithmStep<rank, FuncType, Architecture, InputOutputDefinitions...> step)
            : step(step) {}
    };
  }  // namespace internal

  template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
  class AlgorithmStepBuilder {
    public:
      using InputRegistrationsTuple = internal::InputRegistrationsTuple<InputOutputDefinitions...>;
      using OutputRegistrationsTuple = internal::OutputRegistrationsTuple<InputOutputDefinitions...>;

    private:
      InputRegistrationsTuple inputRegistrations;
      OutputRegistrationsTuple outputRegistrations;

    public:
      AlgorithmStepBuilder(
          const InputRegistrationsTuple &inputRegistrations, const OutputRegistrationsTuple &outputRegistrations
      )
          : inputRegistrations(inputRegistrations), outputRegistrations(outputRegistrations) {}

      template<typename GhostCells, typename FieldType>
      AlgorithmStepBuilder<
          rank,
          Architecture,
          InputOutputDefinitions...,
          internal::InputDefinition<rank, GhostCells, FieldType>>
      input(Registration<FieldType> &registration, GhostCells) {
        using NewInputRegistrationsTuple = internal::InputRegistrationsTuple<
            InputOutputDefinitions..., internal::InputDefinition<rank, GhostCells, FieldType>>;
        Registration<FieldType> *originalRegistration =
            &dynamic_cast<internal::RegistrationWrapperImpl<FieldType> *>(registration.wrapper)->registration;

        auto newInputRegistrations =
            generic::tupleAssign<InputRegistrationsTuple, NewInputRegistrationsTuple>(inputRegistrations);
        std::get<std::tuple_size<NewInputRegistrationsTuple>::value - 1>(newInputRegistrations) = originalRegistration;

        return AlgorithmStepBuilder<
            rank, Architecture, InputOutputDefinitions..., internal::InputDefinition<rank, GhostCells, FieldType>>(
            newInputRegistrations, outputRegistrations
        );
      }

      template<typename GhostCells, typename FieldType>
      AlgorithmStepBuilder<
          rank,
          Architecture,
          InputOutputDefinitions...,
          internal::OutputDefinition<rank, GhostCells, FieldType>>
      output(Registration<FieldType> &registration, GhostCells) {
        using NewOutputRegistrationsTuple = internal::OutputRegistrationsTuple<
            InputOutputDefinitions..., internal::OutputDefinition<rank, GhostCells, FieldType>>;
        Registration<FieldType> *originalRegistration =
            &dynamic_cast<internal::RegistrationWrapperImpl<FieldType> *>(registration.wrapper)->registration;
        auto newOutputRegistrations =
            generic::tupleAssign<OutputRegistrationsTuple, NewOutputRegistrationsTuple>(outputRegistrations);
        std::get<std::tuple_size<NewOutputRegistrationsTuple>::value - 1>(newOutputRegistrations) =
            originalRegistration;

        return AlgorithmStepBuilder<
            rank, Architecture, InputOutputDefinitions..., internal::OutputDefinition<rank, GhostCells, FieldType>>(
            inputRegistrations, newOutputRegistrations
        );
      }

      template<typename FunctionObject>
      AlgorithmStep<rank, FunctionObject, Architecture, InputOutputDefinitions...> build(FunctionObject func) {
        return AlgorithmStep<rank, FunctionObject, Architecture, InputOutputDefinitions...>{
            inputRegistrations, outputRegistrations, func
        };
      }
  };

  namespace internal {
    /**
     * @brief Records the state of the fields in the algorithm as the algroithm is executed
     *
     * @tparam Architectures The architectures that the algorithm is run on
     */
    template<typename... Architectures>
    struct AlgorithmState {
        enum class State { GOOD, OLD, LOCAL };

        /**
         * @brief Maps registration IDs to the field states
         */
        typedef std::map<long, State> FieldStates;
        std::array<FieldStates, sizeof...(Architectures)> fieldStates;
    };
  }  // namespace internal

  //=================================================================
  //========================= Algorithm =============================
  //=================================================================

  template<typename... Architectures>
  template<typename FieldType>
  Registration<FieldType> Algorithm<Architectures...>::registerFieldFactory(
      MultiArchitectureFieldFactory<FieldType> &factory
  ) {
    Registration<FieldType> registration{factory};
    internal::pRegistrationWrapper wrapper =
        std::make_shared<internal::RegistrationWrapperImpl<FieldType>>(registration);
    registration.wrapper = wrapper.get();
    registrations[registration.getId()] = wrapper;
    return registration;
  }

  template<typename... Architectures>
  template<size_t rank, typename Architecture>
  AlgorithmStepBuilder<rank, Architecture> Algorithm<Architectures...>::stepBuilder() {
    static_assert(rank > 0, "Rank must be greater than 0");
    static_assert(
        concepts::ArchitectureConcept<Architecture>::value, "Architecture must meet ArchitectureConcept requirements"
    );
    return AlgorithmStepBuilder<rank, Architecture>(std::tuple<>(), std::tuple<>());
  }

  template<typename... Architectures>
  template<size_t rank, typename Architecture, typename... InputOutputDefinitions>
  void Algorithm<Architectures...>::addStep(AlgorithmStep<rank, Architecture, InputOutputDefinitions...> &step) {
    internal::pAlgorithmStepWrapper wrapper =
        std::make_shared<internal::AlgorithmStepWrapperImpl<rank, Architecture, InputOutputDefinitions...>>(step);
    steps.push_back(wrapper);
    // Add the step to each input and output registration
    std::apply([&wrapper](auto... r) { (r->addAlgorithmStep(wrapper, true), ...); }, step.inputRegistrations);
    std::apply([&wrapper](auto... r) { (r->addAlgorithmStep(wrapper, false), ...); }, step.outputRegistrations);
  }

  template<typename... Architectures>
  std::list<internal::pAlgorithmAction> Algorithm<Architectures...>::makeActions() {
    std::list<internal::pAlgorithmAction> actions;
    internal::AlgorithmState<Architectures...> state;
    for (auto &step : steps) {
      // check peconditions of the step; the required registrations must be present and in the GOOD state
      // on at least one architecture

      // create the necessary actions that copy data between architectures and/or update boundary cells

      // create the action that runs the step

      // update the state of the fields

      // Question: What other infrastructure is needed to support this?
      // - Structures to hold the data on the architectures
      // - A way to copy data between architectures
      // - A way to update boundary cells
    }
    return actions;
  }
}  // namespace schnek::computation

#endif  // SCHNEK_COMPUTATION_ALGORITHM_HPP_