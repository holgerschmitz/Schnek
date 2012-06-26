/*
 * block.cpp
 *
 *  Created on: 2 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
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
