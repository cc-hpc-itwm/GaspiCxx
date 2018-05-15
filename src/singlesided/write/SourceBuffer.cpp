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
#include <GaspiCxx/singlesided/write/CommBuffer.hpp>
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
: CommBuffer
  ( segment
  , size )
{}

SourceBuffer
  ::SourceBuffer
   ( void * const pointer
   , segment::Segment & segment
   , std::size_t size )
: CommBuffer
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
: CommBuffer
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
: CommBuffer
  ( pointer
  , segment
  , size
  , notification)
{}

SourceBuffer
  ::~SourceBuffer
    ()
{ }

void
SourceBuffer
  ::connectToRemoteTarget
   ( Context & context
   , group::Rank & rank
   , Tag & tag )
{
  CommBuffer::connectToRemotePartner
    ( context
    , rank
    , tag ).waitForCompletion();
}

void
SourceBuffer
  ::initTransfer
   ( Context & context )
{
  context.write
     ( CommBuffer::_localBufferDesc
     , CommBuffer::_otherBufferDesc );
}

void
SourceBuffer
  ::checkForTransferAck
  (Context & context)
{
  context.checkForBufferNotification(CommBuffer::_localBufferDesc);
}

void
SourceBuffer
  ::waitForTransferAck
  (Context & context)
{
  context.waitForBufferNotification(CommBuffer::_localBufferDesc);
}


} // namespace write
} // namespace singlesided
} // namespace gaspi
