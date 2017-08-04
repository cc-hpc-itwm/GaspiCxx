/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiCXX.
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
 * along with GaspiCXX. If not, see <http://www.gnu.org/licenses/>.
 *
 * ScopedAllocation.hpp
 *
 */

#ifndef SCOPED_ALLOCATION_HPP_
#define SCOPED_ALLOCATION_HPP_

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <segment/Segment.hpp>

namespace gaspi {

//! This class implements the RAII idiom for memory
//! allocations on GASPI segments.
//! \brief Managed memory allocation on a GASPI segment
template < class T
         , template <typename> class Allocator = segment::Allocator >
class ScopedAllocation
{
  private:

    Allocator<T>   _allocator;
    T * const      _g_pointer;
    std::size_t    _count;

public:
  //! Allocate <c>count*sizeof(T)</c> bytes on the given interface.
  //! \brief Constructor
  ScopedAllocation
    ( Allocator<T> const & allocator
    , std::size_t count)
  : _allocator(allocator)
  , _g_pointer
      (_allocator.allocate(count))
  , _count(count)
  { }

  ~ScopedAllocation
    ()
  {
    _allocator.deallocate(_g_pointer,_count);
  }
  //! \brief   Get a pointer to the allocated memory.
  //! \warning Do not take ownership of the pointer and make sure
  //!          it is not used outside of the current scope!
  T *
  pointer() const
    { return _g_pointer; }

  //! Access the allocated memory.
  T&
  operator[]
    (int index) const
  { return _g_pointer[index]; }

};

}

#endif /* SCOPED_ALLOCATION_HPP_ */
