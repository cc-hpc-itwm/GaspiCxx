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
 * PassiveTest.cpp
 *
 */

#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/utility/serialization.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

namespace gaspi {
namespace passive {

class PassiveTest : public ::testing::Test
{
  protected:

  PassiveTest()
  {
    getRuntime().barrier();
  }

  ~PassiveTest()
  {
    getRuntime().barrier();
  }

};

TEST_F(PassiveTest, BufferDescSerialization)
{
  singlesided::BufferDescription const sdesc
   ( 10
   , 2
   , 3000
   , 20
   , 3 );

  singlesided::BufferDescription ddesc;

  {
    char * cPtr( new char[ serialization::size(sdesc) ]);

    serialization::serialize(cPtr,sdesc);

    serialization::deserialize(ddesc,cPtr);

    delete[] cPtr;
  }

  EXPECT_EQ
    ( sdesc.rank()
    , ddesc.rank() );
  EXPECT_EQ
    ( sdesc.segmentId()
    , ddesc.segmentId() );
  EXPECT_EQ
    ( sdesc.offset()
    , ddesc.offset() );
  EXPECT_EQ
    ( sdesc.size()
    , ddesc.size() );
  EXPECT_EQ
    ( sdesc.notificationId()
    , ddesc.notificationId() );
}

TEST_F(PassiveTest, SendMessg)
{
   Passive & passive(getRuntime().passive());

   if( getRuntime().global_rank() == group::GlobalRank(0) ) {

     int nRecv( getRuntime().size() - 1 );

     for(int iRecv(0)
        ;    iRecv<nRecv
        ;  ++iRecv ) {

       std::vector<char> cData;
       int rank;
       passive.recvMessg
         ( cData
         , rank );

       int & data (*reinterpret_cast<int*>(&cData.front()));

       EXPECT_EQ(data,rank);

     }

   } else {

     int data( getRuntime().global_rank() );

     passive.sendMessg( reinterpret_cast<char*>(&data)
                      , sizeof(int)
                      , 0 );

   }

}

//TEST_F(PassiveTest, WriteTargetBufferRequest)
//{
//   Passive & passive(getRuntime().passive());
//
//   passive.requestWriteTargetBuffer
//     ( 1024
//     , 0 );
//
//}

TEST_F(PassiveTest, iSendRecvTest)
{
   Passive & passive(getRuntime().passive());

   using Buffer = singlesided::Buffer;

   if( getRuntime().global_rank() == group::GlobalRank(0) ) {

     int nRecv( getRuntime().size() - 1 );

     for(int iRecv(0)
        ;    iRecv<nRecv
        ;  ++iRecv ) {

       Buffer rcvBuffer( getRuntime().segment()
                       , sizeof(int) );

       int & data( *reinterpret_cast<int*>(rcvBuffer.address()));

       data = 0;

       passive.iRecvTagMessg
        ( iRecv + 1
        , iRecv + 1
        , rcvBuffer );

       rcvBuffer.waitForNotification();

       EXPECT_EQ(data,iRecv+1);

     }

   } else {

     Buffer srcBuffer( getRuntime().segment()
                     , sizeof(int) );

     int & data( *reinterpret_cast<int*>(srcBuffer.address()));

     data = getRuntime().global_rank();

     passive.iSendTagMessg
       ( 0
       , getRuntime().global_rank()
       , srcBuffer );

     srcBuffer.waitForNotification();

   }

}

} // namespace passive
} // namespace gaspi
