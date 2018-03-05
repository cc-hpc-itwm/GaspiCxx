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
 * SourceBuffer.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <GaspiCxx/utility/serialization.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

SourceBuffer
  ::SourceBuffer
   ( segment::Segment & segment
   , std::size_t size )
: _allocMemory(true)
, _allocNotify(true)
, _pointer( reinterpret_cast<void*>(segment.allocator().allocate(size)) )
, _size (size)
, _notification(segment.acquire_notification())
, _segment(segment)
, _targetBufferDesc()
{}

SourceBuffer
  ::SourceBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: _allocMemory(false)
, _allocNotify(true)
, _pointer( pointer )
, _size (size)
, _notification(segment.acquire_notification())
, _segment(segment)
, _targetBufferDesc()
{}

SourceBuffer
  ::SourceBuffer
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
, _targetBufferDesc()
{}

SourceBuffer
  ::SourceBuffer
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
, _targetBufferDesc()
{}

SourceBuffer
  ::~SourceBuffer
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
SourceBuffer
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
SourceBuffer
  ::address
   () const
{
  return _pointer;
}

void
SourceBuffer
  ::connectToRemoteTarget
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{

  _segment.remoteRegistration
    (group::groupToGlobalRank( context.group()
                             , rank ) );

  std::unique_ptr<TargetBuffer> pBuffer
    ( new TargetBuffer( _segment
                      , serialization::size(_targetBufferDesc) ) );

  getRuntime().passive().iRecvTagMessg
    (group::groupToGlobalRank( context.group()
                             , rank )
    ,tag
    ,*pBuffer);

  pBuffer->waitForCompletion();

  serialization::deserialize
    ( _targetBufferDesc
    , pBuffer->address());
}

void
SourceBuffer
  ::initTransfer
   ( Context & context )
{
  context.write
    ( this->description()
    , _targetBufferDesc );
}


} // namespace write
} // namespace singlesided
} // namespace gaspi
