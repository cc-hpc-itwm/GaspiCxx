/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
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
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * serialization.hpp
 *
 */

#include <utility/Macros.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <stdexcept>

#ifndef SERIALIZATION_HPP_
#define SERIALIZATION_HPP_

namespace gaspi {
/** \brief defines interface for serialization / deserialization and
 *         defines the routines for basic types
 */
namespace serialization {

// common declaration

/** \brief size of the data type after serialization
 */
template<typename T>
size_t size (T const & elem);

/**  \brief serializes data of type T in elem to ostream
 *
 *   \param[out] os contains the serialized data after conversion
 *   \param[in] elem data to be serialized
 *
 *   \throws std::runtime::error on stream operation failure
 */
template<typename T>
void serialize (std::ostream & os, T const & elem);

/**  \brief serializes data of type T in elem to buffer
 *
 *   \param[out] buffer contains the serialized data after conversion
 *   \param[in] elem data to be serialized
 *
 *   \return size of the serialized data, which has been converted
 *
 *   \note insufficient buffer size causes undefined writes
 */
template<typename T>
size_t serialize (void * const buffer, T const & elem);

/**  \brief converts the serialized data from istream to elem
 *
 *   \param[out] elem  data type, the serialized data is converted to
 *   \param[in] is serialized data
 *
 *   \throws std::runtime::error on stream operation failure
 */
template<typename T>
void deserialize (T & elem, std::istream & is);

/**  \brief converts the serialized data from buffer to elem
 *
 *   \param[out] elem  data type, the serialized data is converted to
 *   \param[in] buffer serialized data
 *
 *   \return size of the serialized data, which has been converted
 */
template<typename T>
size_t deserialize (T & elem, void const * const buffer);




// -------------------------------
// Implementation of partial specialization
// -------------------------------


// std::pair
template<typename U, typename V>
size_t size (std::pair<U, V> const & data)
{
  return ( sizeof (data.first)
         + sizeof (data.second)
         );
}

template<typename U, typename V>
void serialize (std::ostream & out, std::pair<U, V> const & data)
{
  serialize (out, data.first);
  serialize (out, data.second);

  if (not out.good())
  {
    throw std::runtime_error
      ("serialization::serialize (ostream &, pair<U,V> const &) failed");
  }
}

template<typename U, typename V>
size_t serialize (void * const buffer, std::pair<U, V> const & data)
{
  char * cPtr (static_cast<char *> (buffer));
  cPtr += serialize (cPtr, data.first);
  cPtr += serialize (cPtr, data.second);

  return (cPtr - static_cast<char const *> (buffer));
}

template<typename U, typename V>
void deserialize (std::pair<U, V> & data, std::istream & in)
{
  deserialize (data.first, in);
  deserialize (data.second, in);

  if (not in.good())
  {
    throw std::runtime_error
      ("serialization::deserialize (pair<U,V> &, istream &) failed");
  }
}

template<typename U, typename V>
size_t deserialize (std::pair<U, V> & data, void const * const buffer)
{
  char const * cPtr (static_cast<char const *> (buffer));
  cPtr += deserialize (data.first, cPtr);
  cPtr += deserialize (data.second, cPtr);

  return (cPtr - static_cast<char const *> (buffer));
}


// container

// std::vector
template<typename T>
size_t size (std::vector<T> const & vec)
{
  size_t totSize (sizeof (size_t));

  typedef typename std::vector<T>::const_iterator iterator;
  for (iterator it (vec.begin()); it != vec.end(); ++it)
  {
    totSize += size (*it);
  }

  return totSize;
}

template<typename T>
void serialize (std::ostream & out, std::vector<T> const & vec)
{
  size_t const length (vec.size());
  serialize (out, length);

  typedef typename std::vector<T>::const_iterator iterator;
  for (iterator it (vec.begin()); it != vec.end(); ++it)
  {
    serialize (out, *it);
  }


  if (not out.good())
  {
    throw std::runtime_error
      ("serialization::serialize (ostream &, vector<T> const &) failed");
  }
}

template<typename T>
size_t serialize (void * const buffer, std::vector<T> const & vec)
{
  size_t const length (vec.size());
  char * cPtr (static_cast<char *> (buffer) + serialize (buffer, length));

  typedef typename std::vector<T>::const_iterator iterator;
  for (iterator it (vec.begin()); it != vec.end(); ++it)
  {
    cPtr += serialize (cPtr, *it);
  }

  return size (vec);
}

template<typename T>
void deserialize (std::vector<T> & vec, std::istream & in)
{
  size_t length;
  deserialize (length, in);

  vec.resize (length);
  for (size_t n (0); n < length; ++n)
  {
    deserialize (vec[n], in);
  }


  if (not in.good())
  {
    throw std::runtime_error
      ("serialization::deserialize (vector<T> &, istream &) failed");
  }
}

template<typename T>
size_t deserialize (std::vector<T> & vec, void const * const buffer)
{
  size_t length;
  char const * cPtr (static_cast<char const *> (buffer) + deserialize (length, buffer));

  vec.resize (length);
  for (size_t n (0); n < length; ++n)
  {
    cPtr += deserialize (vec[n], cPtr);
  }

  return size (vec);
}


// std::deque
template<typename T>
size_t size (std::deque<T> const & vec)
{
  size_t totSize (sizeof (size_t));

  typedef typename std::deque<T>::const_iterator iterator;
  for (iterator it (vec.begin()); it != vec.end(); ++it)
  {
    totSize += size (*it);
  }

  return totSize;
}

template<typename T>
void serialize (std::ostream & out, std::deque<T> const & vec)
{
  size_t const length (vec.size());
  serialize (out, length);

  typedef typename std::deque<T>::const_iterator iterator;
  for (iterator it (vec.begin()); it != vec.end(); ++it)
  {
    serialize (out, *it);
  }


  if (not out.good())
  {
    throw std::runtime_error
      ("serialization::serialize (ostream &, vector<T> const &) failed");
  }
}

template<typename T>
size_t serialize (void * const buffer, std::deque<T> const & vec)
{
  size_t const length (vec.size());
  char * cPtr (static_cast<char *> (buffer) + serialize (buffer, length));

  typedef typename std::deque<T>::const_iterator iterator;
  for (iterator it (vec.begin()); it != vec.end(); ++it)
  {
    cPtr += serialize (cPtr, *it);
  }

  return size (vec);
}

template<typename T>
void deserialize (std::deque<T> & vec, std::istream & in)
{
  size_t length;
  deserialize (length, in);

  vec.resize (length);
  for (size_t n (0); n < length; ++n)
  {
    deserialize (vec[n], in);
  }


  if (not in.good())
  {
    throw std::runtime_error
      ("serialization::deserialize (vector<T> &, istream &) failed");
  }
}

template<typename T>
size_t deserialize (std::deque<T> & vec, void const * const buffer)
{
  size_t length;
  char const * cPtr (static_cast<char const *> (buffer) + deserialize (length, buffer));

  vec.resize (length);
  for (size_t n (0); n < length; ++n)
  {
    cPtr += deserialize (vec[n], cPtr);
  }

  return size (vec);
}


// std::set
template<typename T>
size_t size (std::set<T> const & set)
{
  size_t totSize (sizeof (size_t));

  typedef typename std::set<T>::const_iterator iterator;
  for (iterator it (set.begin()); it != set.end(); ++it)
  {
    totSize += size (*it);
  }

  return totSize;
}

template<typename T>
void serialize (std::ostream & out, std::set<T> const & set)
{
  size_t const length (set.size());
  out.write (reinterpret_cast<char const *> (&length), sizeof (length));

  typedef typename std::set<T>::const_iterator iterator;
  for (iterator it (set.begin()); it != set.end(); ++it)
  {
    serialize (out, *it);
  }

  if (not out.good())
  {
    throw std::runtime_error
      ("serialization::serialize (ostream &, set<T> const &) failed");
  }
}

template<typename T>
size_t serialize (void * const buffer, std::set<T> const & set)
{
  size_t const length (set.size());
  char * cPtr (static_cast<char *> (buffer) + serialize (buffer, length));

  typedef typename std::set<T>::const_iterator iterator;
  for (iterator it (set.begin()); it != set.end(); ++it)
  {
    cPtr += serialize (cPtr, *it);
  }

  return size (set);
}

template<typename T>
void deserialize (std::set<T> & set, std::istream & in)
{
  size_t length;
  deserialize (length, in);

  set.clear();
  for (size_t n (0); n < length; ++n)
  {
    T elem;
    deserialize (elem, in);
    set.insert (elem);
  }


  if (not in.good())
  {
    throw std::runtime_error
      ("serialization::deserialize (set<T> &, istream &) failed");
  }
}

template<typename T>
size_t deserialize (std::set<T> & set, void const * const buffer)
{
  size_t length;
  char const * cPtr (static_cast<char const *> (buffer) + deserialize (length, buffer));

  set.clear();
  for (size_t n (0); n < length; ++n)
  {
    T elem;
    cPtr += deserialize (elem, cPtr);
    set.insert  (elem);
  }

  return size (set);
}


// std::map
template<typename U, typename V, typename T>
size_t size (std::map<U, V, T> const & map)
{
  size_t totSize (sizeof (size_t));

  typedef typename std::map<U, V, T>::const_iterator iterator;
  for (iterator it (map.begin()); it != map.end(); ++it)
  {
    totSize += ( size (it->first)
               + size (it->second)
               );
  }

  return totSize;
}

template<typename U, typename V, typename T>
void serialize (std::ostream & out, std::map<U, V, T> const & map)
{
  size_t const length (map.size());
  out.write (reinterpret_cast<char const *> (&length), sizeof (length));

  typedef typename std::map<U, V, T>::const_iterator iterator;
  for (iterator it (map.begin()); it != map.end(); ++it)
  {
    serialize (out, it->first);
    serialize (out, it->second);
  }

  if (not out.good())
  {
    throw std::runtime_error
      ("serialization::serialize (ostream &, map<U, V> const &) failed");
  }
}

template<typename U, typename V, typename T>
size_t serialize (void * const buffer, std::map<U, V, T> const & map)
{
  size_t const length (map.size());
  char * cPtr (static_cast<char *> (buffer) + serialize (buffer, length));

  typedef typename std::map<U, V, T>::const_iterator iterator;
  for (iterator it (map.begin()); it != map.end(); ++it)
  {
    cPtr += serialize (cPtr, it->first);
    cPtr += serialize (cPtr, it->second);
  }

  return (cPtr - static_cast<char const *> (buffer));
}

template<typename U, typename V, typename T>
void deserialize (std::map<U, V, T> & map, std::istream & in)
{
  size_t length;
  deserialize (length, in);

  map.clear();
  for (size_t n (0); n < length; ++n)
  {
    U key;
    deserialize (key, in);

    V elem;
    deserialize (elem, in);

    map.insert (std::pair<U, V> (key, elem));
  }


  if (not in.good())
  {
    throw std::runtime_error
      ("serialization::deserialize (map<U, V> &, istream &) failed");
  }
}

template<typename U, typename V, typename T>
size_t deserialize (std::map<U, V, T> & map, void const * const buffer)
{
  size_t length;
  char const * cPtr (static_cast<char const *> (buffer) + deserialize (length, buffer));

  map.clear();
  for (size_t n (0); n < length; ++n)
  {
    U key;
    cPtr += deserialize (key, cPtr);

    V elem;
    cPtr += deserialize (elem, cPtr);

    map.insert (std::pair<U, V> (key, elem));
  }

  return (cPtr - static_cast<char const *> (buffer));
}

} // namespace serialization
} // namespace gaspi

#endif /* SERIALIZATION_HPP_ */
