#include <gtest/gtest.h>

#include <GlobalTestConfiguration.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Allreduce.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/Context.hpp>

#include <cstring>
#include <stdexcept>
#include <vector>

extern GlobalTestConfiguration *globalTestConf;

namespace gaspi {
  namespace collectives {

    class AllreduceNonBlockingTest : public ::testing::Test
    {
      protected:
        AllreduceNonBlockingTest()
        : context(),
          group_all(context.group())
        {
          getRuntime().barrier();
        }

        ~AllreduceNonBlockingTest()
        {
          getRuntime().barrier();
        }

        gaspi::Context context;
        gaspi::group::Group const& group_all;
    };

    class MakeResources
    {
      public:
        MakeResources(gaspi::Context& context,
                      std::size_t number_elements,
                      std::size_t segment_size = 1024UL)
        : segment(segment_size),
          allreduce(segment, context, number_elements, ReductionOp::SUM)
        { }

        auto& get_allreduce() { return allreduce; }

      private:
        gaspi::segment::Segment segment;
        Allreduce<int, AllreduceAlgorithm::RING> allreduce;
    };

    TEST_F(AllreduceNonBlockingTest, start_allreduce)
    {
      auto setup = MakeResources(context, 0UL);
      auto& allreduce = setup.get_allreduce();
      std::vector<int> inputs;
      allreduce.start(inputs.data());

      ASSERT_TRUE(allreduce.is_running() || allreduce.is_finished());
    }

    TEST_F(AllreduceNonBlockingTest, start_allreduce_twice)
    {
      auto setup = MakeResources(context, 0UL);
      auto& allreduce = setup.get_allreduce();
      std::vector<int> inputs;
      allreduce.start(inputs.data());

      ASSERT_THROW(allreduce.start(inputs.data()), std::logic_error);
    }

    TEST_F(AllreduceNonBlockingTest, single_element_allreduce)
    {
      auto setup = MakeResources(context, 1);
      auto& allreduce = setup.get_allreduce();

      int elem = 5;
      std::vector<int> inputs {elem};
      std::vector<int> expected;
      std::vector<int> outputs(1);

      auto size = context.size().get();
      expected.push_back(elem * size);

      allreduce.start(inputs.data());
      while(!allreduce.is_finished())
      {
        allreduce.trigger_communication_step();
      }
      allreduce.reset_and_retrieve(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

    TEST_F(AllreduceNonBlockingTest, multi_elem_allreduce)
    {
      std::size_t num_elements = 9;
      auto setup = MakeResources(context, num_elements);
      auto& allreduce = setup.get_allreduce();

      std::vector<int> inputs(num_elements);
      std::vector<int> expected(num_elements);
      std::vector<int> outputs(num_elements);

      // fill in input buffer
      std::iota(inputs.begin(), inputs.end(), 1);

      auto size = context.size().get();
      std::transform(inputs.begin(), inputs.end(), expected.begin(),
                    [&size](int elem) { return elem * size; });

      allreduce.start(inputs.data());
      while(!allreduce.is_finished())
      {
        allreduce.trigger_communication_step();
      }
      allreduce.reset_and_retrieve(outputs.data());

      ASSERT_EQ(outputs, expected);
    }
  }
}