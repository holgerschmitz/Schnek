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

#include <cmath>

namespace schnek {
namespace expression {

  template<class vtype>
  struct OperatorNeg
  {
      static vtype eval(vtype val);
  };

  template<class vtype>
  struct OperatorAdd
  {
      static vtype eval(vtype val1, vtype val2);
  };

  template<class vtype>
  struct OperatorSubtract
  {
      static vtype eval(vtype val1, vtype val2);
  };

  template<class vtype>
  struct OperatorMultiply
  {
      static vtype eval(vtype val1, vtype val2);
  };

  template<class vtype>
  struct OperatorDivide
  {
      static vtype eval(vtype val1, vtype val2);
  };

  template<class vtype>
  struct OperatorExponent
  {
      static vtype eval(vtype val1, vtype val2);
  };

  template<class vtype>
  vtype OperatorNeg<vtype>::eval(vtype val) { return -val; }

  template<class vtype>
  vtype OperatorAdd<vtype>::eval(vtype val1, vtype val2) { return val1 + val2; }

  template<class vtype>
  vtype OperatorSubtract<vtype>::eval(vtype val1, vtype val2) { return val1 - val2; }

  template<class vtype>
  vtype OperatorMultiply<vtype>::eval(vtype val1, vtype val2) { return val1 * val2; }

  template<class vtype>
  vtype OperatorDivide<vtype>::eval(vtype val1, vtype val2) { return val1 / val2; }

  template<class vtype>
  vtype OperatorExponent<vtype>::eval(vtype val1, vtype val2) { return pow(val1,val2); }


  template<>
  inline std::string OperatorNeg<std::string>::eval(std::string val) { return ""; }

  template<>
  inline std::string OperatorSubtract<std::string>::eval(std::string val1, std::string val2) { return ""; }

  template<>
  inline std::string OperatorMultiply<std::string>::eval(std::string val1, std::string val2) { return ""; }

  template<>
  inline std::string OperatorDivide<std::string>::eval(std::string val1, std::string val2) { return ""; }

  template<>
  inline std::string OperatorExponent<std::string>::eval(std::string val1, std::string val2) { return ""; }

} // namespace expression
} // namespace schnek

#endif // SCHNEK_OPERATORS_HPP_
