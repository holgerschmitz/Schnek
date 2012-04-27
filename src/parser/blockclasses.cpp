/*
 * blockclasses.cpp
 *
 *  Created on: 26 Apr 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#include "blockclasses.hpp"

using namespace schnek;

BlockClassDescriptor::BlockClassChildAdder&
  BlockClassDescriptor::BlockClassChildAdder::operator()(std::string child)
{
  descriptor.doAddChild(child);
  return *this;
}

void BlockClassDescriptor::doAddChild(std::string child)
{
  allowedChildren.insert(child);
}

BlockClassDescriptor::BlockClassChildAdder BlockClassDescriptor::addChildren(std::string child)
{
  doAddChild(child);
  return BlockClassDescriptor::BlockClassChildAdder(*this);
}

bool BlockClassDescriptor::hasChild(std::string child)
{
  return allowedChildren.count(child)>0;
}


BlockClassDescriptor &BlockClasses::addBlockClass(std::string blockClass)
{
  pBlockClassDescriptor bcs(new BlockClassDescriptor());
  (*classDescriptors)[blockClass] = bcs;
  return *bcs;
}

bool BlockClasses::hasChild(std::string parent, std::string child)
{
  if (classDescriptors->count(parent) < 1)
    throw BlockNotFoundException();

  pBlockClassDescriptor bcs = (*classDescriptors)[parent];
  return bcs->hasChild(child);
}
