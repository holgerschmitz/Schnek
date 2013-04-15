/*
 * exceptions.cpp
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

#include <sstream>
#include "exceptions.hpp"

using namespace schnek;

ScheckException::ScheckException(const std::string& file, long line,
             const std::string& function,
             const std::string& message_) {
  std::ostringstream msg_stream;
  msg_stream << function << std::endl
      << file << "(" << line << "): " << message_ << std::endl;
  message = msg_stream.str();
}

const char* ScheckException::what() const throw () {
    return message.c_str();
}
