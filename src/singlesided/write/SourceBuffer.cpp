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

#include <cassert>

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
namespace write {

SourceBuffer
  ::SourceBuffer
   ( segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( segment
  , size )
{}

SourceBuffer
  ::SourceBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( pointer
  , segment
  , size )
{}

SourceBuffer
  ::SourceBuffer
   ( segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: Endpoint
  ( segment
  , size
  , notification)
{}

SourceBuffer
  ::SourceBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: Endpoint
  ( pointer
  , segment
  , size
  , notification)
{}

SourceBuffer
  ::~SourceBuffer
    ()
{ }

Endpoint::ConnectHandle
SourceBuffer
  ::connectToRemoteTarget
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{
  return Endpoint::connectToRemotePartner
      ( context
      , rank
      , tag );
}

void
SourceBuffer
  ::initTransfer
   ( Context & context )
{
  assert(Endpoint::isConnected());

  context.write
     ( Endpoint::_localBufferDesc
     , Endpoint::_otherBufferDesc );
}

bool
SourceBuffer
  ::checkForTransferAck
  ( )
{
  assert(Endpoint::isConnected());

  return Buffer::checkForNotification();
}

bool
SourceBuffer
  ::waitForTransferAck
  ( )
{
  assert(Endpoint::isConnected());

  return Buffer::waitForNotification();
}


} // namespace write
} // namespace singlesided
} // namespace gaspi
