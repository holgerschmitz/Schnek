/*
 * operators.hpp
 *
 *  Created on: 27 Jan 2011
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
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

#endif /* SCHNEK_OPERATORS_HPP_ */
