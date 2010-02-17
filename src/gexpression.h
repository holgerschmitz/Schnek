#ifndef SCHNEK_GEXPRESSION_H
#define SCHNEK_GSEXPRESSION_H

namespace schnek {

/** Expression template for the Grid class.
 *  This template will be created when indexing Grid objects with the
 *  MIndex class and will allow performing expressions without temporary 
 *  variables.
 *  @todo Handle type promotion
 */
template<typename T, class Operator>
class GridExpression {
  private:
    /**The operator type. The operator will hold the information about
     * the type of operation and refernces to all the arguments
     */
    const Operator &Op;
  public:
    /**Construct the expression by passing a reference to the operator*/
    GridExpression(const Operator &Op_) : Op(Op_) {}
    
    /**Return an element of the expression*/
    T operator()() const { return Op(); }
};

/**Operator class implementing unary operators for the GridExpression.
 * Holds const reference to the expression and type information of the
 * operator
 */
template<
  typename T,
  class Exp, 
  template<class> class OperatorType
>
class GridIdentityOp {
  private:
    /// Expression
    const Exp &exp;
  public:
    /**Construct passing the two references to the expressions*/
    GridIdentityOp(const Exp &exp_) : exp(exp_) {}
    
    /**Return the result of the operator expression
     * Gets the result of exp and asks the static OperatorType::apply 
     * method to perform the calculation
     */
    T operator()() const { return OperatorType<T>::apply(this->A(), this->B()); }
};

/**Operator class implementing unary operators for the GridExpression.
 * Holds const reference to the expression and type information of the
 * operator
 */
template<
  typename T,
  class Exp, 
  template<class> class OperatorType
>
class GridUnaryOp {
  private:
    /// Expression
    const Exp &exp;
  public:
    /**Construct passing the two references to the expressions*/
    GridUnaryOp(const Exp &exp_) : exp(exp_) {}
    
    /**Return the result of the operator expression
     * Gets the result of exp and asks the static OperatorType::apply 
     * method to perform the calculation
     */
    T operator()() const { return OperatorType<T>::apply(this->A(), this->B()); }
};

/**Operator class implementing binary operators for the GridExpression.
 * Holds const references to the expressions and type information of the
 * operator
 */
template<
  typename T,
  class Exp1, 
  class Exp2, 
  template<class> class OperatorType
>
class GridBinaryOp {
  private:
    /// Expression A
    const Exp1 &A;
    /// Expression B
    const Exp2 &B;
  public:
    /** Construct passing the two references to the expressions*/
    GridBinaryOp(const Exp1 &A_, const Exp2 &B_) : A(A_), B(B_) {}
    /** Return the result of the operator expression
     *  Gets the result elements of A and B and asks the static 
     *  OperatorType::apply method to perform the calculation
     */
    T operator()() const { return OperatorType<T>::apply(A(), B()); }
};

} // namespace schnek


#endif
