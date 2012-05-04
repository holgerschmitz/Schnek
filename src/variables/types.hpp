/*
 * types.hpp
 *
 *  Created on: 9 Dec 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef TYPES_HPP_
#define TYPES_HPP_

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




} // namespace

#endif /* TYPES_HPP_ */
