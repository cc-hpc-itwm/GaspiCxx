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
 * Utilities.hpp
 *
 */

#pragma once

#include <GaspiCxx/group/Rank.hpp>

namespace gaspi
{
  namespace group
  {
    std::size_t incrementIndexOnRing(std::size_t index, std::size_t ring_size);
    std::size_t incrementRankOnRing(Rank const& rank, std::size_t ring_size);
    std::size_t decrementIndexOnRing(std::size_t index, std::size_t ring_size);
    std::size_t decrementRankOnRing(Rank const& rank, std::size_t ring_size);

    class RingIndex
    {
      public:
        RingIndex(std::size_t index, std::size_t ring_size);

        RingIndex(RingIndex const&) = default;
        RingIndex& operator=(RingIndex const&) = default;
        operator int() const;

        RingIndex& operator++();
        RingIndex& operator--();
        RingIndex operator++(int);
        RingIndex operator--(int);

      private:
        std::size_t index;
        std::size_t size;
    };
  }
}
