#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervRing.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <numeric>
#include <algorithm> 
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    class AllgathervNonBlockingLowLevelTest : public ::testing::Test
    {
      protected:
        using ElemType = int;

        AllgathervNonBlockingLowLevelTest()
        : group_all(),
          elem(group_all.rank().get() + 1)
        {
          getRuntime().barrier();
        }

        ~AllgathervNonBlockingLowLevelTest()
        {
          getRuntime().barrier();
        }

        auto fill_expect_data(std::vector<std::size_t> const& counts)
        {
          std::vector<std::size_t> offsets(counts.size(), 0);
          if(offsets.size() > 1)
          {
            std::partial_sum(counts.begin(), counts.end() - 1, 
                             offsets.begin() + 1);
          }
          auto output_size = std::accumulate(counts.begin(), counts.end(), 0);
          std::vector<ElemType> expected(output_size);

          for(auto i = 0UL; i < counts.size(); ++i)
          {
            auto head = expected.begin() + offsets[i];
            std::fill(head, head + counts[i], i + 1);
          }
          
          return expected;
        }

        gaspi::group::Group const group_all;
        ElemType elem;
    };

    TEST_F(AllgathervNonBlockingLowLevelTest, single_element_allgatherv)
    {
      std::vector<std::size_t> counts(group_all.size(), 1);

      AllgathervLowLevel<ElemType, AllgathervAlgorithm::RING> allgatherv(
        group_all, counts);
        
      auto rank = group_all.rank().get();
      std::vector<ElemType> inputs(counts[rank], elem);
      std::vector<ElemType> expected = fill_expect_data(counts);
      std::vector<ElemType> outputs(expected.size(), 0);


      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }
    
    TEST_F(AllgathervNonBlockingLowLevelTest, multiple_different_element_allgatherv)
    {
      std::vector<std::size_t> counts(group_all.size());

      //rank i has count i + 1;
      for(auto i = 0UL;i < counts.size(); ++i)
      {
        counts[i] = i + 1;
      }

      AllgathervLowLevel<ElemType, AllgathervAlgorithm::RING> allgatherv(
        group_all, counts);

      auto rank = group_all.rank().get();
      std::vector<ElemType> inputs(counts[rank], elem);
      std::vector<ElemType> expected = fill_expect_data(counts);
      std::vector<ElemType> outputs(expected.size(), 0);

      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

    TEST_F(AllgathervNonBlockingLowLevelTest, zero_element_allgatherv)
    {
      std::vector<std::size_t> counts(group_all.size(), 0);

      //Even rank has count 0, odd rank i has count i + 1
      for(auto i = 0UL;i < group_all.size(); ++i)
      {
        if(i % 2)
        {
          counts[i] = i + 1;
        }
      }
      
      auto rank = group_all.rank().get();
      std::vector<ElemType> inputs(counts[rank], elem);
      std::vector<ElemType> expected = fill_expect_data(counts);
      std::vector<ElemType> outputs(expected.size(), 0);
      

      AllgathervLowLevel<ElemType, AllgathervAlgorithm::RING> allgatherv(
        group_all, counts);

      allgatherv.waitForSetup();
      allgatherv.copyIn(inputs.data());
      allgatherv.start();
      allgatherv.waitForCompletion();
      allgatherv.copyOut(outputs.data());

      ASSERT_EQ(outputs, expected);
    }

  }
}
