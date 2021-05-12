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
 * SegmentPool.hpp
 *
 */

#pragma once

#include <GaspiCxx/segment/Segment.hpp>

namespace gaspi
{
  namespace segment
  {
    // A `SegmentPool` provides an abstraction
    // of the GPI segments, in order to
    // simplify the creation of `Buffer`s
    // used for communication.
    // 
    // It manages one or several segments,
    // and returns references to one of them
    // that currently has enough free memory.
    class SegmentPool
    {
      public:
        // Return a reference to a `Segment` that
        // has at least `size` bytes of memory available
        virtual Segment& getSegment(std::size_t size) = 0;

        virtual ~SegmentPool() = default;
    };
  }
}
