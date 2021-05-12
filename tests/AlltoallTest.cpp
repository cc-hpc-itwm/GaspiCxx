/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
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
 * AlltoallTest.cpp
 *
 */

#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/Allgather.hpp>
#include <GaspiCxx/collectives/Alltoall.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/utility/serialization.hpp>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

namespace gaspi {
namespace collectives {

class AlltoallTest : public ::testing::Test
{
  protected:

  AlltoallTest()
  {
    getRuntime().barrier();
  }

  ~AlltoallTest()
  {
    getRuntime().barrier();
  }

};


TEST_F(AlltoallTest, alltoall)
{
  group::Group group{}; //all ranks

  segment::Segment sourceSegment(1024);
  segment::Segment targetSegment(1024);

  std::size_t size(sizeof(int));

  char * gSource ( sourceSegment.allocator()
                    .allocate(size * group.size() ) );
  char * gTarget ( targetSegment.allocator()
                    .allocate(size * group.size() ) );

  for( auto i(0UL)
    ;      i<group.size()
    ;    ++i ) {
    *(reinterpret_cast<int*>(gSource)+i) = group.rank().get()
                                        * group.size()
                                        + i;
  }

  alltoall( gSource
          , sourceSegment
          , gTarget
          , targetSegment
          , size
          , group
          , getRuntime() );

  for( auto i(0UL)
    ;      i<group.size()
    ;    ++i ) {
    EXPECT_EQ( *(reinterpret_cast<int*>(gTarget)+i)
            , i
            * group.size()
            + group.rank().get() );
  }

  sourceSegment.allocator()
      .deallocate( gSource, size * group.size() );
  targetSegment.allocator()
      .deallocate( gTarget, size * group.size() );

}

TEST_F(AlltoallTest, alltoallLoop)
{
  group::Group group{}; //all ranks

  segment::Segment sourceSegment(1024);
  segment::Segment targetSegment(1024);

  std::size_t size(sizeof(int));

  char * gSource ( sourceSegment.allocator()
                    .allocate(size * group.size() ) );
  char * gTarget ( targetSegment.allocator()
                    .allocate(size * group.size() ) );

  int const nLoop(10);
  for(int iLoop(0);iLoop<nLoop;++iLoop) {

    for( auto i(0UL)
      ;      i<group.size()
      ;   ++i ) {
      *(reinterpret_cast<int*>(gSource)+i) = ( group.rank().get()
                                            * group.size()
                                            + i ) * iLoop;
    }

    alltoall( gSource
            , sourceSegment
            , gTarget
            , targetSegment
            , size
            , group
            , getRuntime() );

    for( auto i(0UL)
      ;      i<group.size()
      ;    ++i ) {
      EXPECT_EQ( *(reinterpret_cast<int*>(gTarget)+i)
                , ( i
                  * group.size()
                  + group.rank().get() ) * iLoop );
    }
  }

  sourceSegment.allocator()
    .deallocate( gSource, size * group.size() );
  targetSegment.allocator()
    .deallocate( gTarget, size * group.size() );

}

TEST_F(AlltoallTest, alltoallv)
{
  // source buffer:
  // rank 0: |2,3,4,...,1|
  // rank 1: |4,5,6,...,3|
  // rank 2: |7,8,9,...,6| ...
  //
  // target buffer:
  // all ranks: target buffer = source buffer
  group::Group group{}; //all ranks

  segment::Segment sourceSegment(1024);
  segment::Segment targetSegment(1024);

  std::size_t elementSize(sizeof(int));
  std::size_t sourceSize(elementSize * (group.size()+1)
                                    * (group.size()+0) / 2);
  std::size_t targetSize(elementSize * (group.size()+1)
                                    * (group.size()+0) / 2);
  std::size_t * sourceSizes(new std::size_t[group.size()]);
  std::size_t * targetSizes(new std::size_t[group.size()]);

  char * gSource ( sourceSegment.allocator().allocate(sourceSize) );
  char * gTarget ( targetSegment.allocator().allocate(targetSize) );

  int k(0);
  for( auto i(0UL)
    ;      i<group.size()
    ;    ++i ) {
    group::Rank rightNeighbour( ( group.rank().get()
                                + i
                                + group.size()
                                + 1 ) % group.size() );

    for( int j(0)
      ;     j<rightNeighbour.get()+1
      ;   ++j ) {
      reinterpret_cast<int*>(gSource)[k] = (rightNeighbour.get()+1)
                                        * (rightNeighbour.get()+0) / 2
                                        + j + 1;
      ++k;
    }

    sourceSizes[i] = (rightNeighbour.get() + 1) * elementSize;
    targetSizes[i] = (rightNeighbour.get() + 1) * elementSize;
  }

  alltoallv( gSource
          , sourceSegment
          , sourceSizes
          , gTarget
          , targetSegment
          , targetSizes
          , group
          , getRuntime() );

  for( auto i(0UL)
    ;      i< (group.size()+1)
            * (group.size()+0) / 2
    ;    ++i) {
    EXPECT_EQ( reinterpret_cast<int*>(gTarget)[i]
            , reinterpret_cast<int*>(gSource)[i] );
  }

  delete[] sourceSizes;
  delete[] targetSizes;
  sourceSegment.allocator().deallocate( gSource, sourceSize );
  targetSegment.allocator().deallocate( gTarget, targetSize );

}

} // namespace passive
} // namespace gaspi
