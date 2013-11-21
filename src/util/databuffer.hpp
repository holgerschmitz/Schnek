/*
 * databuffer.hpp
 *
 * Created on: 16 Apr 2013
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


#ifndef SCHNEK_DATABUFFER_HPP_
#define SCHNEK_DATABUFFER_HPP_

#include "../grid/grid.hpp"
#include "../grid/gridstorage.hpp"
#include "../grid/gridcheck.hpp"

namespace schnek {
template<class T>
class DataBuffer
{
  public:
    typedef Grid<unsigned char, 1, GridAssertCheck, LazyArrayGridStorage> BufferType;
  private:
    BufferType buffer;
  public:
    class iterator : public std::iterator<std::forward_iterator_tag, T>
     {
       private:
         friend class DataBuffer;
         unsigned char *data;

         iterator(unsigned char *data_) : data(data_) {}

       public:
         iterator() : data(NULL) {}

         iterator(const iterator &it) { data = it.data; }

         iterator& operator++()
         {
           data += sizeof(Particle);
           return *this;
         }

         iterator operator++(int)
         {
           iterator tmp(*this);
           operator++();
           return tmp;
         }

         bool operator==(const iterator& rhs)
         {
           return (data == rhs.data);
         }

         bool operator!=(const iterator& rhs)
         {
           return (data != rhs.data);
         }

         T& operator*()
         {
           return *((T*)(data));
         }

         T* operator->()
         {
           return (T*)(data);
         }
     };

    /** Make the buffer from some data. ContainerType is expected to
     * contain pointers (or iterators) to T.
     *
     * @param first
     * @param last
     */
    template<class ContainerType>
    void makeBuffer(ContainerType &container);

    BufferType &getBuffer()
    {
      return buffer;
    }

    iterator begin()
    {
      return iterator(buffer.getRawData());
    }

    iterator end()
    {
      return iterator(buffer.getRawData() + buffer.getSize());
    }

};

} // namespace schnek

#include "databuffer.t"

#endif
