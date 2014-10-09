/*
 * blockclasses.hpp
 *
 * Created on: 26 Apr 2012
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

#ifndef SCHNEK_BLOCKCLASSES_HPP_
#define SCHNEK_BLOCKCLASSES_HPP_

#include <set>
#include <map>
#include <string>
#include "block.hpp"
#include "../exception.hpp"

#include <boost/shared_ptr.hpp>

namespace schnek {

class BlockNotFoundException : public SchnekException
{
  public:
    BlockNotFoundException() : SchnekException() {}
};

class BlockFactory
{
  public:
    virtual pBlock makeBlock(const std::string &name) = 0;
};
typedef boost::shared_ptr<BlockFactory> pBlockFactory;

template<class B>
class GenericBlockFactory : public BlockFactory
{
  public:
    pBlock makeBlock(const std::string &name)
    {
      pBlock pb(new B);
      pb->setName(name);
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

    template<class B> void setClass()
    {
        pBlockFactory bf(new GenericBlockFactory<B>());
        blockFactory = bf;
    }

    bool hasBlockFactory() { return blockFactory; }
    pBlock makeBlock(const std::string &name) { return blockFactory->makeBlock(name); }
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

    BlockClassDescriptor &addBlock(std::string blockClass);
    BlockClassDescriptor &operator()(std::string blockClass) { return this->get(blockClass); }
    BlockClassDescriptor &get(std::string blockClass);

    bool hasChild(std::string parent, std::string child);
    bool restrictBlocks() {return restr;}
};

} // namespace

#endif // SCHNEK_BLOCKCLASSES_HPP_
