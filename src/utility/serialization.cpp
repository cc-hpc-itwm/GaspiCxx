/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019
 *
 * This file is part of GaspiCxx.
 *
 * GaspiCxx is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * GaspiCxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * serialization.cpp
 *
 */

#include <GaspiCxx/utility/serialization.hpp>

#include <cstring>
#include <memory>

namespace gaspi {
namespace serialization {


template<typename T>
size_t size (T const & elem)
{
  return (sizeof (elem));
}

template<typename T>
void serialize (std::ostream & out, T const & elem)
{
  out.write (reinterpret_cast<char const *> (&elem), size (elem));


  if (not out.good())
  {
    throw std::runtime_error
      (CODE_ORIGIN + "serialization::serialize (ostream &, T const &) failed");
  }
}

template<typename T>
size_t serialize (void * const buffer, T const & elem)
{
  memcpy (buffer, &elem, size (elem));

  return serialization::size (elem);
}

template<typename T>
void deserialize (T & elem, std::istream & in)
{
  in.read (reinterpret_cast<char *> (&elem), size (elem));


  if (not in.good())
  {
    throw std::runtime_error
      (CODE_ORIGIN + "serialization::deserialize (T &, istream &) failed");
  }
}

template<typename T>
size_t deserialize (T & elem, void const * const buffer)
{
  memcpy (&elem, buffer, size (elem));

  return serialization::size (elem);
}

/* * * * * * * * * * * * * * * * * *
 * char
 * * * * * * * * * * * * * * * * * */
template size_t size (char const &);

template void serialize (std::ostream &, char const &);

template size_t serialize (void * const, char const &);

template void deserialize (char &, std::istream &);

template size_t deserialize (char &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * unsigned char
 * * * * * * * * * * * * * * * * * */
template size_t size (unsigned char const &);

template void serialize (std::ostream &, unsigned char const &);

template size_t serialize (void * const, unsigned char const &);

template void deserialize (unsigned char &, std::istream &);

template size_t deserialize (unsigned char &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * short
 * * * * * * * * * * * * * * * * * */
template size_t size (short const &);

template void serialize (std::ostream &, short const &);

template size_t serialize (void * const, short const &);

template void deserialize (short &, std::istream &);

template size_t deserialize (short &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * unsigned short
 * * * * * * * * * * * * * * * * * */
template size_t size (unsigned short const &);

template void serialize (std::ostream &, unsigned short const &);

template size_t serialize (void * const, unsigned short const &);

template void deserialize (unsigned short &, std::istream &);

template size_t deserialize (unsigned short &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * int
 * * * * * * * * * * * * * * * * * */
template size_t size (int const &);

template void serialize (std::ostream &, int const &);

template size_t serialize (void * const, int const &);

template void deserialize (int &, std::istream &);

template size_t deserialize (int &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * unsigned int
 * * * * * * * * * * * * * * * * * */
template size_t size (unsigned int const &);

template void serialize (std::ostream &, unsigned int const &);

template size_t serialize (void * const, unsigned int const &);

template void deserialize (unsigned int &, std::istream &);

template size_t deserialize (unsigned int &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * long
 * * * * * * * * * * * * * * * * * */
template size_t size (long const &);

template void serialize (std::ostream &, long const &);

template size_t serialize (void * const, long const &);

template void deserialize (long &, std::istream &);

template size_t deserialize (long &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * unsigned long
 * * * * * * * * * * * * * * * * * */
template size_t size (unsigned long const &);

template void serialize (std::ostream &, unsigned long const &);

template size_t serialize (void * const, unsigned long const &);

template void deserialize (unsigned long &, std::istream &);

template size_t deserialize (unsigned long &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * float
 * * * * * * * * * * * * * * * * * */
template size_t size (float const &);

template void serialize (std::ostream &, float const &);

template size_t serialize (void * const, float const &);

template void deserialize (float &, std::istream &);

template size_t deserialize (float &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * double
 * * * * * * * * * * * * * * * * * */
template size_t size (double const &);

template void serialize (std::ostream &, double const &);

template size_t serialize (void * const, double const &);

template void deserialize (double &, std::istream &);

template size_t deserialize (double &, void const * const);


/* * * * * * * * * * * * * * * * * *
 * std::string
 * * * * * * * * * * * * * * * * * */
template<>
size_t size (std::string const & str)
{
  return (sizeof(size_t) + str.length());
}

template<>
void serialize (std::ostream & out, std::string const & str)
{
  size_t const length (str.length());
  out.write (reinterpret_cast<char const *> (&length), sizeof (length));
  out.write (str.data(), length);

  if (not out.good())
  {
    throw std::runtime_error
      (CODE_ORIGIN + "serialization::serialize (ostream &, string const &) failed");
  }
}

template<>
size_t serialize (void * const buffer, std::string const & str)
{
  size_t const length (str.length());
  char * cPtr (static_cast<char *> (buffer) + serialize (buffer, length));
  str.copy (cPtr, length);

  return serialization::size (str);
}

template<>
void deserialize (std::string & str, std::istream & in)
{
  size_t length;
  deserialize (length, in);

  std::unique_ptr<char[]> ch(new char[length]);
  in.read (ch.get(), length);
  str.assign (ch.get(), length);


  if (not in.good())
  {
    throw std::runtime_error
      (CODE_ORIGIN + "serialization::deserialize (string &, istream &) failed");
  }
}

template<>
size_t deserialize (std::string & str, void const * const buffer)
{
  size_t length;
  char const * cPtr (static_cast<char const *> (buffer) + deserialize (length, buffer));
  str.assign (cPtr, length);

  return serialization::size (str);
}

}
}
