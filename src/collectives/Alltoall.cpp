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
 * Alltoall.cpp
 *
 */

#include <GaspiCxx/collectives/Alltoall.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>
#include <GaspiCxx/utility/serialization.hpp>

#include <vector>
#include <memory>

namespace gaspi {
namespace collectives {

// allgather: every process with process id iProc sends its  data of size size
//            from source buffer into the target buffer at location iProc * size
void
alltoall
  ( void * const gSource
  , segment::Segment & sourceSegment
  , void * const gTarget
  , segment::Segment & targetSegment
  , std::size_t const & size
  , Context & context )
{
  // generate sourceBuffer description for every

  std::vector<singlesided::BufferDescription>
        localSourceDescriptions(context.size().get());
  std::vector<singlesided::BufferDescription>
        localTargetDescriptions(context.size().get());
  std::vector<singlesided::BufferDescription>
        remoteSourceDescriptions(context.size().get());
  std::vector<singlesided::BufferDescription>
        remoteTargetDescriptions(context.size().get());

  std::vector<std::unique_ptr<singlesided::Buffer> >
      descriptionSendBuffers( context.size().get() );
  std::vector<std::unique_ptr<singlesided::Buffer> >
      descriptionRecvBuffers( context.size().get() );

  for( int i(0)
     ;     i<context.size().get()
     ;   ++i ) {

    group::Rank iGroup(i);

    gaspi_rank_t const iGlobalRank
      (group::groupToGlobalRank
         ( context.group()
         , iGroup ) );

    sourceSegment.remoteRegistration( iGlobalRank );
    targetSegment.remoteRegistration( iGlobalRank );

    {
      localSourceDescriptions[i].rank()
          = group::groupToGlobalRank
              ( context.group()
              , context.rank() );
      localSourceDescriptions[i].segmentId()
          = sourceSegment.id();
      localSourceDescriptions[i].offset()
          = sourceSegment.pointerToOffset( gSource )
          + i * size;
      localSourceDescriptions[i].size()
          = size;
      localSourceDescriptions[i].notificationId()
          = sourceSegment.acquire_notification();
    }

    {
      localTargetDescriptions[i].rank()
          = group::groupToGlobalRank
              ( context.group()
              , context.rank() );
      localTargetDescriptions[i].segmentId()
          = targetSegment.id();
      localTargetDescriptions[i].offset()
          = targetSegment.pointerToOffset( gTarget )
          + i * size;
      localTargetDescriptions[i].size()
          =  size;
      localTargetDescriptions[i].notificationId()
          = targetSegment.acquire_notification();
    }

    descriptionSendBuffers[i].reset
      ( new singlesided::Buffer
          ( targetSegment
          , serialization::size(localTargetDescriptions[i])
          + serialization::size(localSourceDescriptions[i]) ) );

    char * cPtr (static_cast<char *> (descriptionSendBuffers[i]->address()));
    cPtr += serialization::serialize (cPtr, localTargetDescriptions[i]);
    cPtr += serialization::serialize (cPtr, localSourceDescriptions[i]);

    descriptionRecvBuffers[i].reset
      ( new singlesided::Buffer
         ( targetSegment
         , serialization::size(remoteTargetDescriptions[i])
         + serialization::size(remoteSourceDescriptions[i]) ) );

    passive::Passive & passive(getRuntime().passive());

    passive.iSendTagMessg
      ( iGlobalRank
      , context.rank().get()
      , *descriptionSendBuffers[i] );

    passive.iRecvTagMessg
      ( iGlobalRank
      , i
      , *descriptionRecvBuffers[i] );
  }

  for(int i(0)
     ;    i<context.size().get()
     ;  ++i) {

    descriptionRecvBuffers[i]->waitForNotification();
    {
      char * cPtr (static_cast<char *> (descriptionRecvBuffers[i]->address()));
      cPtr += serialization::deserialize (remoteTargetDescriptions[i], cPtr);
      cPtr += serialization::deserialize (remoteSourceDescriptions[i], cPtr);
    }
    descriptionRecvBuffers[i].reset( nullptr );

    context.write
      ( localSourceDescriptions[i]
      , remoteTargetDescriptions[i] );
  }

  // release notifications of target segment;
  for(int i(0)
     ;    i<context.size().get()
     ;  ++i) {

    context.waitForBufferNotification(localTargetDescriptions[i]);

    context.notify(remoteSourceDescriptions[i]);

    targetSegment.release_notification
      (localTargetDescriptions[i].notificationId());
  }

  for(int i(0)
     ;    i<context.size().get()
     ;  ++i) {

    context.waitForBufferNotification(localSourceDescriptions[i]);

    descriptionSendBuffers[i]->waitForNotification();

    descriptionSendBuffers[i].reset( nullptr );

    sourceSegment.release_notification
          (localSourceDescriptions[i].notificationId());
  }

  context.flush();

}

void
alltoallv
  ( void * const gSource
  , segment::Segment & sourceSegment
  , std::size_t const * const sourceSizes
  , void * const gTarget
  , segment::Segment & targetSegment
  , std::size_t const * const targetSizes
  , Context & context )
{
  // generate sourceBuffer description for every

  std::vector<singlesided::BufferDescription>
        localSourceDescriptions(context.size().get());
  std::vector<singlesided::BufferDescription>
        localTargetDescriptions(context.size().get());
  std::vector<singlesided::BufferDescription>
        remoteSourceDescriptions(context.size().get());
  std::vector<singlesided::BufferDescription>
        remoteTargetDescriptions(context.size().get());

  std::vector<std::unique_ptr<singlesided::Buffer> >
      descriptionSendBuffers( context.size().get() );
  std::vector<std::unique_ptr<singlesided::Buffer> >
      descriptionRecvBuffers( context.size().get() );

  std::size_t sourceOffset(0);
  std::size_t targetOffset(0);

  for( int i(0)
     ;     i<context.size().get()
     ;   ++i ) {

    group::Rank iGroup(i);

    gaspi_rank_t const iGlobalRank
      (group::groupToGlobalRank
         ( context.group()
         , iGroup ) );

    sourceSegment.remoteRegistration( iGlobalRank );
    targetSegment.remoteRegistration( iGlobalRank );

    {
      localSourceDescriptions[i].rank()
          = group::groupToGlobalRank
              ( context.group()
              , context.rank() );
      localSourceDescriptions[i].segmentId()
          = sourceSegment.id();
      localSourceDescriptions[i].offset()
          = sourceSegment.pointerToOffset( gSource )
          + sourceOffset;
      localSourceDescriptions[i].size()
          = sourceSizes[i];
      localSourceDescriptions[i].notificationId()
          = sourceSegment.acquire_notification();
    }

    sourceOffset += sourceSizes[i];

    {
      localTargetDescriptions[i].rank()
          = group::groupToGlobalRank
              ( context.group()
              , context.rank() );
      localTargetDescriptions[i].segmentId()
          = targetSegment.id();
      localTargetDescriptions[i].offset()
          = targetSegment.pointerToOffset( gTarget )
          + targetOffset;
      localTargetDescriptions[i].size()
          =  targetSizes[i];
      localTargetDescriptions[i].notificationId()
          = targetSegment.acquire_notification();
    }

    targetOffset += targetSizes[i];

    descriptionSendBuffers[i].reset
      ( new singlesided::Buffer
          ( targetSegment
          , serialization::size(localTargetDescriptions[i])
          + serialization::size(localSourceDescriptions[i]) ) );

    char * cPtr (static_cast<char *> (descriptionSendBuffers[i]->address()));
    cPtr += serialization::serialize (cPtr, localTargetDescriptions[i]);
    cPtr += serialization::serialize (cPtr, localSourceDescriptions[i]);

    descriptionRecvBuffers[i].reset
      ( new singlesided::Buffer
         ( targetSegment
         , serialization::size(remoteTargetDescriptions[i])
         + serialization::size(remoteSourceDescriptions[i]) ) );

    passive::Passive & passive(getRuntime().passive());

    passive.iSendTagMessg
      ( iGlobalRank
      , context.rank().get()
      , *descriptionSendBuffers[i] );

    passive.iRecvTagMessg
      ( iGlobalRank
      , i
      , *descriptionRecvBuffers[i] );
  }

  for(int i(0)
     ;    i<context.size().get()
     ;  ++i) {

    descriptionRecvBuffers[i]->waitForNotification();
    {
      char * cPtr (static_cast<char *> (descriptionRecvBuffers[i]->address()));
      cPtr += serialization::deserialize (remoteTargetDescriptions[i], cPtr);
      cPtr += serialization::deserialize (remoteSourceDescriptions[i], cPtr);
    }
    descriptionRecvBuffers[i].reset( nullptr );

    context.write
      ( localSourceDescriptions[i]
      , remoteTargetDescriptions[i] );
  }

  // release notifications of target segment;
  for(int i(0)
     ;    i<context.size().get()
     ;  ++i) {

    context.waitForBufferNotification(localTargetDescriptions[i]);

    context.notify(remoteSourceDescriptions[i]);

    targetSegment.release_notification
      (localTargetDescriptions[i].notificationId());
  }

  for(int i(0)
     ;    i<context.size().get()
     ;  ++i) {

    context.waitForBufferNotification(localSourceDescriptions[i]);

    descriptionSendBuffers[i]->waitForNotification();

    descriptionSendBuffers[i].reset( nullptr );

    sourceSegment.release_notification
          (localSourceDescriptions[i].notificationId());
  }

  context.flush();

}

}
}
