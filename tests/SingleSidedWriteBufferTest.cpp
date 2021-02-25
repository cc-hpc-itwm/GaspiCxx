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

#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

class SingleSidedWriteBufferTest : public ::testing::Test
{
  protected:

  std::size_t _segmentSize;
  gaspi::Runtime& context;
  gaspi::group::Group const group;


  SingleSidedWriteBufferTest()
  : _segmentSize(1024*1024)
  , context(getRuntime())
  , group(context.group())
  {

  }

  ~SingleSidedWriteBufferTest()
  { }

};

TEST_F(SingleSidedWriteBufferTest, Connect)
{
  if(context.size() == 1) return;

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
        ( group
        , rightNeighbour
        , tag ).waitForCompletion();


    target.connectToRemoteSource
        ( group
        , leftNeighbour
        , tag ).waitForCompletion();

    isource = 1;

    source.initTransfer(context);
    target.waitForCompletion();

    EXPECT_EQ(itarget,context.size());

  }
  else {

    target.connectToRemoteSource
            ( group
            , leftNeighbour
            , tag ).waitForCompletion();

    source.connectToRemoteTarget
            ( group
            , rightNeighbour
            , tag ).waitForCompletion();

    target.waitForCompletion();

    EXPECT_EQ(itarget,context.rank().get());

    isource = itarget + 1;

    source.initTransfer(context);

  }

  context.barrier();

}

TEST_F(SingleSidedWriteBufferTest, SelfConnect)
{
  group::Rank rank(context.rank());

  int tag(1);

  segment::Segment segment(_segmentSize);

  SourceBuffer source(segment,sizeof(int));
  TargetBuffer target(segment,sizeof(int));

  int & isource ( *reinterpret_cast<int*>(source.address()) ); isource = -1;
  int & itarget ( *reinterpret_cast<int*>(target.address()) ); itarget = -1;

  Endpoint::ConnectHandle tHandle
    ( target.connectToRemoteSource
      ( group
      , rank
      , tag ) );

  Endpoint::ConnectHandle sHandle
    ( source.connectToRemoteTarget
       ( group
       , rank
       , tag ) );

  tHandle.waitForCompletion();
  sHandle.waitForCompletion();
}

TEST_F(SingleSidedWriteBufferTest, ConnectOutOfOrder)
{
  if(context.size() == 1) return;

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
      ( group
      , leftNeighbour
      , tag ) );

  Endpoint::ConnectHandle sHandle
    ( source.connectToRemoteTarget
       ( group
       , rightNeighbour
       , tag ) );

  tHandle.waitForCompletion();
  sHandle.waitForCompletion();

  if( context.rank() == group::Rank(0) ) {

    isource = 1;

    source.initTransfer(context);
    target.waitForCompletion();

    EXPECT_EQ(itarget,context.size());

  }
  else {

    target.waitForCompletion();

    EXPECT_EQ(itarget,context.rank().get());

    isource = itarget + 1;

    source.initTransfer(context);

  }

  context.barrier();

}

} // namespace write
} // namespace singlesided
} // namespace gaspi
