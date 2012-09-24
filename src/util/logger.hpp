/*
 * logger.hpp
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

#ifndef SCHNEK_LOGGER_HPP_
#define SCHNEK_LOGGER_HPP_


#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/comparison/greater_equal.hpp>

#include <iostream>
#include <string>

namespace schnek {

/** Macro for writing to the normal log.
 *
 *  The first argument is the log level of the message, the second argument
 *  is the string passed to the LoggerInstance::out stream.
 *
 *  When the current log level is higher or equal than the log level of the
 *  message, the output will be generated, otherwise the macro extends to an
 *  empty code string and no code whatsoever will be generated.
 *
 *  The message argument can consist of multiple strings or values
 *  concatenated with <<. It should end in a newline.
 */
#define SCHNEK_TRACE_LOG(i,x)                                  \
  BOOST_PP_IIF( BOOST_PP_GREATER_EQUAL( LOGLEVEL, i ), \
    schnek::Logger::instance().out() << __LINE__ << " " << __FILE__ << ": "<< x << "\n";, \
    BOOST_PP_EMPTY()                                   \
  )

/** Macro for writing to the error log.
 *
 *  The first argument is the log level of the message, the second argument
 *  is the string passed to the LoggerInstance::err stream.
 *
 *  When the current log level is higher or equal than the log level of the
 *  message, the output will be generated, otherwise the macro extends to an
 *  empty code string and no code whatsoever will be generated.
 *
 *  The message argument can consist of multiple strings or values
 *  concatenated with <<. It should end in a newline.
 */
#define SCHNEK_TRACE_ERR(i,x)                                  \
  BOOST_PP_IIF( BOOST_PP_GREATER_EQUAL( LOGLEVEL, i ), \
      schnek:::Logger::instance().err() << __LINE__ << " " << __FILE__ << ": " << x << "\n";, \
    BOOST_PP_EMPTY()                                   \
  )

#define SCHNEK_TRACE_ENTER_FUNCTION(i)                                  \
  BOOST_PP_IIF( BOOST_PP_GREATER_EQUAL( LOGLEVEL, i ), \
    schnek::Logger::instance().out() << "Entering " << BOOST_CURRENT_FUNCTION << std::endl;, \
    BOOST_PP_EMPTY()                                   \
  )

#define SCHNEK_TRACE_EXIT_FUNCTION(i)                                  \
  BOOST_PP_IIF( BOOST_PP_GREATER_EQUAL( LOGLEVEL, i ), \
    schnek::Logger::instance().out() << "Leaving " << BOOST_CURRENT_FUNCTION << std::endl;, \
    BOOST_PP_EMPTY()                                   \
  )

/** Predefines the log level.
 *
 * To redefine to eg level 2 use the following commands:
 *
 * #undef LOGLEVEL
 * #define LOGLEVEL 2
 */
#define LOGLEVEL 2

#include "../singleton.h"

/** Instance of the logger singleton used for logging debug and error
 *  messages.
 *
 *  The logger provides a std::ostream for error messages and for other
 *  messages. These are provided by the methods out and err. Currently they
 *  are implemented to return the std::cout and std::cerr streams.
 */
class Logger : public Singleton<Globals>
{
  public:
    /** Return the ostream for writing standard debug comments.
     *
     *  Currently implemented to return std::cout
     */
    std::ostream &out() { return std::cout; }

    /** Return the ostream for writing error messages.
     *
     *  Currently implemented to return std::cerr
     */
    std::ostream &err() { return std::cerr; }
  private:
    friend class Singleton<Logger>;
    friend class CreateUsingNew<Logger>;

    /** The private default constructor can only be called by the
     *  singleton template.
     */
    LoggerInstance() { }

    /** The private destructor can only be called by the
     *  singleton template.
     */
    ~LoggerInstance() {
//      std::cerr << "DELETING LOGGER "<< this <<" " <<i<<"\n";
    }
};

/** @file logger.hpp
 *
 *  Defines the Logger singleton and macros for using it
 *
 *  Example:
 *
 * #undef LOGLEVEL
 * #define LOGLEVEL 2
 *
 * WRITELOG(1,"Level 1")
 *
 * WRITELOG(2,"Level 2")
 *
 * WRITELOG(3,"Level 3")
 *
 * WRITELOG(4,"Level 4")
 *
 * WRITELOG(5,"Level 5")
 *
 *  This will only produce code for the level 1 and 2 statements. All other
 *  macros expand to nothing.
 */

} // namespace davtools

#include "logger.t"

#endif


} // namespace 
#endif // LOGGER_HPP_ 
