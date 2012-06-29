/*
 * block.cpp
 *
 * Created on: 2 May 2012
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

#include "block.hpp"
#include <boost/foreach.hpp>

using namespace schnek;


BlockTree::BlockTree() : root(), cursor(), depth() {}

void BlockTree::addChild(pBlock child)
{
  if (cursor) cursor->addChild(child);
  cursor = child;
  if (!root) root = child;
  depth.push(0);
}

void BlockTree::moveDown()
{
  if (depth.size() >0) ++(depth.top());
}

void BlockTree::moveUp()
{
  if (depth.size() == 0) return;
  if (0 == depth.top())
  {
    cursor = cursor->getParent();
    depth.pop();
  }
  else
  {
    --(depth.top());
  }
}


void Block::evaluateParameters()
{
  blockParameters.evaluate();
  BOOST_FOREACH(pBlock child, children)
  {
    child->evaluateParameters();
  }
}

void Block::setup()
{
  this->initParameters(blockParameters);
  BOOST_FOREACH(pBlock child, children)
  {
    child->setup();
  }
}

void Block::addChild(pBlock child)
{
  children.push_back(child);
}


template<typename T>
bool Block::getData(std::string key, const T &data, bool upward)
{
  if (BlockData::instance().exists(this->getId(), key))
  {
    data = BlockData::instance().get(this->getId(), key);
    return true;
  }

  if (upward && parent)
    return parent->getData(key, data, true);

  count = 0;
  BOOST_FOREACH(pBlock child, children)
  {
    if (child->getData(key,data, false)) ++count;
  }

  if (count>1) throw DuplicateVariableException();
  return (count!=0);
}

template<typename T>
void Block::addData(std::string key, const T &data)
{
  BlockData::instance().add(this->getId(), key, data);
}

template<typename T>
void Block::retrieveData(std::string key, T &data)
{
  if (!getData(key, data, true)) throw VariableNotFoundException();
}


void Block::initHierarchy()
{
  BOOST_FOREACH(pBlock child, children)
  {
    child->registerData();
  }

  BOOST_FOREACH(pBlock child, children)
  {
    child->init();
  }
}

void Block::initAll()
{
  pBlock b = this;
  while (b->parent) b = parent;

  b->initHierarchy();
}
