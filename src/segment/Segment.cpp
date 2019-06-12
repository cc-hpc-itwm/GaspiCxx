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
 * Segment.cpp
 *
 */

#include <stdexcept>
#include <sstream>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {
namespace segment {

Segment
  ::Segment
    ( std::size_t segmentSize )
: SegmentResource( segmentSize )
, SegmentManager( SegmentResource::id() )
{ }

Segment
  ::Segment
    ( SegmentID segmentId
    , std::size_t segmentSize )
: SegmentResource( segmentId, segmentSize )
, SegmentManager( SegmentResource::id() )
{

}

#undef GASPI_CHECK

} // namespace segment
} // namespace gaspi
