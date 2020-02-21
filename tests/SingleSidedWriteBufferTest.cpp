/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiCommLayer.
 * 
 * GaspiCommLayer is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 * 
 * GaspiCommLayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GaspiCommLayer. If not, see <http://www.gnu.org/licenses/>.
 */


#include <gtest/gtest.h>

#include <GlobalTestConfiguration.hpp>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>

extern GlobalTestConfiguration *globalTestConf;

namespace gaspi {
namespace singlesided {
namespace write {

class SingleSidedWriteBufferTest : public ::testing::Test
{
  protected:

  std::size_t _segmentSize;


  SingleSidedWriteBufferTest()
  : _segmentSize(1024*1024)
  {

  }

  ~SingleSidedWriteBufferTest()
  { }

};

TEST_F(SingleSidedWriteBufferTest, Connect)
{
  Context context;

  if(context.size().get() == 1) return;

  group::Rank rightNeighbour( ( context.rank()
                              + context.size()
                              + 1 ) % context.size() );

  group::Rank leftNeighbour ( ( context.rank()
                              + context.size()
                              - 1 ) % context.size() );

  int tag(1);

  segment::Segment segment(_segmentSize);

  SourceBuffer source(segment,sizeof(int));
  TargetBuffer target(segment,sizeof(int));

  int & isource ( *reinterpret_cast<int*>(source.address()) ); isource = -1;
  int & itarget ( *reinterpret_cast<int*>(target.address()) ); itarget = -1;

  if( context.rank() == group::Rank(0) ) {

    source.connectToRemoteTarget
        ( context
        , rightNeighbour
        , tag ).waitForCompletion();


    target.connectToRemoteSource
        ( context
        , leftNeighbour
        , tag ).waitForCompletion();

    isource = 1;

    source.initTransfer( context );
    target.waitForCompletion();

    EXPECT_EQ(itarget,context.size().get());

  }
  else {

    target.connectToRemoteSource
            ( context
            , leftNeighbour
            , tag ).waitForCompletion();

    source.connectToRemoteTarget
            ( context
            , rightNeighbour
            , tag ).waitForCompletion();

    target.waitForCompletion();

    EXPECT_EQ(itarget,context.rank().get());

    isource = itarget + 1;

    source.initTransfer( context );

  }

  context.barrier();

}

TEST_F(SingleSidedWriteBufferTest, SelfConnect)
{
  Context context;

  group::Rank rank(context.rank());

  int tag(1);

  segment::Segment segment(_segmentSize);

  SourceBuffer source(segment,sizeof(int));
  TargetBuffer target(segment,sizeof(int));

  int & isource ( *reinterpret_cast<int*>(source.address()) ); isource = -1;
  int & itarget ( *reinterpret_cast<int*>(target.address()) ); itarget = -1;

  Endpoint::ConnectHandle tHandle
    ( target.connectToRemoteSource
      ( context
      , rank
      , tag ) );

  Endpoint::ConnectHandle sHandle
    ( source.connectToRemoteTarget
       ( context
       , rank
      , tag ) );

  tHandle.waitForCompletion();
  sHandle.waitForCompletion();
}

TEST_F(SingleSidedWriteBufferTest, ConnectOutOfOrder)
{
  Context context;

  if(context.size().get() == 1) return;

  group::Rank rightNeighbour( ( context.rank()
                              + context.size()
                              + 1 ) % context.size() );

  group::Rank leftNeighbour ( ( context.rank()
                              + context.size()
                              - 1 ) % context.size() );

  int tag(1);

  segment::Segment segment(_segmentSize);

  SourceBuffer source(segment,sizeof(int));
  TargetBuffer target(segment,sizeof(int));

  int & isource ( *reinterpret_cast<int*>(source.address()) ); isource = -1;
  int & itarget ( *reinterpret_cast<int*>(target.address()) ); itarget = -1;

  Endpoint::ConnectHandle tHandle
    ( target.connectToRemoteSource
      ( context
      , leftNeighbour
      , tag ) );

  Endpoint::ConnectHandle sHandle
    ( source.connectToRemoteTarget
       ( context
       , rightNeighbour
      , tag ) );

  tHandle.waitForCompletion();
  sHandle.waitForCompletion();

  if( context.rank() == group::Rank(0) ) {

    isource = 1;

    source.initTransfer( context );
    target.waitForCompletion();

    EXPECT_EQ(itarget,context.size().get());

  }
  else {

    target.waitForCompletion();

    EXPECT_EQ(itarget,context.rank().get());

    isource = itarget + 1;

    source.initTransfer( context );

  }

  context.barrier();

}

} // namespace write
} // namespace singlesided
} // namespace gaspi
