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
#include <GaspiCxx/collectives/Allreduce.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/Allocator.hpp>

extern GlobalTestConfiguration *globalTestConf;

namespace gaspi {
namespace collectives {

template <typename T>
class AllreduceTest : public ::testing::Test
{
  protected:

  AllreduceTest()
  {
    getRuntime().barrier();
  }

  ~AllreduceTest()
  {
    getRuntime().barrier();
  }

};

typedef ::testing::Types
    < int
    , unsigned int
    , long
    , unsigned long
    , float
    , double > TestTypes;

TYPED_TEST_CASE(AllreduceTest, TestTypes);


TYPED_TEST(AllreduceTest, sum)
{
   Context context;


   TypeParam input( context.rank().get() + 1 );


   TypeParam result ( allreduce( input
                               , Allreduce::SUM
                               , context ) );

   TypeParam expect ( (context.size().get()+1)
                    * (context.size().get()+0) / 2 );

   EXPECT_EQ( result, expect );
}

TYPED_TEST(AllreduceTest, min)
{
   Context context;


   TypeParam input( context.rank().get() + 1 );


   TypeParam result ( allreduce( input
                               , Allreduce::MIN
                               , context ) );

   TypeParam expect ( 1 );

   EXPECT_EQ( result, expect );
}

TYPED_TEST(AllreduceTest, max)
{
   Context context;


   TypeParam input( context.rank().get() + 1 );


   TypeParam result ( allreduce( input
                               , Allreduce::MAX
                               , context ) );

   TypeParam expect ( context.size().get() );

   EXPECT_EQ( result, expect );
}

} // namespace passive
} // namespace gaspi
