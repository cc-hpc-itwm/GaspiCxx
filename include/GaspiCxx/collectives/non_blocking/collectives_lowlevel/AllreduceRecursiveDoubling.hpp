/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
 *
 * This file is part of GaspiCxx.
 *
 * GaspiCxx is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * GaspiCxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * AllreduceRing.hpp
 *
 */

#pragma once

#include <GaspiCxx/group/Utilities.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    namespace
    {
      template <typename Integer,
                std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
      Integer nearest_power_of_two_less_equal(Integer i)
      {
        auto const exponent = std::trunc(std::log2(i));
        auto const power = std::pow(2, exponent);
        return static_cast<Integer>(power);
      }
    }

    /*
     * RECURSIVE (DISTANCE) DOUBLING
     * =============================
     *
     * Implemented as described Section 2.4 (p.3, cf. also Fig.1) in
     *
     *   Rabenseifner, R. (2004, June).
     *   Optimization of collective reduction operations.
     *   In International Conference on Computational Science (pp. 1-9).
     *   Springer, Berlin, Heidelberg.
     *
     * This algorithms is particularly performant for small message sizes.
     *
     *
     * Description:
     * ------------
     *
     *   The number of ranks used in this algorithm is always the
     *   largest power of two which is smaller or equal to the number
     *   of ranks that started the collective.
     *
     *   Nomenclature:
     *
     *     number ranks       : p
     *     number ranks used  : p' = 2^(floor(log_2 p))
     *     number ranks rest  : r = p - p'
     *     extra ranks        : all ranks with i >= 2r
     *     non-extra ranks    : all ranks with i < 2r
     *     active ranks       : all even non-extra ranks + all extra ranks
     *     non-active ranks   : all odd non-extra ranks
     *
     *   Steps:
     *
     *     1. Non-active ranks send their input vector to their respective active partner
     *        (a.k.a. "even non-extra rank"), who reduces it with its own input vector.
     *     2. In (log p') iterations:
     *        Pairs of ranks separated by a distance that doubles every iteration (start distance: 1),
     *        send their respective current vector to their partner, and reduce the incoming vector
     *        with their own.
     *     3. Final result is send from even non-extra rank back to their respective
     *        non-active partner.
     *
     *   Relabelling:
     *
     *     To find the correct partner ranks in step 2 for the non-power-two case,
     *     the active ranks are (implicitely) relabelled.
     *
     *       even i < 2r:   i |---> i/2
     *       odd  i < 2r:   not used
     *            i >= 2r:  i |---> i-r
     *
     *     The inverse mappings accordingly are
     *
     *       j < r:   j |---> 2*j
     *       j >= r:  j |---> j+r
     *
     *   Example (7 ranks):
     *
     *     p = 7, p' = 4, r = 3, 2r = 6
     *
     *     Relabelling:
     *       old rank   0  1  2  3  4  5  6
     *       new rank   0  -  1  -  2  -  3
     *
     *     Initial state:
     *       rank    0     1     2     3     4     5     6
     *       input  [0]   [1]   [2]   [3]   [4]   [5]   [6]
     *
     *     Iteration 0 (step 1):
     *       rank    0     1     2     3     4     5     6
     *       input [0-1]  [1]  [2-3]  [3]  [4-5]  [5]   [6]
     *
     *     Iteration 1 (step 2):
     *       rank    0     1     2     3     4     5     6
     *       input [0-3]  [1]  [0-3]  [3]  [4-6]  [5]  [4-6]
     *
     *     Iteration 2 (step 2):
     *       rank    0     1     2     3     4     5     6
     *       input [0-6]  [1]  [0-6]  [3]  [0-6]  [5]  [0-6]
     *
     *     Final State (step 3):
     *       rank    0     1     2     3     4     5     6
     *       input [0-6] [0-6] [0-6] [0-6] [0-6] [0-6] [0-6]
     */
    template<typename T>
    class AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING> : public AllreduceCommon
    {
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;
      using ConnectHandle = gaspi::singlesided::Endpoint::ConnectHandle;

      public:
        using AllreduceCommon::AllreduceCommon;

        AllreduceLowLevel(gaspi::group::Group const& group,
                          std::size_t number_elements,
                          ReductionOp reduction_op);

      private:
        enum class AlgStage
        {
          NOT_STARTED,
          WAIT_FOR_DATA,
          WAIT_FOR_ACK,
          INITIAL_STEP_NON_POWER_TWO,
          FINAL_STEP_NON_POWER_TWO,
        };

        std::size_t number_ranks;
        std::size_t number_ranks_used;
        std::size_t number_ranks_rest;
        gaspi::group::Rank rank;

        std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
        std::vector<std::unique_ptr<TargetBuffer>> target_buffers;
        std::unique_ptr<SourceBuffer> source_buffer_non_power_two_case;
        std::unique_ptr<TargetBuffer> target_buffer_non_power_two_case;
        std::size_t size_buffer_bytes;

        std::vector<ConnectHandle> handles;
        std::vector<T> data_for_1rank_case;

        std::size_t iteration;
        std::size_t number_iterations;

        AlgStage alg_stage;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        // algorithm-specific methods
        bool is_extra_rank() const;
        bool is_non_extra_rank() const;
        bool is_even_non_extra_rank() const;
        bool is_odd_non_extra_rank() const;
        bool is_active_rank() const;
        bool is_power_two_case() const;
        bool is_non_power_two_case() const;
        bool is_last_iteration() const;
    };

    template<typename T>
    AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::AllreduceLowLevel(
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      ReductionOp reduction_op)
    : AllreduceCommon(group, number_elements, reduction_op),
      number_ranks(group.size()),
      number_ranks_used(nearest_power_of_two_less_equal(number_ranks)),
      number_ranks_rest(number_ranks - number_ranks_used),
      rank(group.rank()),
      source_buffers(), target_buffers(),
      source_buffer_non_power_two_case(), target_buffer_non_power_two_case(),
      size_buffer_bytes(sizeof(T) * number_elements),
      handles(),
      data_for_1rank_case(),
      iteration(0),
      number_iterations(static_cast<std::size_t>(std::log2(number_ranks_used))),
      alg_stage(AlgStage::NOT_STARTED)
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        if (is_active_rank())
        {
          source_buffers.push_back(std::make_unique<SourceBuffer>(size_buffer_bytes));
          target_buffers.push_back(std::make_unique<TargetBuffer>(size_buffer_bytes));

          for (auto iteration = 1UL; iteration < number_iterations; ++iteration)
          {
            source_buffers.push_back(std::make_unique<SourceBuffer>(*source_buffers.front()));
            target_buffers.push_back(std::make_unique<TargetBuffer>(size_buffer_bytes));
          }

          for (auto iteration = 0UL; iteration < number_iterations; ++iteration)
          {
            auto const distance = static_cast<std::size_t>(std::exp2(iteration));
            auto const relabeled_rank = rank.get() < 2 * number_ranks_rest ?
                                          rank.get() / 2 : rank.get() - number_ranks_rest;
            auto const relabeled_neighbor = relabeled_rank ^ distance; // +/- distance
            auto const neighbor = relabeled_neighbor < number_ranks_rest ?
                                    group::Rank(relabeled_neighbor * 2) :
                                    group::Rank(relabeled_neighbor + number_ranks_rest);
            auto const source_tag = SourceBuffer::Tag(iteration);
            auto const target_tag = TargetBuffer::Tag(iteration);
            handles.push_back(
              source_buffers[iteration]->connectToRemoteTarget(group, neighbor, source_tag));
            handles.push_back(
              target_buffers[iteration]->connectToRemoteSource(group, neighbor, target_tag));
          }
        }

        if (is_non_extra_rank())
        {
          if (is_even_non_extra_rank())
          {
            source_buffer_non_power_two_case = std::make_unique<SourceBuffer>(*source_buffers.back());
          }
          else
          {
            source_buffer_non_power_two_case = std::make_unique<SourceBuffer>(size_buffer_bytes);
          }
          target_buffer_non_power_two_case = std::make_unique<TargetBuffer>(size_buffer_bytes);

          auto const neighbor = group::Rank(rank.get() ^ 1); // next rank when even, previous otherwise
          auto const source_tag = SourceBuffer::Tag(number_iterations);
          auto const target_tag = TargetBuffer::Tag(number_iterations);
          handles.push_back(
            source_buffer_non_power_two_case->connectToRemoteTarget(group, neighbor, source_tag));
          handles.push_back(
            target_buffer_non_power_two_case->connectToRemoteSource(group, neighbor, target_tag));
        }
      }
      else if (number_elements > 0 && number_ranks == 1)
      {
        data_for_1rank_case.resize(number_elements);
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::waitForSetupImpl()
    {
      for (auto& handle : handles)
      {
        handle.waitForCompletion();
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::startImpl()
    {
      if (number_elements == 0 || number_ranks == 1) { return; }

      iteration = 0;
      alg_stage = is_non_extra_rank() ? AlgStage::INITIAL_STEP_NON_POWER_TWO:
                                        AlgStage::WAIT_FOR_DATA;
      if (is_extra_rank())
      {
        source_buffers[iteration]->initTransfer();
      }
      if (is_odd_non_extra_rank())
      {
        source_buffer_non_power_two_case->initTransfer();
      }
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::triggerProgressImpl()
    {
      if (number_elements == 0 || number_ranks == 1) { return true; }

      if (is_even_non_extra_rank() && alg_stage == AlgStage::INITIAL_STEP_NON_POWER_TWO)
      {
        bool made_progress = false;

        // Wait for initial vector from non-active ranks
        made_progress = target_buffer_non_power_two_case->checkForCompletion();
        if (!made_progress) { return false; }

        apply_reduce_op<T>(*source_buffers[iteration], *target_buffer_non_power_two_case);
        source_buffers[iteration]->initTransfer();
        alg_stage = AlgStage::WAIT_FOR_DATA;
      }

      if (alg_stage == AlgStage::FINAL_STEP_NON_POWER_TWO)
      {
        if (is_power_two_case())
        { return true; }

        bool done = true;
        if (is_even_non_extra_rank())
        {
          done = source_buffer_non_power_two_case->checkForTransferAck();
        }
        else if (is_odd_non_extra_rank())
        {
          done = target_buffer_non_power_two_case->checkForCompletion();
          if (done)
          {
            target_buffer_non_power_two_case->ackTransfer();
          }
        }
        return done;
      }

      if (is_active_rank())
      {
        bool made_progress = false;
        if (alg_stage == AlgStage::WAIT_FOR_DATA)
        {
          made_progress = target_buffers[iteration]->checkForCompletion();
        }
        else if (alg_stage == AlgStage::WAIT_FOR_ACK)
        {
          made_progress = source_buffers[iteration]->checkForTransferAck();
        }
        if (!made_progress) { return false; }

        if (alg_stage == AlgStage::WAIT_FOR_DATA)
        {
          // Make sure data has left source_buffers[iteration] before
          // accumulating the data received in target_buffers[iteration],
          // which can only be done with notifications.
          target_buffers[iteration]->ackTransfer();
          alg_stage = AlgStage::WAIT_FOR_ACK;
          return false;
        }
        else
        {
          apply_reduce_op<T>(*source_buffers[iteration], *target_buffers[iteration]);
          if (!is_last_iteration())
          {
            source_buffers[iteration + 1]->initTransfer();
            alg_stage = AlgStage::WAIT_FOR_DATA;
          }
        }
      }

      if (is_last_iteration())
      {
        // Send results back to non-active ranks
        if (is_even_non_extra_rank())
        {
          source_buffer_non_power_two_case->initTransfer();
        }
        alg_stage = AlgStage::FINAL_STEP_NON_POWER_TWO;
      }
      iteration++;
      return false;
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::copyInImpl(void const* inputs)
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        if (is_active_rank())
        {
          std::memcpy(source_buffers.front()->address(), inputs, size_buffer_bytes);
        }
        else
        {
          std::memcpy(source_buffer_non_power_two_case->address(), inputs, size_buffer_bytes);
        }
      }
      else if (number_elements > 0 && number_ranks == 1)
      {
        std::memcpy(data_for_1rank_case.data(), inputs, size_buffer_bytes);
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::copyOutImpl(void* outputs)
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        if (is_active_rank())
        {
          std::memcpy(outputs, source_buffers.back()->address(), size_buffer_bytes);
        }
        else
        {
          std::memcpy(outputs, target_buffer_non_power_two_case->address(), size_buffer_bytes);
        }
      }
      else if (number_elements > 0 && number_ranks == 1)
      {
        std::memcpy(outputs, data_for_1rank_case.data(), size_buffer_bytes);
      }
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_extra_rank() const
    {
      return rank.get() >= 2 * number_ranks_rest;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_non_extra_rank() const
    {
      return !is_extra_rank();
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_even_non_extra_rank() const
    {
      return (rank.get() % 2 == 0) && is_non_extra_rank();
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_odd_non_extra_rank() const
    {
      return (rank.get() % 2 == 1) && is_non_extra_rank();
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_active_rank() const
    {
      return is_even_non_extra_rank() || is_extra_rank();
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_power_two_case() const
    {
      return number_ranks_rest == 0;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_non_power_two_case() const
    {
      return !is_power_two_case();
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RECURSIVE_DOUBLING>::is_last_iteration() const
    {
      return iteration == number_iterations - 1;
    }
  }
}
