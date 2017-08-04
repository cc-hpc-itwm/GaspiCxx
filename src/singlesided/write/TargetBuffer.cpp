/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiLS.
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
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * TargetBuffer.hpp
 *
 */

#include <group/Group.hpp>
#include <passive/Passive.hpp>
#include <Runtime.hpp>
#include <singlesided/write/TargetBuffer.hpp>
#include <utility/Macros.hpp>
#include <utility/serialization.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

TargetBuffer
  ::TargetBuffer
   ( segment::Segment & segment
   , std::size_t size )
: _allocMemory(true)
, _allocNotify(true)
, _pointer( reinterpret_cast<void*>(segment.allocator().allocate(size)) )
, _size (size)
, _notification(segment.acquire_notification())
, _segment(segment)
{}

TargetBuffer
  ::TargetBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: _allocMemory(false)
, _allocNotify(true)
, _pointer( pointer )
, _size (size)
, _notification(segment.acquire_notification())
, _segment(segment)
{}

TargetBuffer
  ::TargetBuffer
   ( segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: _allocMemory(true)
, _allocNotify(false)
, _pointer( reinterpret_cast<void*>(segment.allocator().allocate(size)) )
, _size (size)
, _notification(notification)
, _segment(segment)
{}

TargetBuffer
  ::TargetBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: _allocMemory(false)
, _allocNotify(false)
, _pointer( pointer )
, _size (size)
, _notification(notification)
, _segment(segment)
{}

TargetBuffer
  ::~TargetBuffer
    ()
{
  if( _allocMemory ) {
    _segment.allocator().deallocate
          ( reinterpret_cast<char*>(_pointer)
          , _size );
  }
  if( _allocNotify ) {
    _segment.release_notification(_notification);
  }
}

BufferDescription
TargetBuffer
  ::description
   () const
{
  BufferDescription desc
    ( group::groupToGlobalRank( getRuntime().group()
                              , getRuntime().rank() )
    , _segment.id()
    , _segment.pointerToOffset
        (_pointer)
    , _size
    , _notification );

  return desc;
}

void *
TargetBuffer
  ::address
   () const
{
  return _pointer;
}

std::unique_ptr<TargetBuffer>
TargetBuffer
  ::connectToRemoteSource
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{
  _segment.remoteRegistration
    ( group::groupToGlobalRank( context.group()
                              , rank ) );

  BufferDescription bufferDesc(description());

  std::unique_ptr<TargetBuffer> pBuffer
    ( new TargetBuffer( _segment
                      , serialization::size(bufferDesc) ) );

  serialization::serialize (pBuffer->address(), bufferDesc);

  getRuntime().passive().iSendTagMessg
    ( group::groupToGlobalRank( context.group()
                              , rank )
    ,tag
    ,*pBuffer );

  return pBuffer;
//  pBuffer->waitForCompletion();

}

void
TargetBuffer
  ::waitForCompletion
   ( )
{
  gaspi_notification_id_t id;
  gaspi_notification_t    value;

  GASPI_CHECK(gaspi_notify_waitsome(_segment.id(),
                                    _notification,
                                    1,
                                    &id,
                                    GASPI_BLOCK));
  GASPI_CHECK(gaspi_notify_reset(_segment.id(), id, &value));
}

bool
TargetBuffer
  ::checkForCompletion
   ( )
{
  bool ret(false);

  gaspi_notification_id_t id;
  gaspi_notification_t    value;

  gaspi_return_t gaspi_return
    ( gaspi_notify_waitsome( _segment.id()
                           , _notification
                           , 1
                           , &id
                           , GASPI_TEST ) );

  if( gaspi_return != GASPI_TIMEOUT) {

    GASPI_CHECK( gaspi_return );

    GASPI_CHECK( gaspi_notify_reset(_segment.id(), id, &value) );

    if( value != 0 ) {
      ret = true;
    }
  }

  return ret;
}

} // namespace write
} // namespace singlesided
} // namespace gaspi
