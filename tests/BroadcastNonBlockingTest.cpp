#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Broadcast.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastSendToAll.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastBasicLinear.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    class BroadcastNonBlockingTest : public ::testing::Test
    {
      protected:
        BroadcastNonBlockingTest()
        : group_all()
        {
          getRuntime().barrier();
        }

        ~BroadcastNonBlockingTest()
        {
          getRuntime().barrier();
        }

        gaspi::group::Group const group_all;
    };

    TEST_F(BroadcastNonBlockingTest, empty_broadcast)
    {
      using ElemType = int;
      auto const num_elements = 0UL;
      auto const root = gaspi::group::Rank(group_all.size()-1);
      Broadcast<ElemType, BroadcastAlgorithm::BASIC_LINEAR> broadcast(
        group_all, num_elements, root);

      std::vector<ElemType> const inputs {};
      std::vector<ElemType> outputs {};

      if (group_all.rank() == root)
      {
        ASSERT_NO_THROW(broadcast.start(inputs));
      }
      else
      {
        ASSERT_NO_THROW(broadcast.start());
      }
      ASSERT_NO_THROW(broadcast.waitForCompletion(outputs));
    }

    TEST_F(BroadcastNonBlockingTest, single_element_allreduce)
    {
      using ElemType = int;
      auto const num_elements = 1UL;
      auto const root = gaspi::group::Rank(group_all.size()-1);
      Broadcast<ElemType, BroadcastAlgorithm::BASIC_LINEAR> broadcast(
        group_all, num_elements, root);

      ElemType const elem = 5;
      std::vector<ElemType> const inputs {elem};
      std::vector<ElemType> const expected = inputs;
      std::vector<ElemType> outputs(num_elements);

      if (group_all.rank() == root)
      {
        broadcast.start(inputs);
      }
      else
      {
        broadcast.start();
      }
      broadcast.waitForCompletion(outputs);

      ASSERT_EQ(outputs, expected);
    }

    TEST_F(BroadcastNonBlockingTest, multi_elem_allreduce)
    {
      using ElemType = float;
      auto const num_elements = 9UL;
      auto const root = gaspi::group::Rank(group_all.size()-1);
      Broadcast<ElemType, BroadcastAlgorithm::BASIC_LINEAR> broadcast(
        group_all, num_elements, root);

      std::vector<ElemType> inputs(num_elements);
      std::iota(inputs.begin(), inputs.end(), 42);
      std::vector<ElemType> const expected = inputs;
      std::vector<ElemType> outputs(num_elements);

      if (group_all.rank() == root)
      {
        broadcast.start(inputs);
      }
      else
      {
        broadcast.start();
      }
      broadcast.waitForCompletion(outputs);


      ASSERT_EQ(outputs, expected);
    }
  }
}
