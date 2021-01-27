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

    template<typename T, AllreduceAlgorithm Algorithm>
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
        AllreduceLowLevel<T, Algorithm> allreduce;
    };

    TEST_F(AllreduceNonBlockingLowLevelTest, start_allreduce)
    {
      using ElemType = int;
      auto setup = MakeResources<ElemType, AllreduceAlgorithm::RING>(group_all, 0UL);
      auto& allreduce = setup.get_allreduce();

      std::vector<ElemType> inputs;
      allreduce.setup();
      allreduce.copy_in(inputs.data());

      ASSERT_FALSE(allreduce.checkForCompletion());
      allreduce.start();

      ASSERT_FALSE(allreduce.checkForCompletion());
    }

    TEST_F(AllreduceNonBlockingLowLevelTest, DISABLED_start_allreduce_twice)
    {
      using ElemType = int;
      auto setup = MakeResources<ElemType, AllreduceAlgorithm::RING>(group_all, 0UL);
      auto& allreduce = setup.get_allreduce();

      allreduce.setup();
      allreduce.start();

      ASSERT_THROW(allreduce.start(), std::logic_error);
    }

    TEST_F(AllreduceNonBlockingLowLevelTest, single_element_allreduce)
    {
      using ElemType = int;
      auto setup = MakeResources<ElemType, AllreduceAlgorithm::RING>(group_all, 1);
      auto& allreduce = setup.get_allreduce();

      ElemType elem = 5;
      std::vector<ElemType> inputs {elem};
      std::vector<ElemType> expected;
      std::vector<ElemType> outputs(1);

      auto size = group_all.size().get();
      expected.push_back(elem * size);

      allreduce.setup();
      allreduce.copy_in(inputs.data());
      allreduce.start();
      allreduce.waitForCompletion();
      allreduce.copy_out(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

  }
}