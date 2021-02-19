#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Broadcast.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastSendToAll.hpp>
#include <GaspiCxx/segment/Segment.hpp>
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

    template<typename T, BroadcastAlgorithm Algorithm>
    class MakeResources
    {
      public:
        MakeResources(gaspi::group::Group const& group,
                      std::size_t number_elements,
                      gaspi::group::Rank const& root,
                      std::size_t segment_size = 1024UL)
        : segment(segment_size),
          broadcast(segment, group, number_elements, root)
        { }

        auto& get_broadcast() { return broadcast; }

      private:
        gaspi::segment::Segment segment;
        Broadcast<T, Algorithm> broadcast;
    };

    TEST_F(BroadcastNonBlockingTest, empty_broadcast)
    {
      using ElemType = int;
      auto const root = gaspi::group::Rank(group_all.size()-1);
      auto setup = MakeResources<ElemType, BroadcastAlgorithm::SEND_TO_ALL>(
        group_all, 0, root);
      auto& broadcast = setup.get_broadcast();

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
      auto const root = gaspi::group::Rank(group_all.size()-1);
      auto setup = MakeResources<ElemType, BroadcastAlgorithm::SEND_TO_ALL>(
        group_all, 1, root);
      auto& broadcast = setup.get_broadcast();

      ElemType elem = 5;
      std::vector<ElemType> const inputs {elem};
      std::vector<ElemType> const expected = inputs;
      std::vector<ElemType> outputs(1);

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
      std::size_t num_elements = 9;

      auto const root = gaspi::group::Rank(group_all.size()-1);
      auto setup = MakeResources<ElemType, BroadcastAlgorithm::SEND_TO_ALL>(
        group_all, num_elements, root);
      auto& broadcast = setup.get_broadcast();

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