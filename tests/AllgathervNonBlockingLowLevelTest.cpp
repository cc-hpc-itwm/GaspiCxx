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
      std::vector<ElemType> outputs(group_all.size());
      for(auto i = 0UL;i < group_all.size(); ++i)
      {
        expected[i] = i + 1;
      }

      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }
    
    TEST_F(AllgathervNonBlockingLowLevelTest, multiple_different_element_allgatherv)
    {
      using ElemType = int;
      std::vector<std::size_t> counts(group_all.size());
      for(auto i = 0UL;i < group_all.size(); ++i)
      {
        counts[i] = i + 1;
      }

      AllgathervLowLevel<ElemType, AllgathervAlgorithm::RING> allgatherv(
        group_all, counts);

      ElemType const elem = group_all.rank().get() + 1;
      auto output_size = std::accumulate(counts.begin(), counts.end(), 0);
      std::vector<ElemType> inputs(elem, elem);
      std::vector<ElemType> expected(output_size);
      
      auto index = 0UL;
      for(auto i = 1UL;i <= group_all.size(); ++i)
      {
        for(auto j = i;j > 0; --j)
        {
          expected[index++] = i;
        }
      }
      
      std::vector<ElemType> outputs(output_size);

      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

    TEST_F(AllgathervNonBlockingLowLevelTest, zero_element_allgatherv)
    {
      using ElemType = int;
      std::vector<std::size_t> counts(group_all.size(), 0);
      for(auto i = 0UL;i < group_all.size(); ++i)
      {
        if(i % 2)
        {
          counts[i] = i + 1;
        }
      }

      ElemType const elem = group_all.rank().get() + 1;
      std::vector<ElemType> inputs;
      auto count = counts[group_all.rank().get()];
      if(count > 0)
      {
        for(auto i = 0UL;i < count;++i)
        {
          inputs.push_back(elem);;
        }
      }

      auto output_size = std::accumulate(counts.begin(), counts.end(), 0);

      std::vector<ElemType> expected(output_size);
      
      auto index = 0;
      for(auto i = 0UL;i < counts.size(); ++i)
      {
        for(auto j = 0UL; j < counts[i]; ++j)
        {
          expected[index++] = i + 1;
        }
      }

      AllgathervLowLevel<ElemType, AllgathervAlgorithm::RING> allgatherv(
        group_all, counts);
      
      std::vector<ElemType> outputs(output_size);

      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

  }
}
