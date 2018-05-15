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
 * CommBuffer.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/write/CommBuffer.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <GaspiCxx/utility/serialization.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

CommBuffer::ConnectHandle
  ::ConnectHandle
  ( CommBuffer & commBuffer
  , std::unique_ptr<TargetBuffer> pSendBuffer
  , std::unique_ptr<TargetBuffer> pRecvBuffer )
: _commBuffer(commBuffer)
, _pSendBuffer(std::move(pSendBuffer))
, _pRecvBuffer(std::move(pRecvBuffer))
{}

void
CommBuffer::ConnectHandle
  ::waitForCompletion() {

  _pSendBuffer->waitForCompletion();

  serialization::deserialize
     ( _commBuffer._localBufferDesc
     , _pSendBuffer->address() );

  _pRecvBuffer->waitForCompletion();

  serialization::deserialize
      ( _commBuffer._otherBufferDesc
      , _pRecvBuffer->address());
}

CommBuffer
  ::CommBuffer
   ( segment::Segment & segment
   , std::size_t size )
: _allocMemory(true)
, _allocNotify(true)
, _pointer( reinterpret_cast<void*>(segment.allocator().allocate(size)) )
, _size (size)
, _notification(segment.acquire_notification())
, _segment(segment)
, _localBufferDesc()
, _otherBufferDesc()
{}

CommBuffer
  ::CommBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: _allocMemory(false)
, _allocNotify(true)
, _pointer( pointer )
, _size (size)
, _notification(segment.acquire_notification())
, _segment(segment)
, _localBufferDesc()
, _otherBufferDesc()
{}

CommBuffer
  ::CommBuffer
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
, _localBufferDesc()
, _otherBufferDesc()
{}

CommBuffer
  ::CommBuffer
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
, _localBufferDesc()
, _otherBufferDesc()
{}

CommBuffer
  ::~CommBuffer
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
CommBuffer
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
CommBuffer
  ::address
   () const
{
  return _pointer;
}

CommBuffer::ConnectHandle
CommBuffer
  ::connectToRemotePartner
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{

  _segment.remoteRegistration
      ( group::groupToGlobalRank( context.group()
                                , rank ) );

  BufferDescription localBufferDesc(description());

  std::unique_ptr<TargetBuffer> pSendBuffer
    ( new TargetBuffer( _segment
                      , serialization::size(localBufferDesc) ) );

  serialization::serialize (pSendBuffer->address(), localBufferDesc);

  getRuntime().passive().iSendTagMessg
      ( group::groupToGlobalRank( context.group()
                                , rank )
      , tag
      , *pSendBuffer );


  std::unique_ptr<TargetBuffer> pRecvBuffer
    ( new TargetBuffer( _segment
                      , serialization::size(_otherBufferDesc) ) );

  getRuntime().passive().iRecvTagMessg
    (group::groupToGlobalRank( context.group()
                             , rank )
    ,tag
    ,*pRecvBuffer);

  return ConnectHandle
      (*this,
       std::move(pSendBuffer),
       std::move(pRecvBuffer));
}

} // namespace write
} // namespace singlesided
} // namespace gaspi
