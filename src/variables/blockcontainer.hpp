/*
 * blockcontainer.hpp
 *
 *  Created on: 14 Sep 2016
 *  Author: Holger Schmitz
 */

#ifndef SCHNEK_BLOCKCONTAINER_HPP_
#define SCHNEK_BLOCKCONTAINER_HPP_

#include "block.hpp"

#include <boost/range.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <list>

namespace schnek
{

template<class BlockType>
class ChildBlock;

template<class ChildType>
class BlockContainer
{
  friend ChildBlock<ChildType>;
  public:
    typedef boost::iterator_range<typename std::list<boost::shared_ptr<ChildType> >::const_iterator> iterator_range;
    typedef typename iterator_range::iterator iterator;
  private:
    std::list<boost::shared_ptr<ChildType> > children;

    void addChild(boost::shared_ptr<ChildType> child)
    {
      children.push_back(child);
    }
  protected:
    iterator_range childBlocks()
    {
      return iterator_range(children.begin(), children.end());
    }

    size_t numChildren()
    {
      return children.size();
    }
};

template<class BlockType>
class ChildBlock : public schnek::Block, public boost::enable_shared_from_this<ChildBlock<BlockType> >
{
  protected:
    void preInit()
    {
      schnek::Block *pParent = getParent().get();
      BlockContainer<BlockType>* parent = dynamic_cast<BlockContainer<BlockType> *>(pParent);
      boost::shared_ptr<BlockType> self = boost::dynamic_pointer_cast<BlockType>(this->shared_from_this());
      if (parent && self) parent->addChild(self);
    }
};

} // namespace

#endif // SCHNEK_BLOCKCONTAINER_HPP_
