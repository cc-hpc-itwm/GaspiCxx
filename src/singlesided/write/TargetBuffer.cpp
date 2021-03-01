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
 * TargetBuffer.cpp
 *
 */

#include <cassert>

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <GaspiCxx/utility/serialization.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

TargetBuffer
  ::TargetBuffer
   ( std::size_t size )
: Endpoint
  ( size
  , Endpoint::Type::TARGET )
{}

TargetBuffer
  ::TargetBuffer
   ( segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( segment
  , size
  , Endpoint::Type::TARGET )
{}

TargetBuffer
  ::TargetBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( pointer
  , segment
  , size
  , Endpoint::Type::TARGET )
{}

TargetBuffer
  ::TargetBuffer
   ( std::size_t size
   , segment
       ::Notification notification )
: Endpoint
  ( size
  , notification
  , Endpoint::Type::TARGET )
{}

TargetBuffer
  ::TargetBuffer
   ( segment::Segment & segment
   , std::size_t size
   , segment
       ::Notification notification )
: Endpoint
  ( segment
  , size
  , notification
  , Endpoint::Type::TARGET )
{}

TargetBuffer
  ::TargetBuffer
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
  , Endpoint::Type::TARGET )
{}

TargetBuffer
  ::TargetBuffer
   ( TargetBuffer const& other )
: Endpoint
  ( other._pointer
  , other._segment
  , other._size
  , Endpoint::Type::TARGET )
{
  // Take shared ownership of existing memory allocation
  _allocMemory = other._allocMemory;
}

Endpoint::ConnectHandle
TargetBuffer
  ::connectToRemoteSource
   ( group::Group const& group
   , group::Rank const& rank
   , Tag const& tag )
{
  return Endpoint::connectToRemotePartner
      ( group
      , rank
      , tag );
}

bool
TargetBuffer
  ::waitForCompletion
   ( )
{
  assert(Endpoint::isConnected());

  return Buffer::waitForNotification();
}

bool
TargetBuffer
  ::checkForCompletion
   ( )
{
  assert(Endpoint::isConnected());

  return Buffer::checkForNotification();
}

void
TargetBuffer
  ::ackTransfer
   ()
{
  ackTransfer(getRuntime().getDefaultCommunicationContext());
}

void
TargetBuffer
  ::ackTransfer
   ( CommunicationContext& comm_context )
{
  assert(Endpoint::isConnected());

  comm_context.notify(Endpoint::otherBufferDesc());
}

} // namespace write
} // namespace singlesided
} // namespace gaspi
