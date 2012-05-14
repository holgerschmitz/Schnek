/*
 * singleton.hpp
 *
 *  Created on: 9 May 2012
 *      Author: Holger Schmitz
 *       Email: h.schmitz@imperial.ac.uk
 */

#ifndef SINGLETON_HPP_
#define SINGLETON_HPP_

/** Creation policy for the Singleton class.
 *
 *  CreateUsingNew is used as a template parameter for the Singleton
 *  class. It determines the creation policy and creates the singleton
 *  object by calling new.
 *
 *  The type to create is given by the template parameter TYPE
 */
template<class TYPE>
struct CreateUsingNew
{
  /** Create the object of type TYPE by calling the standard "new"
   *  allocation
   */
  static TYPE* create()
  {
    return new TYPE;
  }
};


/** A singleton template class
 *
 *  The singleton has an instance type specified by TYPE and a creation
 *  policy specified by CreationPolicy<TYPE>.
 *
 *  The singleton instance should make the constructors, destructors
 *  and copy operators private, so no two instances can be generated
 *  at any time. The CreationPolicy<TYPE> should be made the only friend to
 *  the instance class. Only the default constructor and the destructor then
 *  normally need to be implemented. (This of course can depend on the
 *  CreationPolicy)
 *
 *  The CreationPolicy should define the method
 *  "static TYPE* create()" which returns a new instance of the singleton
 *  instance class.
 *
 *  Access to the singleton instance is through the static instance() method.
 *
 *  The Singleton class itself can not be instantiated because all
 *  constructors are private.
 */
template<
  class TYPE,
  template<class> class CreationPolicy = CreateUsingNew
>
class Singleton
{
  public:

    /** Access to the singleton instance.
     *
     *  If no instance exists the CreationPolicy<TYPE>::create() method
     *  is called to create one.
     */
    static TYPE& instance()
    {
      if (!pInstance)
      {
        pInstance = CreationPolicy<TYPE>::create();
      }
      return *pInstance;
    }

  private:
    /** Default constructor made private and not implemented  */
    Singleton();
    /** Copy constructor made private and not implemented  */
    Singleton(const Singleton&);
    /** Copy operator made private and not implemented  */
    Singleton& operator=(const Singleton&);
    /** Destructor made private and not implemented  */
    ~Singleton() {}

    /** The pointer to the one and only instance of the singleton instance
     *  class.
     */
    static TYPE* pInstance;
};

template<
  class TYPE,
  template<class> class CreationPolicy
>
TYPE* Singleton<TYPE,CreationPolicy,LifeTimePolicy>::pInstance = 0;



#endif /* SINGLETON_HPP_ */