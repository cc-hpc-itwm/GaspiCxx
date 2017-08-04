/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiLS.
 * 
 * GaspiLS is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 * 
 * GaspiLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * Macros.hpp
 *
 */

#ifndef MACROS_HPP_
#define MACROS_HPP_

#include <iostream>
#include <sstream>
#include <stdexcept>

extern "C" {
#include <GASPI.h>
#include <GASPI_Ext.h>
}

namespace gaspi {

namespace macro_detail {

struct CodeOriginString : public std::string
{
  CodeOriginString (const char* s, const int i) : std::string ()
  {
    std::ostringstream oss;
    oss << s << " [" << i << "]: ";

    assign (oss.str());
  }
};

}

// macro definitions

#define CODE_ORIGIN                             \
  macro_detail::CodeOriginString                \
    (__PRETTY_FUNCTION__, __LINE__)             \

#define GASPI_CHECK(X)                          \
  {                                             \
    gaspi_return_t retval((X));                 \
    if(retval != GASPI_SUCCESS)                 \
    {                                           \
      gaspi_string_t error_str                  \
        ( gaspi_error_str(retval) );            \
      std::stringstream ss;                     \
      ss << "In ";                              \
      ss << CODE_ORIGIN;                        \
      ss << "Gaspi error: ";                    \
      ss << error_str;                          \
      throw std::runtime_error(ss.str());       \
    }                                           \
  }

#define GASPI_CHECK_NOTHROW(X)                  \
  {                                             \
    gaspi_return_t retval((X));                 \
    if(retval != GASPI_SUCCESS)                 \
    {                                           \
      gaspi_string_t error_str                  \
        ( gaspi_error_str(retval) );            \
      std::stringstream ss(CODE_ORIGIN);        \
      ss << ": Gaspi error: ";                  \
      ss << error_str;                          \
      std::cout << ss.str() << std::endl;       \
    }                                           \
  }

}

#endif /* MACROS_HPP_ */
