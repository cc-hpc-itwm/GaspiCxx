#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/Barrier.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/Context.hpp>

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {
    namespace blocking {

      class BarrierTest : public ::testing::Test
      {
        protected:
          BarrierTest()
          : group_all()
          {
            getRuntime().barrier();
          }

          ~BarrierTest()
          {
            getRuntime().barrier();
          }

          gaspi::group::Group const group_all;
      };

      TEST_F(BarrierTest, one_barrier)
      {
        gaspi::segment::Segment segment(1024UL);

        Barrier barrier(segment, group_all);
        ASSERT_NO_THROW(barrier.execute());
      }

      TEST_F(BarrierTest, multiple_barriers)
      {
        gaspi::segment::Segment segment(1024UL);
        Barrier barrier(segment, group_all);

        auto num_barrier_calls = 10UL;
        for (auto i = 0UL; i < num_barrier_calls; ++i)
        {
          ASSERT_NO_THROW(barrier.execute());
        }
      }

      namespace
      {
        group::Group generate_group_range(std::size_t start_rank, std::size_t nranks)
        {
          std::vector<group::GlobalRank> global_ranks(nranks);
          std::iota(global_ranks.begin(), global_ranks.end(), start_rank);
          return group::Group(global_ranks);
        }
      }

      TEST_F(BarrierTest, group_subset_ranks)
      {
        gaspi::segment::Segment segment(1024UL);
        auto nranks = group_all.size();

        if (group_all.rank().get() < nranks/3)
        {
          Barrier barrier(segment, generate_group_range(0, nranks/3));
          ASSERT_NO_THROW(barrier.execute());
        }
      }

      TEST_F(BarrierTest, overlapping_groups)
      {
        gaspi::segment::Segment segment(1024UL);
        auto nranks = group_all.size();

        std::vector<std::unique_ptr<Barrier>> barriers;
        if (group_all.rank().get() < nranks/3)
        {
          auto const group = generate_group_range(0, nranks/3);
          barriers.push_back(std::make_unique<Barrier>(segment, group));
        }

        if (group_all.rank().get() < nranks/2)
        {
          auto const group = generate_group_range(0, nranks/2);
          barriers.push_back(std::make_unique<Barrier>(segment, group));
        }

        if (group_all.rank().get() < nranks-1)
        {
          auto const group = generate_group_range(0, nranks-1);
          barriers.push_back(std::make_unique<Barrier>(segment, group));
        }

        auto num_barrier_calls = 5UL;
        for (auto i = 0UL; i < num_barrier_calls; ++i)
        {
          for (auto& barrier : barriers)
          {
            ASSERT_NO_THROW(barrier->execute());
          }
        }

        // FIXME: remove manual cleanup
        // (needed to destroy queues in the right order)
        while(barriers.size() > 0)
        {
          barriers.pop_back();
        }
      }
    }
  }
}
