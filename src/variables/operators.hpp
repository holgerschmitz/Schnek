/*
 * operators.hpp
 *
 * Created on: 27 Jan 2011
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

#ifndef SCHNEK_OPERATORS_HPP_
#define SCHNEK_OPERATORS_HPP_

#include "expression.hpp"
#include "../util/logger.hpp"

#include <memory>
#include <cmath>

#undef LOGLEVEL
#define LOGLEVEL 0

namespace schnek {
namespace expression {

  template<class vtype>
  struct OperatorId
  {
      static vtype eval(vtype val);
  };

  template<class vtype>
  struct OperatorNeg
  {
      static vtype eval(vtype val);
  };

  template<class vtype>
  struct OperatorInv
  {
      static vtype eval(vtype val);
  };

  template<class vtype>
  struct OperatorSubtract;

  template<class vtype>
  struct OperatorAdd
  {
      enum { isPositive = true };
      typedef OperatorAdd<vtype> Positive;
      typedef OperatorSubtract<vtype> Negative;
      typedef OperatorSubtract<vtype> Inverted;

      static vtype eval(vtype val1, vtype val2);
      static typename Expression<vtype>::pExpression negate(typename Expression<vtype>::pExpression val);
  };

  template<class vtype>
  struct OperatorSubtract
  {
      enum { isPositive = false };
      typedef OperatorAdd<vtype> Positive;
      typedef OperatorSubtract<vtype> Negative;
      typedef OperatorAdd<vtype> Inverted;

      static vtype eval(vtype val1, vtype val2);
      static typename Expression<vtype>::pExpression negate(typename Expression<vtype>::pExpression val);
  };

  template<class vtype>
  struct OperatorDivide;

  template<class vtype>
  struct OperatorMultiply
  {
      enum { isPositive = true };
      typedef OperatorMultiply<vtype> Positive;
      typedef OperatorDivide<vtype> Negative;
      typedef OperatorDivide<vtype> Inverted;

      static vtype eval(vtype val1, vtype val2);
      static typename Expression<vtype>::pExpression negate(typename Expression<vtype>::pExpression val);
  };

  template<class vtype>
  struct OperatorDivide
  {
      enum { isPositive = false };
      typedef OperatorMultiply<vtype> Positive;
      typedef OperatorDivide<vtype> Negative;
      typedef OperatorMultiply<vtype> Inverted;

      static vtype eval(vtype val1, vtype val2);
      static typename Expression<vtype>::pExpression negate(typename Expression<vtype>::pExpression val);
  };

  template<class vtype>
  struct OperatorExponent
  {
      enum { isPositive = true };
      typedef OperatorExponent<vtype> Positive;
      typedef OperatorExponent<vtype> Negative;
      typedef OperatorExponent<vtype> Inverted;

      static vtype eval(vtype val1, vtype val2);
      static typename Expression<vtype>::pExpression negate(typename Expression<vtype>::pExpression val);
  };

  template<class vtype>
  vtype OperatorId<vtype>::eval(vtype val)
  {
      SCHNEK_TRACE_LOG(5, "OperatorId<vtype>::eval(" << val << ")")
      return val;
  }

  template<class vtype>
  vtype OperatorNeg<vtype>::eval(vtype val)
  {
      SCHNEK_TRACE_LOG(5, "OperatorNeg<vtype>::eval(" << val << ")")
      return -val;
  }

  template<class vtype>
  vtype OperatorInv<vtype>::eval(vtype val)
  {
      SCHNEK_TRACE_LOG(5, "OperatorInv<vtype>::eval(" << val << ")")
      return 1/val;
  }

  template<class vtype>
  vtype OperatorAdd<vtype>::eval(vtype val1, vtype val2)
  {
      SCHNEK_TRACE_LOG(5, "OperatorAdd<vtype>::eval(" << val1 << ", " << val2 << ")")
      return val1 + val2;
  }

  template<class vtype>
  vtype OperatorSubtract<vtype>::eval(vtype val1, vtype val2)
  {
      SCHNEK_TRACE_LOG(5, "OperatorSubtract<vtype>::eval(" << val1 << ", " << val2 << ")")
      return val1 - val2;
  }

  template<class vtype>
  vtype OperatorMultiply<vtype>::eval(vtype val1, vtype val2)
  {
      SCHNEK_TRACE_LOG(5, "OperatorMultiply<vtype>::eval(" << val1 << ", " << val2 << ")")
      return val1 * val2;
  }

  template<class vtype>
  vtype OperatorDivide<vtype>::eval(vtype val1, vtype val2)
  {
      SCHNEK_TRACE_LOG(5, "OperatorDivide<vtype>::eval(" << val1 << ", " << val2 << ")")
      return val1 / val2;
  }

  template<class vtype>
  vtype OperatorExponent<vtype>::eval(vtype val1, vtype val2)
  {
      SCHNEK_TRACE_LOG(5, "OperatorExponent<vtype>::eval(" << val1 << ", " << val2 << ")")
      return pow(val1,val2);
  }


  template<class vtype>
  typename Expression<vtype>::pExpression OperatorAdd<vtype>::negate(typename Expression<vtype>::pExpression val)
  {
      return std::make_shared<UnaryOp<OperatorNeg<vtype>, vtype> >(val);
  }

  template<class vtype>
  typename Expression<vtype>::pExpression OperatorSubtract<vtype>::negate(typename Expression<vtype>::pExpression val)
  {
      return std::make_shared<UnaryOp<OperatorNeg<vtype>, vtype> >(val);
  }

  template<class vtype>
  typename Expression<vtype>::pExpression OperatorMultiply<vtype>::negate(typename Expression<vtype>::pExpression val)
  {
      return std::make_shared<UnaryOp<OperatorInv<vtype>, vtype> >(val);
  }

  template<class vtype>
  typename Expression<vtype>::pExpression OperatorDivide<vtype>::negate(typename Expression<vtype>::pExpression val)
  {
      return std::make_shared<UnaryOp<OperatorInv<vtype>, vtype> >(val);
  }

  template<class vtype>
  typename Expression<vtype>::pExpression OperatorExponent<vtype>::negate(typename Expression<vtype>::pExpression val)
  {
      return val;
  }


  template<>
  inline std::string OperatorNeg<std::string>::eval(std::string) { return ""; }

  template<>
  inline std::string OperatorInv<std::string>::eval(std::string) { return ""; }

  template<>
  inline std::string OperatorSubtract<std::string>::eval(std::string, std::string) { return ""; }

  template<>
  inline std::string OperatorMultiply<std::string>::eval(std::string, std::string) { return ""; }

  template<>
  inline std::string OperatorDivide<std::string>::eval(std::string, std::string) { return ""; }

  template<>
  inline std::string OperatorExponent<std::string>::eval(std::string, std::string) { return ""; }

} // namespace expression
} // namespace schnek

#undef LOGLEVEL
#define LOGLEVEL 0

#endif // SCHNEK_OPERATORS_HPP_
