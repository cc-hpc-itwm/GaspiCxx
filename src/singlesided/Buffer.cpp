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
 * Buffer.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/segment/SegmentManager.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {
namespace singlesided {

class MemoryAllocation {

  public:

    MemoryAllocation
      ( segment::Allocator<char> const & allocator
      , std::size_t size )
    : _allocator(allocator)
    , _pointer(_allocator.allocate(size))
    , _size(size)
    {};

    ~MemoryAllocation
      ()
    {
      _allocator.deallocate(_pointer,_size);
    }

    void *
    pointer
      () const {
      return reinterpret_cast<void *>(_pointer);
    }

  private:

    segment::Allocator<char> _allocator;
    char * const             _pointer;
    std::size_t              _size;


};

class NotifyAllocation {

  public:

    NotifyAllocation
      ( segment::SegmentManager & segmentManager )
    : _segmentManager(segmentManager)
    , _notification(_segmentManager.acquire_notification())
    {};

    ~NotifyAllocation
      ()
    {
      _segmentManager.release_notification(_notification);
    }

    segment::Notification const &
    notification
      () const {
      return _notification;
    }

  private:

    segment::SegmentManager & _segmentManager;
    segment::Notification     _notification;

};

Buffer
  ::Buffer
   ( std::size_t size )
: Buffer(gaspi::getRuntime().getFreeSegment(size)
  , size)
{ }

Buffer
  ::Buffer
   ( segment::Segment & segment
   , std::size_t size )
: _allocMemory(new MemoryAllocation(segment.allocator(),size))
, _allocNotify(new NotifyAllocation(segment))
, _pointer( _allocMemory->pointer() )
, _size (size)
, _notification(_allocNotify->notification() )
, _segment(segment)
{}

Buffer
  ::Buffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: _allocMemory()
, _allocNotify(new NotifyAllocation(segment))
, _pointer( pointer )
, _size (size)
, _notification(_allocNotify->notification())
, _segment(segment)
{}

Buffer
  ::Buffer
   ( std::size_t size
   , segment
       ::Notification notification )
: Buffer(gaspi::getRuntime().getFreeSegment(size)
  , size, notification)
{ }

Buffer
  ::Buffer
   ( segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification )
: _allocMemory(new MemoryAllocation(segment.allocator(),size))
, _allocNotify()
, _pointer( _allocMemory->pointer() )
, _size (size)
, _notification(notification)
, _segment(segment)
{}

Buffer
  ::Buffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification )
: _allocMemory()
, _allocNotify()
, _pointer( pointer )
, _size (size)
, _notification(notification)
, _segment(segment)
{}

Buffer
  ::~Buffer
    ()
{ }

BufferDescription
Buffer
  ::description
   () const
{
  BufferDescription desc
    ( getRuntime().group().toGlobalRank( getRuntime().rank() )
    , _segment.id()
    , _segment.pointerToOffset
        (_pointer)
    , _size
    , _notification );

  return desc;
}

void *
Buffer
  ::address
   () const
{
  return _pointer;
}

bool
Buffer
  ::checkForNotification
   ()
{
  bool ret(false);

  gaspi_segment_id_t      segId(_segment.id());
  gaspi_notification_id_t activeId;
  gaspi_notification_t    value;

  gaspi_return_t gaspi_return
    (gaspi_notify_waitsome
       ( segId
       , _notification
       , 1
       , &activeId
       , GASPI_TEST ) );

  if( gaspi_return != GASPI_TIMEOUT) {

    GASPI_CHECK
      (gaspi_return);

    GASPI_CHECK
      (gaspi_notify_reset
         ( segId
         , activeId
         , &value) );

    if( value != 0 ) {
      ret = true;
    }
  }

  return ret;
}

bool
Buffer
  ::waitForNotification
   ()
{
  bool ret(false);

  gaspi_segment_id_t      segId(_segment.id());
  gaspi_notification_id_t activeId;
  gaspi_notification_t    value;

  GASPI_CHECK
    (gaspi_notify_waitsome
       ( segId
       , _notification
       , 1
       , &activeId
       , GASPI_BLOCK ) );

  GASPI_CHECK
    (gaspi_notify_reset
       ( segId
       , activeId
       , &value) );

  if( value != 0 ) {
    ret = true;
  }

  return ret;
}

} // namespace singlesided
} // namespace gaspi
