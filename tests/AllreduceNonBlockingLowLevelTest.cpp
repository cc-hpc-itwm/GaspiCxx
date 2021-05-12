#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceRing.hpp>
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

    TEST_F(AllreduceNonBlockingLowLevelTest, start_empty_allreduce)
    {
      using ElemType = int;
      auto const num_elements = 0UL;
      AllreduceLowLevel<ElemType, AllreduceAlgorithm::RING> allreduce(
        group_all, num_elements, ReductionOp::SUM);

      std::vector<ElemType> inputs {};

      allreduce.waitForSetup();
      allreduce.copyIn(inputs.data());

      allreduce.start();
      ASSERT_NO_THROW(allreduce.checkForCompletion());
    }

    TEST_F(AllreduceNonBlockingLowLevelTest, single_element_allreduce)
    {
      using ElemType = int;
      auto const num_elements = 1UL;
      AllreduceLowLevel<ElemType, AllreduceAlgorithm::RING> allreduce(
        group_all, num_elements, ReductionOp::SUM);

      ElemType const elem = 5;
      std::vector<ElemType> inputs {elem};
      std::vector<ElemType> expected;
      std::vector<ElemType> outputs(num_elements);

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
      auto const num_elements = 9UL;
      AllreduceLowLevel<ElemType, AllreduceAlgorithm::RING> allreduce(
        group_all, num_elements, ReductionOp::SUM);

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
