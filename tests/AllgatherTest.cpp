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
 * AllgatherTest.cpp
 *
 */

#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/Allgather.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/utility/serialization.hpp>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

namespace gaspi {
namespace collectives {

class AllgatherTest : public ::testing::Test
{
  protected:

  AllgatherTest()
  {
    getRuntime().barrier();
  }

  ~AllgatherTest()
  {
    getRuntime().barrier();
  }

};


TEST_F(AllgatherTest, allgather)
{
  group::Group group{}; // all ranks

  segment::Segment sourceSegment(1024);
  segment::Segment targetSegment(1024);

  std::size_t elementSize(sizeof(int));
  std::size_t sourceSize(elementSize * 1 );
  std::size_t targetSize(elementSize * group.size());

  char * gSource ( sourceSegment.allocator().allocate(sourceSize) );
  char * gTarget ( targetSegment.allocator().allocate(targetSize) );

  *reinterpret_cast<int*>(gSource) = group.rank().get();

  allgather( gSource
          , sourceSegment
          , gTarget
          , targetSegment
          , elementSize
          , group
          , getRuntime() );

  for( auto i(0UL)
    ;      i<group.size()
    ;    ++i) {
    EXPECT_EQ( reinterpret_cast<int*>(gTarget)[i]
            , i );
  }

  sourceSegment.allocator().deallocate( gSource, sourceSize );
  targetSegment.allocator().deallocate( gTarget, targetSize );
}

TEST_F(AllgatherTest, allgatherLoop)
{
  group::Group group{}; // all ranks

  segment::Segment sourceSegment(1024);
  segment::Segment targetSegment(1024);

  std::size_t elementSize(sizeof(int));
  std::size_t sourceSize(elementSize * 1 );
  std::size_t targetSize(elementSize * group.size());

  char * gSource ( sourceSegment.allocator().allocate(sourceSize) );
  char * gTarget ( targetSegment.allocator().allocate(targetSize) );

  int const nLoop(10);
  for(int iLoop(0)
    ;    iLoop<nLoop
    ;  ++iLoop ) {

    *reinterpret_cast<int*>(gSource) = group.rank().get() * iLoop;

    allgather( gSource
            , sourceSegment
            , gTarget
            , targetSegment
            , elementSize
            , group
            , getRuntime() );

    for(auto i(0UL)
      ;     i<group.size()
      ;   ++i ) {
      EXPECT_EQ( reinterpret_cast<int*>(gTarget)[i]
              , i * iLoop );
    }

  }

  sourceSegment.allocator().deallocate( gSource, sourceSize );
  targetSegment.allocator().deallocate( gTarget, targetSize );

}

TEST_F(AllgatherTest, allgatherv)
{
  // source buffer:
  // rank 0: |1|
  // rank 1: |2,3|
  // rank 2: |4,5,6| ...
  //
  // target buffer:
  // all ranks: |1,2,3,4,...,(nProc+1)*nProc/2|
  group::Group group{}; // all ranks

  segment::Segment sourceSegment(1024);
  segment::Segment targetSegment(1024);

  std::size_t elementSize(sizeof(int));
  std::size_t sourceSize(elementSize * (group.rank().get()+1) );
  std::size_t targetSize(elementSize * (group.size()+1)
                                    * (group.size()+0) / 2);
  std::size_t * sizes(new std::size_t[group.size()]);

  char * gSource ( sourceSegment.allocator().allocate(sourceSize) );
  char * gTarget ( targetSegment.allocator().allocate(targetSize) );

  for( auto i(0)
    ;      i<group.rank().get()+1
    ;    ++i ) {
    reinterpret_cast<int*>(gSource)[i] = (group.rank().get()+1)
                                      * (group.rank().get()+0) / 2
                                      + i + 1;
  }

  for( auto i(0UL)
    ;      i<group.size() 
    ;    ++i ) {
    sizes[i] = (i + 1) * elementSize;
  }

  allgatherv( gSource
            , sourceSegment
            , gTarget
            , targetSegment
            , sizes
            , group
            , getRuntime() );

  for( auto i(0UL)
    ;      i< (group.size()+1)
            * (group.size()+0) / 2
    ;    ++i) {
    EXPECT_EQ( reinterpret_cast<int*>(gTarget)[i]
            , i+1 );
  }

  delete[] sizes;
  sourceSegment.allocator().deallocate( gSource, sourceSize );
  targetSegment.allocator().deallocate( gTarget, targetSize );

}

} // namespace passive
} // namespace gaspi
