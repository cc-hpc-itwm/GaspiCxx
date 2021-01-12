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
    MakeResources(gaspi::group::Group const& group,
                  std::size_t number_elements,
                  std::size_t segment_size = 1024UL)
    : segment(segment_size),
      allreduce(segment, group, number_elements, ReductionOp::SUM)
    { }

    auto& get_allreduce() { return allreduce; }

  private:
    gaspi::segment::Segment segment;
    Allreduce<int, AllreduceAlgorithm::RING> allreduce;
};

TEST_F(AllreduceNonBlockingTest, start_allreduce)
{
  auto setup = MakeResources(group_all, 0UL);
  auto& allreduce = setup.get_allreduce();
  allreduce.start();

  ASSERT_TRUE(allreduce.is_running() || allreduce.is_finished());
}

TEST_F(AllreduceNonBlockingTest, start_allreduce_twice)
{
  auto setup = MakeResources(group_all, 0UL);
  auto& allreduce = setup.get_allreduce();
  allreduce.start();

  ASSERT_THROW(allreduce.start(), std::logic_error);
}

TEST_F(AllreduceNonBlockingTest, single_element_allreduce)
{
  auto setup = MakeResources(group_all);
  auto& allreduce = setup.get_allreduce();

  std::vector<int> inputs {1};
  std::vector<int> expected {context.size().get()};
  std::vector<int> outputs (1);

  // Copy in
  auto in_ptr = inputs.data();
  auto in_segment_ptr = setup.source_buffer.address();
  std::memcpy(in_segment_ptr, in_ptr, inputs.size() * sizeof(int));

  allreduce.start();
  while(!allreduce.is_finished())
  {
    allreduce.trigger_communication_step();
  }

  // Copy out
  auto out_ptr = outputs.data();
  auto out_segment_ptr = setup.target_buffer.address();
  std::memcpy(out_ptr, out_segment_ptr, outputs.size() * sizeof(int));

  ASSERT_EQ(outputs, expected);
}
}
}