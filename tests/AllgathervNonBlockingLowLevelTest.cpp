#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervRing.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    class AllgathervNonBlockingLowLevelTest : public ::testing::Test
    {
      protected:
        AllgathervNonBlockingLowLevelTest()
        : group_all()
        {
          getRuntime().barrier();
        }

        ~AllgathervNonBlockingLowLevelTest()
        {
          getRuntime().barrier();
        }

        gaspi::group::Group const group_all;
    };

    TEST_F(AllgathervNonBlockingLowLevelTest, single_element_allgatherv)
    {
      using ElemType = int;
      std::vector<std::size_t> counts(group_all.size(), 1);
      AllgathervLowLevel<ElemType, AllgathervAlgorithm::RING> allgatherv(
        group_all, counts);

      ElemType const elem = group_all.rank().get() + 1;
      std::vector<ElemType> inputs {elem};
      std::vector<ElemType> expected(group_all.size());
      for(auto i = 0UL;i < group_all.size(); ++i)
        expected[i] = i + 1;
      std::vector<ElemType> outputs(group_all.size());

      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

  }
}
