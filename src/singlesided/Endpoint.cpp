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
 * Endpoint.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/Endpoint.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <GaspiCxx/utility/serialization.hpp>

namespace gaspi {
namespace singlesided {

Endpoint::ConnectHandle
  ::ConnectHandle
  ( Endpoint & commBuffer
  , std::unique_ptr<Buffer> pSendBuffer
  , std::unique_ptr<Buffer> pRecvBuffer )
: _commBuffer(commBuffer)
, _pSendBuffer(std::move(pSendBuffer))
, _pRecvBuffer(std::move(pRecvBuffer))
{}

void
Endpoint::ConnectHandle
  ::waitForCompletion() {

  if(!_pRecvBuffer->waitForNotification()) {
    throw std::runtime_error(CODE_ORIGIN+"Unexpected behavior");
  }

  BufferDescription remotePartnerDescription;

  serialization::deserialize
      ( remotePartnerDescription
      , _pRecvBuffer->address());

  _commBuffer.setRemotePartner(remotePartnerDescription);

  if(!_pSendBuffer->waitForNotification()) {
      throw std::runtime_error(CODE_ORIGIN+"Unexpected behavior");
  }
}

Endpoint
  ::Endpoint
   ( segment::Segment & segment
   , std::size_t size
   , Type type )
: Buffer
  ( segment
  , size )
, _pLocalBufferDesc
    (std::make_unique<BufferDescription>(Buffer::description()))
, _pOtherBufferDesc
    (std::make_unique<BufferDescription>())
, _isConnected(false)
, _type(type)
{}

Endpoint
  ::Endpoint
   ( std::size_t size
   , Type type )
: Endpoint(gaspi::getRuntime().getFreeSegment(size), size, type)
{ }

Endpoint
  ::Endpoint
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , Type type )
: Buffer
  ( pointer
  , segment
  , size )
, _pLocalBufferDesc
    (std::make_unique<BufferDescription>(Buffer::description()))
, _pOtherBufferDesc
    (std::make_unique<BufferDescription>())
, _isConnected(false)
, _type(type)
{}

Endpoint
  ::Endpoint
   ( segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification
   , Type type )
: Buffer
  ( segment
  , size
  , notification )
, _pLocalBufferDesc
    (std::make_unique<BufferDescription>(Buffer::description()))
, _pOtherBufferDesc
    (std::make_unique<BufferDescription>())
, _isConnected(false)
, _type(type)
{}

Endpoint
  ::Endpoint
   ( std::size_t size
   , segment
       ::Notification notification
   , Type type )
: Endpoint(gaspi::getRuntime().getFreeSegment(size), size, notification, type)
{ }

Endpoint
  ::Endpoint
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification
   , Type type )
: Buffer
  ( pointer
  , segment
  , size
  , notification )
, _pLocalBufferDesc
    (std::make_unique<BufferDescription>(Buffer::description()))
, _pOtherBufferDesc
    (std::make_unique<BufferDescription>())
, _isConnected(false)
, _type(type)
{}

Endpoint
  ::~Endpoint
   ()
{

}

void
Endpoint
  ::setRemotePartner
   ( BufferDescription const & partnerDescription ) {
  *_pOtherBufferDesc = partnerDescription;
  _isConnected = true;
}

Endpoint::ConnectHandle
Endpoint
  ::connectToRemotePartner
   ( group::Group const& group
   , group::Rank const& rank
   , Tag const& tag )
{
  if (!group.contains_rank(rank))
  {
    throw std::logic_error
      (CODE_ORIGIN + "`group` does not contain `rank`");
  }

  if( static_cast<passive::Passive::Tag>(tag)
      >= std::numeric_limits<passive::Passive::Tag>::max() / 3 ) {
    throw std::runtime_error
      (CODE_ORIGIN + "tag out of bounds");
  }

  passive::Passive::Tag sendTag ( 3 * static_cast<passive::Passive::Tag>(tag) );
  passive::Passive::Tag recvTag ( 3 * static_cast<passive::Passive::Tag>(tag) );

  switch (_type)
  {
    case SOURCE : {
      sendTag += 1;
      recvTag += 2;
      break;
    }

    case TARGET : {
      sendTag += 2;
      recvTag += 1;
      break;
    }

    case GENERIC : {
      sendTag += 0;
      recvTag += 0;
      break;
    }

    default: {
      throw std::runtime_error
        (CODE_ORIGIN + "Unsupported Endpoint::Type");
    }
  }

  auto const global_rank = group.toGlobalRank(rank);
  _segment.remoteRegistration( global_rank );

  std::unique_ptr<Buffer> pSendBuffer
    ( new Buffer( _segment
                , serialization::size(localBufferDesc()) ) );

  serialization::serialize (pSendBuffer->address(), localBufferDesc() );

  getRuntime().passive().iSendTagMessg
      ( global_rank
      , sendTag
      , *pSendBuffer );


  std::unique_ptr<Buffer> pRecvBuffer
    ( new Buffer( _segment
                , serialization::size( otherBufferDesc() ) ) );

  getRuntime().passive().iRecvTagMessg
    ( global_rank
    , recvTag
    , *pRecvBuffer);

  return ConnectHandle
      (*this,
       std::move(pSendBuffer),
       std::move(pRecvBuffer));
}


bool
Endpoint
  ::isConnected
   () const
{
  return _isConnected;
}

BufferDescription &
Endpoint
  ::localBufferDesc()
{
  return *_pLocalBufferDesc;
}

BufferDescription const &
Endpoint
  ::localBufferDesc() const
{
  return *_pLocalBufferDesc;
}

BufferDescription &
Endpoint
  ::otherBufferDesc()
{
  return *_pOtherBufferDesc;
}

BufferDescription const &
Endpoint
  ::otherBufferDesc() const
{
  return *_pOtherBufferDesc;
}

} // namespace singlesided
} // namespace gaspi
