/*
 * blockclasses.cpp
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

BlockClassDescriptor &BlockClasses::get(std::string blockClass)
{
  if (classDescriptors->count(blockClass) < 1)
    return addBlockClass(blockClass);

  return *((*classDescriptors)[blockClass]);
}

bool BlockClasses::hasChild(std::string parent, std::string child)
{
  if (classDescriptors->count(parent) < 1)
    throw BlockNotFoundException();

  pBlockClassDescriptor bcs = (*classDescriptors)[parent];
  return bcs->hasChild(child);
}
