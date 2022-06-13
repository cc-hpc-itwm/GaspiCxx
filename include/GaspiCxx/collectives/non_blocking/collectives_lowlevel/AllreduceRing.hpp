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
#include <memory>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    namespace
    {
      template <typename Integer,
                std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
      Integer ceil_div(Integer a, Integer b)
      {
        return (a + b - 1) / b;
      }
    }

    template<typename T>
    class AllreduceLowLevel<T, AllreduceAlgorithm::RING> : public AllreduceCommon
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
        enum class RingStage
        {
          REDUCE,
          GATHER
        };

        std::size_t number_ranks;
        gaspi::group::Rank rank;
        gaspi::group::Rank left_neighbor;
        gaspi::group::Rank right_neighbor;
        std::vector<std::unique_ptr<SourceBuffer>> source_buffers_reduce;
        std::vector<std::unique_ptr<TargetBuffer>> target_buffers_reduce;
        std::vector<std::unique_ptr<SourceBuffer>> source_buffers_gather;
        std::vector<std::unique_ptr<TargetBuffer>> target_buffers_gather;
        std::vector<ConnectHandle> handles;
        std::vector<T> data_for_1rank_case;
      
        std::size_t current_step;
        std::size_t steps_per_stage;
        gaspi::group::RingIndex current_index;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        // algorithm-specific methods
        auto algorithm_get_current_stage() const;
        bool is_last_step_reduce() const;
        bool is_last_step_gather() const;
        bool is_last_step() const;
    };

    template<typename T>
    AllreduceLowLevel<T, AllreduceAlgorithm::RING>::AllreduceLowLevel(
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      ReductionOp reduction_op)
    : AllreduceCommon(group, number_elements, reduction_op),
      number_ranks(group.size()),
      rank(group.rank()),
      left_neighbor(group::decrementRankOnRing(rank, number_ranks)),
      right_neighbor(group::incrementRankOnRing(rank, number_ranks)),
      source_buffers_reduce(), target_buffers_reduce(), source_buffers_gather(), target_buffers_gather(),
      handles(),
      data_for_1rank_case(),
      current_step(0),
      steps_per_stage(number_ranks-1),
      current_index(rank.get(), number_ranks)
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        auto const number_elements_padded = ceil_div(number_elements, number_ranks);
        auto const size_padded = sizeof(T) * number_elements_padded;

        for (auto i = 0UL; i < number_ranks; ++i)
        {
          source_buffers_reduce.push_back(
            std::make_unique<SourceBuffer>(size_padded));
          target_buffers_reduce.push_back(
            std::make_unique<TargetBuffer>(size_padded));
        }

        // Make sends in gather phase in-place
        for (auto i = 0UL; i < number_ranks; ++i)
        {
          source_buffers_gather.push_back(std::make_unique<SourceBuffer>(*source_buffers_reduce[i]));
          target_buffers_gather.push_back(std::make_unique<TargetBuffer>(*source_buffers_reduce[i]));
        }

        for (auto i = 0UL; i < number_ranks; ++i)
        {
          auto const source_tag_reduce = static_cast<SourceBuffer::Tag>(i);
          auto const target_tag_reduce = static_cast<TargetBuffer::Tag>(i);
          auto const source_tag_gather = static_cast<SourceBuffer::Tag>(i + number_ranks);
          auto const target_tag_gather = static_cast<TargetBuffer::Tag>(i + number_ranks);

          handles.push_back(
            source_buffers_reduce[i]->connectToRemoteTarget(group, right_neighbor, source_tag_reduce));
          handles.push_back(
            target_buffers_reduce[i]->connectToRemoteSource(group, left_neighbor, target_tag_reduce));
          handles.push_back(
            source_buffers_gather[i]->connectToRemoteTarget(group, right_neighbor, source_tag_gather));
          handles.push_back(
            target_buffers_gather[i]->connectToRemoteSource(group, left_neighbor, target_tag_gather));
        }
      }
      else
      {
        data_for_1rank_case.resize(number_elements);
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::waitForSetupImpl()
    {
      for (auto& handle : handles)
      {
        handle.waitForCompletion();
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::startImpl()
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        current_step = 0;
        current_index = group::RingIndex(rank.get(), number_ranks);
        source_buffers_reduce[current_index]->initTransfer();
        current_index--;
      }
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::triggerProgressImpl()
    {
      if (number_elements == 0 || number_ranks == 1) { return true; }

      auto made_progress = false;
      if (is_last_step()) // wait for final transfer acknowledgements
      {
        auto index = group::RingIndex(current_index + 2, number_ranks);
        made_progress = source_buffers_gather[index]->checkForTransferAck();
        return made_progress;
      }

      // each algorithm stage starts by waiting for data to arrive
      switch(algorithm_get_current_stage())
      {
        case RingStage::REDUCE:
        {
          made_progress = target_buffers_reduce[current_index]->checkForCompletion();
          break;
        }
        case RingStage::GATHER:
        {
          made_progress = target_buffers_gather[current_index]->checkForCompletion();
        }
      }
      if (!made_progress) { return false; }

      // when data transfer for the current stage had ended, start transfers
      // for the next iteration
      switch(algorithm_get_current_stage())
      {
        case RingStage::REDUCE:
        {
          apply_reduce_op<T>(*source_buffers_reduce[current_index], *target_buffers_reduce[current_index]);
          if (!is_last_step_reduce())
          {
            source_buffers_reduce[current_index]->initTransfer();
          }
          else
          {
            source_buffers_gather[current_index]->initTransfer();
          }
          break;
        }
        case RingStage::GATHER:
        {
          if (!is_last_step_gather())
          {
            source_buffers_gather[current_index]->initTransfer();
          }
          else
          {
            target_buffers_gather[current_index]->ackTransfer();
          }
        }
      }

      current_step++;
      current_index--;
      return false;
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::copyInImpl(void const* inputs)
    {
      auto total_copied_elements = 0UL;
      auto current_begin = static_cast<T const*>(inputs);

      if (number_ranks == 1)
      {
        std::copy(current_begin, current_begin + number_elements,
                  data_for_1rank_case.begin());
      }
      else
      {
        for (auto& buffer : source_buffers_reduce)
        {
          auto elements_to_copy = buffer->description().size()/sizeof(T);
          if (total_copied_elements + elements_to_copy > number_elements)
          {
            elements_to_copy = number_elements - total_copied_elements;
          }

          auto const current_end = current_begin + elements_to_copy;
          std::copy(current_begin, current_end, static_cast<T*>(buffer->address()));
          current_begin = current_end;
          total_copied_elements += elements_to_copy;

          if (total_copied_elements >= number_elements)
          {
            break;
          }
        }
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::copyOutImpl(void* outputs)
    {
      auto total_copied_elements = 0UL;
      auto current_start = static_cast<T*>(outputs);

      if (number_ranks == 1)
      {
        std::copy(data_for_1rank_case.begin(), data_for_1rank_case.end(),
                  current_start);
      }
      else
      {
        for (auto& buffer : source_buffers_gather)
        {
          auto elements_to_copy = buffer->description().size()/sizeof(T);
          if (total_copied_elements + elements_to_copy > number_elements)
          {
            elements_to_copy = number_elements - total_copied_elements;
          }

          auto const buffer_start = static_cast<T*>(buffer->address());
          auto const buffer_end = buffer_start + elements_to_copy;
          std::copy(buffer_start, buffer_end, current_start);
          current_start = current_start + elements_to_copy;
          total_copied_elements += elements_to_copy;

          if (total_copied_elements >= number_elements)
          {
            break;
          }
        }
      }
    }

    template<typename T>
    auto AllreduceLowLevel<T, AllreduceAlgorithm::RING>::algorithm_get_current_stage() const
    {
      return current_step / steps_per_stage == 0 ? RingStage::REDUCE : RingStage::GATHER;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::is_last_step_reduce() const
    {
      return current_step == steps_per_stage - 1;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::is_last_step_gather() const
    {
      return current_step == 2 * steps_per_stage - 1;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::is_last_step() const
    {
      return current_step == 2 * steps_per_stage;
    }
  }
}
