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
 * TargetBuffer.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
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
   ( segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( segment
  , size )
{}

TargetBuffer
  ::TargetBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: Endpoint
  ( pointer
  , segment
  , size )
{}

TargetBuffer
  ::TargetBuffer
   ( segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: Endpoint
  ( segment
  , size
  , notification )
{}

TargetBuffer
  ::TargetBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size
   , segment::Segment
       ::Notification notification )
: Endpoint
  ( pointer
  , segment
  , size
  , notification )
{}

Endpoint::ConnectHandle
TargetBuffer
  ::connectToRemoteSource
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{
  return Endpoint::connectToRemotePartner
      ( context
      , rank
      , tag );
}

bool
TargetBuffer
  ::waitForCompletion
   ( )
{
  return Buffer::waitForNotification();
}

bool
TargetBuffer
  ::checkForCompletion
   ( )
{
  return Buffer::checkForNotification();
}

void
TargetBuffer
  ::ackTransfer
   (Context & context)
{
  context.notify(Endpoint::_otherBufferDesc);
}

} // namespace write
} // namespace singlesided
} // namespace gaspi
