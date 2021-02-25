#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceRing.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    class AllreduceNonBlockingLowLevelTest : public ::testing::Test
    {
      protected:
        AllreduceNonBlockingLowLevelTest()
        : group_all()
        {
          getRuntime().barrier();
        }

        ~AllreduceNonBlockingLowLevelTest()
        {
          getRuntime().barrier();
        }

        gaspi::group::Group const group_all;
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

    TEST_F(AllreduceNonBlockingLowLevelTest, start_empty_allreduce)
    {
      using ElemType = int;
      std::vector<ElemType> inputs;
      auto setup = MakeResources<ElemType, AllreduceAlgorithm::RING>(group_all, 0UL);
      auto& allreduce = setup.get_allreduce();

      allreduce.waitForSetup();
      allreduce.copyIn(inputs.data());

      allreduce.start();
      ASSERT_NO_THROW(allreduce.checkForCompletion());
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

      expected.push_back(elem * group_all.size());

      allreduce.waitForSetup();
      allreduce.copyIn(inputs.data());
      allreduce.start();
      allreduce.waitForCompletion();
      allreduce.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

    TEST_F(AllreduceNonBlockingLowLevelTest, multi_elem_allreduce)
    {
      using ElemType = float;
      std::size_t num_elements = 9;
      auto setup = MakeResources<ElemType, AllreduceAlgorithm::RING>(group_all, num_elements);
      auto& allreduce = setup.get_allreduce();

      std::vector<ElemType> inputs(num_elements);
      std::vector<ElemType> expected(num_elements);
      std::vector<ElemType> outputs(num_elements);

      std::iota(inputs.begin(), inputs.end(), 1);

      auto size = group_all.size();
      std::transform(inputs.begin(), inputs.end(), expected.begin(),
                    [&size](auto elem) { return elem * size; });

      allreduce.waitForSetup();
      allreduce.copyIn(inputs.data());
      allreduce.start();
      allreduce.waitForCompletion();
      allreduce.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }
  }
}
