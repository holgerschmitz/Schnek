/*
 * singleton.hpp
 *
 * Created on: 9 May 2012
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

#ifndef SCHNEK_SINGLETON_HPP_
#define SCHNEK_SINGLETON_HPP_

namespace schnek
{

/** Creation policy for the Singleton class.
 *
 *  CreateUsingNew is used as a template parameter for the Singleton
 *  class. It determines the creation policy and creates the singleton
 *  object by calling new.
 *
 *  The type to create is given by the template parameter TYPE
 */
template<class TYPE>
class CreateUsingNew
{
  public:
    /** Create the object of type TYPE by calling the standard "new"
    *  allocation
    */
    static TYPE* create()
    {
      return new TYPE();
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

  protected:
    Singleton() {}
    /** Destructor made private and not implemented  */
    ~Singleton() {}
  private:
    /** Copy constructor made private and not implemented  */
    Singleton(const Singleton&);
    /** Copy operator made private and not implemented  */
    Singleton& operator=(const Singleton&);


    /** The pointer to the one and only instance of the singleton instance
     *  class.
     */
    static TYPE* pInstance;
};

template<
  class TYPE,
  template<class> class CreationPolicy
>
TYPE* Singleton<TYPE,CreationPolicy>::pInstance = 0;


}

#endif // SCHNEK_SINGLETON_HPP_
