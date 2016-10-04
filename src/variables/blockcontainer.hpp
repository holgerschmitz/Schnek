/*
 * blockcontainer.hpp
 *
 *  Created on: 14 Sep 2016
 *  Author: Holger Schmitz
 */

#ifndef SRC_BLOCKCONTAINER_HPP_
#define SRC_BLOCKCONTAINER_HPP_


#include <schnek/variables.hpp>

#include <list>

#include <boost/range.hpp>

template<class BlockType>
class ChildBlock;

template<class ChildType>
class BlockContainer
{
  friend ChildBlock<ChildType>;
  private:
    std::list<ChildType*> children;

    void addChild(ChildType *child)
    {
      children.push_back(child);
    }
  protected:
    typedef boost::iterator_range<typename std::list<ChildType*>::const_iterator> child_iterator_range;
    child_iterator_range childBlocks()
    {
      return child_iterator_range(children.begin(), children.end());
    }

    size_t numChildren()
    {
      return children.size();
    }

};

template<class BlockType>
class ChildBlock : public schnek::Block
{
  protected:
    void preInit()
    {
      boost::shared_ptr<BlockContainer<BlockType> > parent
        = boost::dynamic_pointer_cast<BlockContainer<BlockType> >(getParent());
      if (parent) {
        try {
          BlockType *self = dynamic_cast<BlockType *>(this);
          parent->addChild(self);
        }
        catch (std::bad_cast &e) {}
      }
    }
};


#endif /* SRC_BLOCKCONTAINER_HPP_ */
