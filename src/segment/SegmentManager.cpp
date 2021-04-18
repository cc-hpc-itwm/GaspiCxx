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
 * SegmentManager.cpp
 *
 */

#include <stdexcept>
#include <sstream>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/SegmentManager.hpp>
#include <GaspiCxx/utility/Macros.hpp>

extern "C" {
#include <GASPI.h>
#include <GASPI_Ext.h>
}

namespace gaspi {
namespace segment {

namespace {

  std::size_t
  getSegmentSize
    (SegmentID segmentID)
  {
    gaspi_size_t segmentSize;
    {
      gaspi_rank_t rank;
      GASPI_CHECK(gaspi_proc_rank(&rank));

      GASPI_CHECK(gaspi_segment_size(segmentID, rank, &segmentSize));
    }
    return static_cast<std::size_t>(segmentSize);
  }

  void *
  getSegmentPtr
    (SegmentID segmentID)
  {
    gaspi_pointer_t segmentPtr;
    {
      GASPI_CHECK(gaspi_segment_ptr(segmentID, &segmentPtr));
    }
    return static_cast<void*>(segmentPtr);
  }

  std::size_t
  getNumNotificationsMax
    ()
  {

    gaspi_number_t nmax;
    {
      GASPI_CHECK(gaspi_notification_num (&nmax));
    }
    return nmax;
  }

}

SegmentManager
  ::SegmentManager
    ( SegmentID segmentID )
: _segmentID(segmentID)
, _memoryManager
  ( std::make_unique<MemoryManager>
    ( getSegmentPtr (segmentID)
    , getSegmentSize(segmentID) )
    )
, _notifyManager
  ( std::make_unique<NotificationManager>
    ( getNumNotificationsMax() )
  )
{

}

SegmentID
SegmentManager
  ::id
   () const
{
  return _segmentID;
}

std::size_t
SegmentManager
  ::size
   () const
{
  return getSegmentSize(id());
}

std::size_t
SegmentManager
  ::pointerToOffset
   ( void const * const pointer) const
{
  return reinterpret_cast<gaspi_offset_t>(pointer)
       - reinterpret_cast<gaspi_offset_t>(getSegmentPtr(id()));
}

Allocator<char>
SegmentManager
  ::allocator
   ()
{
  return Allocator<char>(_memoryManager.get());
}

bool
SegmentManager
  ::hasFreeMemory( std::size_t size )
{
  return _memoryManager->canAllocate(size);
}

Notification
SegmentManager
  ::acquire_notification
    ()
{
  return _notifyManager->allocate( 1 );
}

void
SegmentManager
  ::release_notification
    (Notification const & notification)
{
  _notifyManager->deallocate( static_cast<std::size_t>(notification) );
}

#undef GASPI_CHECK

} // namespace segment
} // namespace gaspi
