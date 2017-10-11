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
 * Allgather.hpp
 *
 */

#ifndef COLLECTIVES_ALLGATHER_HPP_
#define COLLECTIVES_ALLGATHER_HPP_

#include <Context.hpp>
#include <segment/Segment.hpp>

namespace gaspi {
namespace collectives {

// allgather: every process with process id iProc sends his  data of size size
//            from source buffer into the target buffer at location iProc * size
void
allgather
  ( void * const gSource
  , segment::Segment & sourceSegment
  , void * const gTarget
  , segment::Segment & targetSegment
  , std::size_t const & size
  , Context & context );

void
allgatherv
  ( void * const gSource
  , segment::Segment & sourceSegment
  , void * const gTarget
  , segment::Segment & targetSegment
  , std::size_t const * const sizes
  , Context & context );

}
}

#endif
