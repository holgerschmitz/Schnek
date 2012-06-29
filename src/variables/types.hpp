/*
 * types.hpp
 *
 * Created on: 9 Dec 2011
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

#ifndef SCHNEK_TYPES_HPP_
#define SCHNEK_TYPES_HPP_

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

namespace schnek {

/// A boost::variant that can hold the basic constant values
typedef boost::variant<int, double, std::string> ValueVariant;

/// A boost::variant that can hold the basic constant values
typedef boost::variant<int*, double*, std::string*> ValuePointerVariant;

template<typename vtype> class Expression;
typedef boost::shared_ptr<Expression<int> > pIntExpression;
typedef boost::shared_ptr<Expression<double> > pFloatExpression;
typedef boost::shared_ptr<Expression<std::string> > pStringExpression;

/// A boost::variant that can hold expressions with different return type
typedef boost::variant<pIntExpression, pFloatExpression, pStringExpression> ExpressionVariant;



class VariableNotFoundException : public SchnekException
{
  public:
    VariableNotFoundException() : SchnekException() {}
};

class VariableNotInitialisedException : public SchnekException
{
  public:
    VariableNotInitialisedException() : SchnekException() {}
};

class DuplicateVariableException : public SchnekException
{
  public:
    DuplicateVariableException() : SchnekException() {}
};

class DuplicateBlockException : public SchnekException
{
  public:
    DuplicateBlockException() : SchnekException() {}
};

class TypeMismatchException : public SchnekException
{
  public:
    TypeMismatchException() : SchnekException() {}
};


} // namespace

#endif // SCHNEK_TYPES_HPP_
