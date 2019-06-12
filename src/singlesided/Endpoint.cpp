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
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
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
   , std::size_t size )
: Buffer
  ( segment
  , size )
, _localBufferDesc(Buffer::description())
, _otherBufferDesc()
, _isConnected(false)
{}

Endpoint
  ::Endpoint
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: Buffer
  ( pointer
  , segment
  , size )
, _localBufferDesc(Buffer::description())
, _otherBufferDesc()
, _isConnected(false)
{}

Endpoint
  ::Endpoint
   ( segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: Buffer
  ( segment
  , size
  , notification )
, _localBufferDesc(Buffer::description())
, _otherBufferDesc()
, _isConnected(false)
{}

Endpoint
  ::Endpoint
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: Buffer
  ( pointer
  , segment
  , size
  , notification )
, _localBufferDesc(Buffer::description())
, _otherBufferDesc()
, _isConnected(false)
{}

void
Endpoint
  ::setRemotePartner
   ( BufferDescription const & partnerDescription ) {
  _otherBufferDesc = partnerDescription;
  _isConnected = true;
}

Endpoint::ConnectHandle
Endpoint
  ::connectToRemotePartner
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{

  _segment.remoteRegistration
      ( group::groupToGlobalRank( context.group()
                                , rank ) );

  std::unique_ptr<Buffer> pSendBuffer
    ( new Buffer( _segment
                , serialization::size(_localBufferDesc) ) );

  serialization::serialize (pSendBuffer->address(), _localBufferDesc);

  getRuntime().passive().iSendTagMessg
      ( group::groupToGlobalRank( context.group()
                                , rank )
      , tag
      , *pSendBuffer );


  std::unique_ptr<Buffer> pRecvBuffer
    ( new Buffer( _segment
                , serialization::size(_otherBufferDesc) ) );

  getRuntime().passive().iRecvTagMessg
    (group::groupToGlobalRank( context.group()
                             , rank )
    , tag
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

} // namespace singlesided
} // namespace gaspi
