/*
 * databuffer.t
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


namespace schnek
{


template<class T>
template<class ContainerType>
void DataBuffer<T>::makeBuffer(ContainerType &container)
{
  static const int dsize = sizeof(T);
  buffer.resize(dsize*BufferType::IndexType(container.size()));
  unsigned char *data = buffer.getRawData();

  for (ContainerType::iterator it = container.begin(); it != container.end(); ++it)
  {
    // Dereference twice and then take address of because container could
    // contain iterators and not pointers.
    memcpy(data, &(**it), dsize);
    data += dsize;
  }
}


}


