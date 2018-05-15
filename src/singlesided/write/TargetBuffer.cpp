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
: CommBuffer
  ( segment
  , size )
{}

TargetBuffer
  ::TargetBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: CommBuffer
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
: CommBuffer
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
: CommBuffer
  ( pointer
  , segment
  , size
  , notification )
{}

TargetBuffer
  ::~TargetBuffer
    ()
{}

CommBuffer::ConnectHandle
TargetBuffer
  ::connectToRemoteSource
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{
  return CommBuffer::connectToRemotePartner
      ( context
      , rank
      , tag );
}

void
TargetBuffer
  ::waitForCompletion
   ( )
{
  gaspi_notification_id_t id;
  gaspi_notification_t    value;

  GASPI_CHECK(gaspi_notify_waitsome(CommBuffer::_segment.id(),
                                    CommBuffer::_notification,
                                    1,
                                    &id,
                                    GASPI_BLOCK));
  GASPI_CHECK(gaspi_notify_reset(CommBuffer::_segment.id(), id, &value));
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
    ( gaspi_notify_waitsome( CommBuffer::_segment.id()
                           , CommBuffer::_notification
                           , 1
                           , &id
                           , GASPI_TEST ) );

  if( gaspi_return != GASPI_TIMEOUT) {

    GASPI_CHECK( gaspi_return );

    GASPI_CHECK( gaspi_notify_reset(CommBuffer::_segment.id(), id, &value) );

    if( value != 0 ) {
      ret = true;
    }
  }

  return ret;
}

void
TargetBuffer
  ::ackTransfer
   (Context & context)
{
  context.notify(CommBuffer::_otherBufferDesc);
}

} // namespace write
} // namespace singlesided
} // namespace gaspi
