/*
 * block.hpp
 *
 * Created on: 1 May 2012
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

#ifndef SCHNEK_BLOCK_HPP_
#define SCHNEK_BLOCK_HPP_

#include "blockparameters.hpp"
#include "blockdata.hpp"
#include "../util/unique.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include <stack>

namespace schnek {

class Block;
typedef boost::shared_ptr<Block> pBlock;
typedef std::list<pBlock> BlockList;
typedef boost::shared_ptr<BlockList> pBlockList;

class BlockTree
{
  private:
    pBlock root;
    pBlock cursor;
    std::stack<int> depth;
  public:
    BlockTree();
    void addChild(pBlock);
    void moveDown();
    void moveUp();
    pBlock getRoot() {return root; }
};

typedef boost::shared_ptr<BlockTree> pBlockTree;

/** Block defines the basic structure of a unit of simulation code
 *
 */
class Block : public Unique<Block>
{
  private:
    BlockParameters blockParameters;
    pBlock parent;
    BlockList children;
    std::string name;

    template<typename T>
    bool getData(std::string key, T* &data, bool upward);

    void registerHierarchy();
    void preInitHierarchy();
    void initHierarchy();
    void postInitHierarchy();

    void setParent(pBlock parent_) { parent=parent_; }
    friend class BlockTree;
  protected:
    virtual void initParameters(BlockParameters&) {}
    virtual void registerData() {}
    virtual void preInit() {}
    virtual void init() {}
    virtual void postInit() {}
    BlockList getChildren() { return BlockList(children); }
  public:
    Block(pBlock parent_ = pBlock()) : parent(parent_) {}
    virtual ~Block() {}

    void setContext(pBlockVariables context)
    {
      blockParameters.setContext(context);
    }

    pBlockVariables getLocalVariables()
    {
      return blockParameters.getContext();
    }

    pBlockVariables getVariables()
    {
      pBlockVariables vars = blockParameters.getContext();
      while (vars->getParent()) vars = vars->getParent();
      return vars;
    }


    void setup();
    void addChild(pBlock child);
    void evaluateParameters();

    pBlock getParent() { return parent; }

    template<typename T>
    void addData(std::string key, T &data);

    template<typename T>
    void retrieveData(std::string key, T* &data);

    template<typename T>
    void retrieveData(std::string key, T &data);

    void initAll();

    void setName(const std::string &name_) { name = name_; }
    std::string getName() { return name; }
};

// template functions

template<typename T>
bool Block::getData(std::string key, T* &data, bool upward)
{
  if (BlockData<T>::instance().exists(this->getId(), key))
  {
    data = BlockData<T>::instance().get(this->getId(), key);
    return true;
  }

  boost::iterator_range<std::string::iterator> dot = boost::find_first(key,".");
  if (!dot.empty())
  {
    std::string head = std::string(key.begin(), dot.begin());
    std::string tail = std::string(dot.end(), key.end());
    int count = 0;
    BOOST_FOREACH(pBlock child, children)
    {
      if ((child->getName()==head) && child->getData(tail, data, false)) ++count;
    }

    if (count>1) throw DuplicateVariableException();
    else if (count==1) return true;
  }

  if (upward && parent)
    return parent->getData(key, data, true);

  int count = 0;
  BOOST_FOREACH(pBlock child, children)
  {
    if (child->getData(key,data, false)) ++count;
  }

  if (count>1) throw DuplicateVariableException();
  return (count!=0);
}

template<typename T>
void Block::addData(std::string key, T &data)
{
  BlockData<T>::instance().add(this->getId(), key, data);
}

template<typename T>
void Block::retrieveData(std::string key, T* &data)
{
  T **datap;
  if (getData(key, datap, true)) {
    data = *datap;
  }
  else if (!getData(key, data, true))
    throw VariableNotFoundException("Could not find Block variable "+key);
}

template<typename T>
void Block::retrieveData(std::string key, T &data)
{
  T *datap;
  if (!getData(key, datap, true))
    throw VariableNotFoundException("Could not find Block variable "+key);
  data = *datap;
}


} // namespace

#endif // SCHNEK_BLOCK_HPP_
