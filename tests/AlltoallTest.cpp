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

#include <GaspiCxx/Runtime.hpp>
#include <GlobalTestConfiguration.hpp>
#include <GaspiCxx/collectives/Allgather.hpp>
#include <GaspiCxx/collectives/Alltoall.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/utility/serialization.hpp>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

extern GlobalTestConfiguration *globalTestConf;

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
   Context context;

   segment::Segment sourceSegment(1024);
   segment::Segment targetSegment(1024);

   std::size_t size(sizeof(int));

   char * gSource ( sourceSegment.allocator()
                      .allocate(size * context.size().get() ) );
   char * gTarget ( targetSegment.allocator()
                      .allocate(size * context.size().get() ) );

   for( int i(0)
      ;     i<context.size().get()
      ;   ++i ) {
     *(reinterpret_cast<int*>(gSource)+i) = context.rank().get()
                                          * context.size().get()
                                          + i;
   }

   alltoall( gSource
           , sourceSegment
           , gTarget
           , targetSegment
           , size
           , context );

   for( int i(0)
     ;     i<context.size().get()
     ;   ++i ) {
     EXPECT_EQ( *(reinterpret_cast<int*>(gTarget)+i)
              , i
              * context.size().get()
              + context.rank().get() );
   }

   sourceSegment.allocator()
       .deallocate( gSource, size * context.size().get() );
   targetSegment.allocator()
       .deallocate( gTarget, size * context.size().get() );

}

TEST_F(AlltoallTest, alltoallLoop)
{
   Context context;

   segment::Segment sourceSegment(1024);
   segment::Segment targetSegment(1024);

   std::size_t size(sizeof(int));

   char * gSource ( sourceSegment.allocator()
                     .allocate(size * context.size().get() ) );
   char * gTarget ( targetSegment.allocator()
                     .allocate(size * context.size().get() ) );

   int const nLoop(10);
   for(int iLoop(0);iLoop<nLoop;++iLoop) {

     for( int i(0)
           ;     i<context.size().get()
           ;   ++i ) {
       *(reinterpret_cast<int*>(gSource)+i) = ( context.rank().get()
                                              * context.size().get()
                                              + i ) * iLoop;
     }

     alltoall( gSource
             , sourceSegment
             , gTarget
             , targetSegment
             , size
             , context );

     for( int i(0)
        ;     i<context.size().get()
        ;   ++i ) {
        EXPECT_EQ( *(reinterpret_cast<int*>(gTarget)+i)
                 , ( i
                   * context.size().get()
                   + context.rank().get() ) * iLoop );
      }
   }

   sourceSegment.allocator()
      .deallocate( gSource, size * context.size().get() );
   targetSegment.allocator()
      .deallocate( gTarget, size * context.size().get() );

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
   Context context;

   segment::Segment sourceSegment(1024);
   segment::Segment targetSegment(1024);

   std::size_t elementSize(sizeof(int));
   std::size_t sourceSize(elementSize * (context.size().get()+1)
                                      * (context.size().get()+0) / 2);
   std::size_t targetSize(elementSize * (context.size().get()+1)
                                      * (context.size().get()+0) / 2);
   std::size_t * sourceSizes(new std::size_t[context.size().get()]);
   std::size_t * targetSizes(new std::size_t[context.size().get()]);

   char * gSource ( sourceSegment.allocator().allocate(sourceSize) );
   char * gTarget ( targetSegment.allocator().allocate(targetSize) );

   int k(0);
   for( int i(0)
      ;     i<context.size().get()
      ;   ++i ) {
     group::Rank rightNeighbour( ( context.rank().get()
                                 + group::Rank(i).get()
                                 + context.size().get()
                                 + 1 ) % context.size().get() );

     group::Rank leftNeighbour ( ( context.rank().get()
                                 + group::Rank(i).get()
                                 + context.size().get()
                                 - 1 ) % context.size().get() );

     for( int j(0)
        ;     j<rightNeighbour.get()+1
        ;   ++j ) {
       reinterpret_cast<int*>(gSource)[k] = (rightNeighbour.get()+1)
                                          * (rightNeighbour.get()+0) / 2
                                          + j + 1;
       ++k;
     }

     sourceSizes[i] = (rightNeighbour.get() + 1) * elementSize;
     targetSizes[i] = (rightNeighbour.get()  + 1) * elementSize;
   }

   alltoallv( gSource
            , sourceSegment
            , sourceSizes
            , gTarget
            , targetSegment
            , targetSizes
            , context );

   for( int i(0)
      ;     i< (context.size().get()+1)
             * (context.size().get()+0) / 2
      ;   ++i) {
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
