/*
 * blockdata.hpp
 *
 * Created on: 26 Jun 2012
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
 
#ifndef SCHNEK_BLOCKDATA_HPP_
#define SCHNEK_BLOCKDATA_HPP_

#include "../util/singleton.hpp"
#include <map>
#include <string>

namespace schnek
{

template<typename T>
class BlockData : public Singleton<BlockData<T> >
{
  private:
    typedef std::map<std::string, T> DataMap;
    typedef boost::shared_ptr<DataMap> pDataMap;
    typedef std::map<long, pDataMap> BlockDataMap;

    BlockDataMap blockDataMap;

  public:
    void add(long blockId, std::string key, T &data);
    T &get(long blockId, std::string key);
    bool exists(long blockId, std::string key);
};


template<typename T>
void BlockData<T>::add(long blockId, std::string key, T &data)
{
  if (0 == blockDataMap.count(blockId))
  {
    pDataMap pdm(new DataMap());
    blockDataMap[blockId] = pdm;
  }

  (*blockDataMap[blockId])[key] = data;
}

template<typename T>
T &BlockData<T>::get(long blockId, std::string key)
{
  if (0 == blockDataMap.count(blockId))
    throw VariableNotFoundException();
  if (0 == blockDataMap[blockId].count(key))
    throw VariableNotFoundException();

  return blockDataMap[blockId][key];
}

template<typename T>
bool BlockData<T>::exists(long blockId, std::string key)
{
  if (0 == blockDataMap.count(blockId)) return false;
  return (blockDataMap[blockId].count(key) > 0);
}

} //namespace


#endif // SCHNEK_BLOCKDATA_HPP_

