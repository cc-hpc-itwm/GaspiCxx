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
 * SingleSegmentPool.cpp
 *
 */

#include <GaspiCxx/segment/SingleSegmentPool.hpp>

#include <stdexcept>

namespace gaspi
{
  namespace segment
  {
    SingleSegmentPool::SingleSegmentPool(std::size_t size)
    : segment(size)
    { }

    SingleSegmentPool::SingleSegmentPool()
    : SingleSegmentPool(1024UL * 1024UL) // 1MiB
    { }

    Segment& SingleSegmentPool::getSegment(std::size_t size)
    {
      if (!segment.hasFreeMemory(size))
      {
        throw std::runtime_error(
          "SingleSegmentPool::getSegment: Not enough memory available");
      }
      return segment;
    }
  }
}
