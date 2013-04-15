/*
 * exceptions.hpp
 *
 * Created on: 20 Sep 2012
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

#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <boost/current_function.hpp>
#include <exception>

namespace schnek {

//! Base error class
class ScheckException : public std::exception {
  private:
    std::string message;
  public:
    ScheckException(const std::string& file,
                    long line,
                    const std::string& functionName,
                    const std::string& message_ = "");

    ~ScheckException() throw() {}
    //! returns the error message.
    const char* what() const throw ();
};

} // namespace

#define SCHNECK_FAIL(message) \
{ \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::ScheckException(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}


#define SCHNEK_ASSERT(condition,message) \
if (!(condition)) { \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::ScheckException(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}

#define SCHNEK_REQUIRE(condition,message) \
if (!(condition)) { \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::ScheckException(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}

#define SCHNEK_ENSURE(condition,message) \
if (!(condition)) { \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::ScheckException(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}


// duplications of the macros allowing for other exceptions with comparable constructor

#define SCHNECK_FAIL_E(message, exceptclass) \
{ \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::exceptclass(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}


#define SCHNEK_ASSERT_E(condition,message, exceptclass) \
if (!(condition)) { \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::exceptclass(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}

#define SCHNEK_REQUIRE_E(condition,message, exceptclass) \
if (!(condition)) { \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::exceptclass(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}

#define SCHNEK_ENSURE_E(condition,message, exceptclass) \
if (!(condition)) { \
    std::ostringstream _schnek_msg_stream; \
    _schnek_msg_stream << message; \
    throw schnek::exceptclass(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_schnek_msg_stream.str()); \
}

#endif // EXCEPTIONS_HPP_ 
