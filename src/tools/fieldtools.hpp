/*
 * fieldtools.hpp
 *
 * Created on: 31 Aug 2012
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2012 Holger Schmitz
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

#ifndef SCHNEK_FIELDTOOLS_HPP_
#define SCHNEK_FIELDTOOLS_HPP_

#include "../grid/field.hpp"
#include "../variables/dependencies.hpp"

namespace schnek {

template<
  typename T,
  int rank,
  template<int> class GridCheckingPolicy,
  template<int> class ArrayCheckingPolicy,
  template<typename, int> class StoragePolicy
>
void fill_field(
    Field<T, rank, GridCheckingPolicy, StoragePolicy> &field,
    Array<double, rank, ArrayCheckingPolicy> &coords,
    T &value,
    DependencyUpdater &updater);

template<
  typename T,
  int rank,
  template<int> class GridCheckingPolicy,
  template<int> class ArrayCheckingPolicy,
  template<typename, int> class StoragePolicy
>
void fill_field(
    Field<T, rank, GridCheckingPolicy, StoragePolicy> &field,
    Array<double, rank, ArrayCheckingPolicy> &coords,
    T &value,
    DependencyUpdater &updater,
    pParameter dependent);


class FieldFiller
{
  private:

    class implBase
    {
      public:
        virtual void fill() = 0;
    };

    template<
      typename T,
      int rank,
      template<int> class GridCheckingPolicy,
      template<int> class ArrayCheckingPolicy,
      template<typename, int> class StoragePolicy
    >
    class impl : public implBase
    {
      private:
        Field<T, rank, GridCheckingPolicy, StoragePolicy> &field;
        Array<double, rank, ArrayCheckingPolicy> &coords;
        T &value;
        DependencyUpdater &updater;

      public:
        impl(Field<T, rank, GridCheckingPolicy, StoragePolicy> &field_,
             Array<double, rank, ArrayCheckingPolicy> &coords_,
             T &value_,
             DependencyUpdater &updater_)
          : field(field_), coords(coords_), value(value_), updater(updater_) {}

        void fill()
        {
          fill_field(field, coords, value, updater);
        }
    };

    typedef boost::shared_ptr<implBase> pImplBase;
    std::list<pImplBase> implementations;
  public:

    template<
      int rank,
      template<int> class ArrayCheckingPolicy
    >
    class fieldAdder
    {
      private:
        friend class FieldFiller;
        Array<double, rank, ArrayCheckingPolicy> &coords;
        DependencyUpdater &updater;
        std::list<pImplBase> &implementations;

        fieldAdder(Array<double, rank, ArrayCheckingPolicy> &coords_,
                   DependencyUpdater &updater_,
                   std::list<pImplBase> &implementations_)
          : coords(coords_), updater(updater_), implementations(implementations_) {}

      public:

        template<
          typename T,
          template<int> class GridCheckingPolicy,
          template<typename, int> class StoragePolicy
        >
        fieldAdder &operator()(Field<T, rank, GridCheckingPolicy, StoragePolicy> &field, T& value)
        {
          pImplBase i(new impl<T, rank, GridCheckingPolicy, ArrayCheckingPolicy, StoragePolicy>(
              field, coords, value, updater
          ));
          implementations.push_back(i);
          return *this;
        }
    };

    FieldFiller() {}

    template<
      int rank,
      template<int> class ArrayCheckingPolicy
    >
    fieldAdder<rank,ArrayCheckingPolicy> &set(Array<double, rank, ArrayCheckingPolicy> &coords, DependencyUpdater &updater)
    {
        static fieldAdder<rank,ArrayCheckingPolicy> adder(coords, updater, implementations);
        return adder;
    }

    void clear() { implementations.clear(); }

    void fillFields()
    {
      BOOST_FOREACH(pImplBase i, implementations) i->fill();
    }

};

} // namespace

#include "fieldtools.t"

#endif // FIELDFILLER_HPP_ 
