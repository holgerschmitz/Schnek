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
#include "../util/unique.hpp"
#include <boost/shared_ptr.hpp>
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

    template<typename T>
    void getData(std::string key, T &data, bool upward);

    void initHierarchy();

  protected:
    virtual void initParameters(BlockParameters&) = 0;
    virtual void registerData() {}
    virtual void init() {}
  public:
    Block(pBlock parent_ = pBlock()) : parent(parent_) {}

    void setContext(pBlockVariables context)
    {
      blockParameters.setContext(context);
    }

    void setup();
    void addChild(pBlock child);
    void evaluateParameters();

    pBlock getParent() { return parent; }

    template<typename T>
    void addData(std::string key, const T &data);

    template<typename T>
    void retrieveData(std::string key, T &data);

    void initAll();
};


} // namespace

#endif // SCHNEK_BLOCK_HPP_
