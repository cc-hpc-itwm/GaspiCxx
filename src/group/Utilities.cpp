/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
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
 * Rank.cpp
 *
 */

#include <GaspiCxx/group/Utilities.hpp>

namespace gaspi
{
  namespace group
  {
    std::size_t incrementIndexOnRing(std::size_t index, std::size_t ring_size)
    {
      return (index + 1) % ring_size;
    }

    std::size_t incrementRankOnRing(Rank const& rank, std::size_t ring_size)
    {
      return incrementIndexOnRing(rank.get(), ring_size);
    }

    std::size_t decrementIndexOnRing(std::size_t index, std::size_t ring_size)
    {
      return (index - 1 + ring_size) % ring_size;
    }

    std::size_t decrementRankOnRing(Rank const& rank, std::size_t ring_size)
    {
      return decrementIndexOnRing(rank.get(), ring_size);
    }

    RingIndex::RingIndex(std::size_t index, std::size_t ring_size)
    : index(index % ring_size),
      size(ring_size)
    { }

    RingIndex::operator int() const
    {
      return index;
    }

    RingIndex& RingIndex::operator++()
    {
      index = incrementIndexOnRing(index, size);
      return *this;
    }

    RingIndex& RingIndex::operator--()
    {
      index = decrementIndexOnRing(index, size);
      return *this;
    }

    RingIndex RingIndex::operator++(int)
    {
      RingIndex old = *this;
      operator++();
      return old;
    }

    RingIndex RingIndex::operator--(int)
    {
      RingIndex old = *this;
      operator--();
      return old;
    }
  }
}
