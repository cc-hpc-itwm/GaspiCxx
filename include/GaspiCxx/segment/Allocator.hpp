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
 * Allocator.hpp
 *
 */

#ifndef SEGMENTALLOCATOR_HPP_
#define SEGMENTALLOCATOR_HPP_

#include <GaspiCxx/segment/MemoryManager.hpp>
#include <type_traits>

namespace gaspi {
namespace segment {

template <typename T = void>
class Allocator
{
  public:

    template <typename U> friend class Allocator;

    using value_type = T;
    using pointer = T *;

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    explicit
    Allocator
      ( MemoryManager * const memory )
    : _memory(memory)
    {}

    template <typename U>
    Allocator
      ( Allocator<U> const & rhs )
    : _memory(rhs._memory)
    {}

    pointer
    allocate
      ( std::size_t n )
    {
      return static_cast<pointer>(_memory->allocate(n * sizeof(T)));
    }

    void
    deallocate
      ( pointer p
      , std::size_t n)
    {
        _memory->deallocate(p, n * sizeof(T));
    }

    template <typename U>
    bool
    operator==
      (Allocator<U> const & rhs) const
    {
      return _memory == rhs._memory;
    }

    template <typename U>
    bool
    operator!=
      (Allocator<U> const & rhs) const
    {
      return _memory != rhs._memory;
    }

private:

    MemoryManager * const _memory;

};

}
}

#endif /* SEGMENTALLOCATOR_HPP_ */
