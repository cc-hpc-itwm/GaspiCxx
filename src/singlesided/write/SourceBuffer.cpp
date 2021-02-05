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
 * SourceBuffer.cpp
 *
 */

#include <cassert>

#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
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
  , size
  , Endpoint::Type::SOURCE)
{}

SourceBuffer
  ::SourceBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( pointer
  , segment
  , size
  , Endpoint::Type::SOURCE )
{}

SourceBuffer
  ::SourceBuffer
   ( segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification )
: Endpoint
  ( segment
  , size
  , notification
  , Endpoint::Type::SOURCE )
{}

SourceBuffer
  ::SourceBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification )
: Endpoint
  ( pointer
  , segment
  , size
  , notification
  , Endpoint::Type::SOURCE )
{}

SourceBuffer
  ::~SourceBuffer
    ()
{ }

Endpoint::ConnectHandle
SourceBuffer
  ::connectToRemoteTarget
   ( Context & context
   , group::Rank const& rank
   , Tag const& tag )
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
     ( Endpoint::localBufferDesc()
     , Endpoint::otherBufferDesc() );
}

void
SourceBuffer
  ::initTransferPart
   ( Context & context
   , std::size_t size
   , std::size_t offset )
{
  assert(Endpoint::isConnected());

  context.writePart
     ( Endpoint::localBufferDesc()
     , Endpoint::otherBufferDesc()
     , size
     , offset );
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
