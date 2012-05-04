/*
 * block.hpp
 *
 *  Created on: 1 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef BLOCK_HPP_
#define BLOCK_HPP_

#include "blockparameters.hpp"
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
class Block
{
  private:
    BlockParameters blockParameters;
    pBlock parent;
    BlockList children;
  protected:
    virtual void initParameters(BlockParameters&) = 0;
  public:
    Block(pBlock parent_ = pBlock()) : parent(parent_) {}

    void setContext(ParserContext context)
    {
      blockParameters.setContext(context);
    }

    void setup();
    void addChild(pBlock child);
    void evaluateParameters();

    pBlock getParent() { return parent; }

};


} // namespace

#endif /* BLOCK_HPP_ */
