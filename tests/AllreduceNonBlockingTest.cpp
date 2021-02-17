#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Allreduce.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceRing.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    class AllreduceNonBlockingTest : public ::testing::Test
    {
      protected:
        AllreduceNonBlockingTest()
        : group_all()
        {
          getRuntime().barrier();
        }

        ~AllreduceNonBlockingTest()
        {
          getRuntime().barrier();
        }

        gaspi::group::Group const group_all;
    };

    TEST_F(AllreduceNonBlockingTest, empty_allreduce)
    {
      using ElemType = int;
      auto const num_elements = 0UL;
      Allreduce<ElemType, AllreduceAlgorithm::RING> allreduce(
        group_all, num_elements, ReductionOp::SUM);

      std::vector<ElemType> const inputs {};
      std::vector<ElemType> outputs {};

      ASSERT_NO_THROW(allreduce.start(inputs));
      ASSERT_NO_THROW(allreduce.waitForCompletion(outputs));
    }

    TEST_F(AllreduceNonBlockingTest, single_element_allreduce)
    {
      using ElemType = int;
      auto const num_elements = 1UL;
      Allreduce<ElemType, AllreduceAlgorithm::RING> allreduce(
        group_all, num_elements, ReductionOp::SUM);

      ElemType const elem = 5;
      std::vector<ElemType> const inputs {elem};
      std::vector<ElemType> expected;
      std::vector<ElemType> outputs(num_elements);

      expected.push_back(elem * group_all.size());

      allreduce.start(inputs);
      allreduce.waitForCompletion(outputs);

      ASSERT_EQ(outputs, expected);
    }

    TEST_F(AllreduceNonBlockingTest, multi_elem_allreduce)
    {
      using ElemType = float;
      auto const num_elements = 9UL;
      Allreduce<ElemType, AllreduceAlgorithm::RING> allreduce(
        group_all, num_elements, ReductionOp::SUM);

      std::vector<ElemType> inputs(num_elements);
      std::vector<ElemType> expected(num_elements);
      std::vector<ElemType> outputs(num_elements);

      std::iota(inputs.begin(), inputs.end(), 1);

      auto size = group_all.size();
      std::transform(inputs.begin(), inputs.end(), expected.begin(),
                    [&size](auto elem) { return elem * size; });

      allreduce.start(inputs);
      allreduce.waitForCompletion(outputs);

      ASSERT_EQ(outputs, expected);
    }
  }
}
