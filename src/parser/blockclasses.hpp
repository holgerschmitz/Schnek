/*
 * blockclasses.hpp
 *
 *  Created on: 26 Apr 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef BLOCKCLASSES_HPP_
#define BLOCKCLASSES_HPP_

#include <set>
#include <map>
#include <string>
#include "../exception.hpp"

#include <boost/shared_ptr.hpp>

namespace schnek {

class Block;
typedef boost::shared_ptr<Block> pBlock;

class BlockNotFoundException : public SchnekException
{
  public:
    BlockNotFoundException() : SchnekException() {}
};

class BlockFactory
{
  public:
    virtual pBlock makeBlock() = 0;
};
typedef boost::shared_ptr<BlockFactory> pBlockFactory;

template<class B>
class GenericBlockFactory : public BlockFactory
{
  public:
    pBlock makeBlock()
    {
      pBlock pb(new B);
      return pb;
    }
};

class BlockClassDescriptor
{
  private:
    std::set<std::string> allowedChildren;
    pBlockFactory blockFactory;

    void doAddChild(std::string child);
  public:
    class BlockClassChildAdder
    {
      private:
       friend class BlockClassDescriptor;
       BlockClassDescriptor &descriptor;
       BlockClassChildAdder(BlockClassDescriptor &descriptor_) : descriptor(descriptor_) {}
      public:
       BlockClassChildAdder(const BlockClassChildAdder &adder) : descriptor(adder.descriptor) {}
       BlockClassChildAdder &operator()(std::string child);
    };
    friend class BlockClassDescriptor::BlockClassChildAdder;


  public:
    BlockClassChildAdder addChildren(std::string child);
    bool hasChild(std::string child);

    template<class B> void setBlockClass()
    {
        pBlockFactory bf(new GenericBlockFactory<B>());
        blockFactory = bf;
    }

    bool hasBlockFactory() { return blockFactory; }
    pBlock makeBlock() { return blockFactory->makeBlock(); }
};

typedef boost::shared_ptr<BlockClassDescriptor> pBlockClassDescriptor;

class BlockClasses
{
  private:
    typedef std::map<std::string, pBlockClassDescriptor> BlockClassDescriptors;
    boost::shared_ptr<BlockClassDescriptors> classDescriptors;
    bool restr;
  public:

    BlockClasses(bool restr_ = true) : classDescriptors(new BlockClassDescriptors), restr(restr_) {}
    BlockClasses(const BlockClasses& blockClasses)
        : classDescriptors(blockClasses.classDescriptors), restr(blockClasses.restr) {}

    BlockClassDescriptor &addBlockClass(std::string blockClass);
    BlockClassDescriptor &operator()(std::string blockClass) { return this->get(blockClass); }
    BlockClassDescriptor &get(std::string blockClass);

    bool hasChild(std::string parent, std::string child);
    bool restrictBlocks() {return restr;}
};

} // namespace

#endif /* BLOCKCLASSES_HPP_ */
