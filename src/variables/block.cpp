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
#include "blockdata.hpp"
#include <boost/foreach.hpp>

using namespace schnek;


BlockTree::BlockTree() : root(), cursor(), depth() {}

void BlockTree::addChild(pBlock child)
{
  //std::cout << "BlockTree::addChild(" << child->getId() << ")\n";
  //if (cursor) std::cout << "  - have cursor " << cursor->getId() << ")\n";

  if (cursor) {
    cursor->addChild(child);
    child->setParent(cursor);
  }
  cursor = child;
  if (!root) root = child;
  depth.push(0);
}

void BlockTree::moveDown()
{
  //std::cout << "BlockTree::moveDown()\n";
  if (depth.size() >0) ++(depth.top());
}

void BlockTree::moveUp()
{
  //std::cout << "BlockTree::moveUp()\n";
  // if (cursor) std::cout << "  - have cursor " << cursor->getId() << ")\n";

  if (depth.size() == 0) return;
  if (0 == depth.top())
  {
    //std::cout << "  to parent\n";
    cursor = cursor->getParent();
    //if (cursor) std::cout << "  - have parent cursor " << cursor->getId() << ")\n";
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
  //std::cout << "Block::setup() " << getId() << "  " << this << std::endl;
  this->initParameters(blockParameters);
  BOOST_FOREACH(pBlock child, children)
  {
    child->setup();
  }
}

void Block::addChild(pBlock child)
{
  //std::cout << "Block("<< getId() <<")::addChild("<< child->getId() <<")\n";
  children.push_back(child);
}


void Block::registerHierarchy()
{
  //std::cout << "Block::registerHierarchy() " << getId() << "  " << this << std::endl;
  this->registerData();
  BOOST_FOREACH(pBlock child, children)
  {
    child->registerHierarchy();
  }
}

void Block::preInitHierarchy()
{
  this->preInit();
  BOOST_FOREACH(pBlock child, children)
  {
    child->preInitHierarchy();
  }
}

void Block::initHierarchy()
{
  this->init();
  BOOST_FOREACH(pBlock child, children)
  {
    child->initHierarchy();
  }
}

void Block::postInitHierarchy()
{
  this->postInit();
  BOOST_FOREACH(pBlock child, children)
  {
    child->postInitHierarchy();
  }
}

void Block::initAll()
{
  Block *b = this;
  while (b->parent) b = &(*parent);

  b->evaluateParameters();
  b->registerHierarchy();
  b->preInitHierarchy();
  b->initHierarchy();
  b->postInitHierarchy();
}

