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

#include <memory>
#include <list>

namespace schnek
{

template<class BlockType>
class ChildBlock;

/** @brief A container for child blocks of a given type
 *
 * Block implementations can inherit from this class template to signal that
 * they contain a number of child blocks of a given type. The @c ChildType
 * can be a base class with multiple implementations.
 *
 * This class works in conjunction with ChildBlock<ChildType>. Only if the
 * @c ChildType extends ChildBlock<ChildType> will any children be added to
 * this container.
 *
 * @c BlockContainer makes available two methods that allow querying and
 * iterating over the child blocks
 *
 */
template<class ChildType>
class BlockContainer
{
  friend ChildBlock<ChildType>;
  public:
    /// The iterator range type that is returned by childBlocks()
    typedef boost::iterator_range<typename std::list<std::shared_ptr<ChildType> >::const_iterator> iterator_range;
    /// The iterator type
    typedef typename iterator_range::iterator iterator;
  private:
    /// A container for all the children
    std::list<std::shared_ptr<ChildType> > children;

    /** @brief The function to add a single child
     *
     * This function is called by ChildBlock<ChildType>
     */
    void addChild(std::shared_ptr<ChildType> child)
    {
      children.push_back(child);
    }
  protected:
    /** @brief Return the child blocks
     *
     * This method returns an iterator range over the child blocks. Children are
     * returned in the order they have been added which is usually the order
     * in which they have been specified in the setup file.
     */
    iterator_range childBlocks()
    {
      return iterator_range(children.begin(), children.end());
    }

    /// Returns the number of child blocks
    size_t numChildren()
    {
      return children.size();
    }
};

template<class BlockType>
class ChildBlock : public schnek::Block, public std::enable_shared_from_this<ChildBlock<BlockType> >
{
  public:
    ChildBlock(pBlock parent = pBlock()) : schnek::Block(parent) {}
  protected:
    void preInit()
    {
      schnek::Block *pParent = getParent().get();
      BlockContainer<BlockType>* parent = dynamic_cast<BlockContainer<BlockType> *>(pParent);
      std::shared_ptr<BlockType> self = std::dynamic_pointer_cast<BlockType>(this->shared_from_this());
      if (parent && self) parent->addChild(self);
    }
};

} // namespace

#endif // SCHNEK_BLOCKCONTAINER_HPP_
