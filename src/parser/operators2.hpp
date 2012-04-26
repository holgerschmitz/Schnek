/*
 * operators.hpp
 *
 *  Created on: 9 Dec 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef OPERATORS_HPP_
#define OPERATORS_HPP_

namespace schnek {

}

class Operator
{
  public:
    virtual ExpressionVariant evaluate() = 0;
};



} // namespace

#endif /* OPERATORS_HPP_ */
