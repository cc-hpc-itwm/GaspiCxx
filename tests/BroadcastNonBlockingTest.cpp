#include <gtest/gtest.h>

#include <GlobalTestConfiguration.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Broadcast.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastSendToAll.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <numeric>
#include <stdexcept>
#include <vector>

extern GlobalTestConfiguration *globalTestConf;

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
      std::vector<ElemType> inputs;
      std::vector<ElemType> outputs;

      if (group_all.rank() == root)
      {
        ASSERT_NO_THROW(broadcast.start(inputs.data()));
      }
      else
      {
        ASSERT_NO_THROW(broadcast.start());
      }
      ASSERT_NO_THROW(broadcast.waitForCompletion(outputs.data()));
    }

    // TEST_F(BroadcastNonBlockingTest, single_element_allreduce)
    // {
    //   using ElemType = int;
    //   auto setup = MakeResources<ElemType, BroadcastAlgorithm::RING>(group_all, 1);
    //   auto& allreduce = setup.get_allreduce();

    //   ElemType elem = 5;
    //   std::vector<ElemType> inputs {elem};
    //   std::vector<ElemType> expected;
    //   std::vector<ElemType> outputs(1);

    //   expected.push_back(elem * group_all.size());

    //   allreduce.start(inputs.data());
    //   allreduce.waitForCompletion(outputs.data());

    //   ASSERT_EQ(outputs, expected);
    // }

    // TEST_F(BroadcastNonBlockingTest, multi_elem_allreduce)
    // {
    //   using ElemType = float;
    //   std::size_t num_elements = 9;
    //   auto setup = MakeResources<ElemType, BroadcastAlgorithm::RING>(group_all, num_elements);
    //   auto& allreduce = setup.get_allreduce();

    //   std::vector<ElemType> inputs(num_elements);
    //   std::vector<ElemType> expected(num_elements);
    //   std::vector<ElemType> outputs(num_elements);

    //   std::iota(inputs.begin(), inputs.end(), 1);

    //   auto size = group_all.size();
    //   std::transform(inputs.begin(), inputs.end(), expected.begin(),
    //                 [&size](auto elem) { return elem * size; });

    //   allreduce.start(inputs.data());
    //   allreduce.waitForCompletion(outputs.data());

    //   ASSERT_EQ(outputs, expected);
    // }
  }
}