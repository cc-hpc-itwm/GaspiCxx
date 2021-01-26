#include <gtest/gtest.h>

#include <GlobalTestConfiguration.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceLowLevel.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/Context.hpp>

#include <cstring>
#include <stdexcept>
#include <vector>

extern GlobalTestConfiguration *globalTestConf;

namespace gaspi {
  namespace collectives {

    class AllreduceNonBlockingLowLevelTest : public ::testing::Test
    {
      protected:
        AllreduceNonBlockingLowLevelTest()
        : group_all(),
          context(group_all)
        {
          getRuntime().barrier();
        }

        ~AllreduceNonBlockingLowLevelTest()
        {
          getRuntime().barrier();
        }

        gaspi::group::Group const group_all;
        gaspi::Context context;
    };

    template<AllreduceAlgorithm Algorithm>
    class MakeResources
    {
      public:
        MakeResources(gaspi::group::Group const& group,
                      std::size_t number_elements,
                      std::size_t segment_size = 1024UL)
        : segment(segment_size),
          allreduce(segment, group, number_elements, ReductionOp::SUM)
        { }

        auto& get_allreduce() { return allreduce; }

      private:
        gaspi::segment::Segment segment;
        AllreduceLowLevel<int, Algorithm> allreduce;
    };

    TEST_F(AllreduceNonBlockingLowLevelTest, start_allreduce)
    {
      auto setup = MakeResources<AllreduceAlgorithm::RING>(group_all, 0UL);
      auto& allreduce = setup.get_allreduce();
      std::vector<int> inputs;
      allreduce.copy_in(inputs.data());
      allreduce.start();

      //ASSERT_TRUE(allreduce.is_running() || allreduce.is_finished());
    }

    // TEST_F(AllreduceNonBlockingLowLevelTest, start_allreduce_twice)
    // {
    //   auto setup = MakeResources(group_all, 0UL);
    //   auto& allreduce = setup.get_allreduce();
    //   std::vector<int> inputs;
    //   allreduce.copy_in(inputs.data());
    //   allreduce.start();

    //   ASSERT_THROW(allreduce.start(), std::logic_error);
    // }

    TEST_F(AllreduceNonBlockingLowLevelTest, single_element_allreduce)
    {
      auto setup = MakeResources(group_all, 1);
      auto& allreduce = setup.get_allreduce();

      int elem = 5;
      std::vector<int> inputs {elem};
      std::vector<int> expected;
      std::vector<int> outputs(1);

      auto size = group_all.size().get();
      expected.push_back(elem * size);

      allreduce.copy_in(inputs.data());
      allreduce.start();
      allreduce.waitForCompletion();
      allreduce.copy_out(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

  }
}