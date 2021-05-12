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

#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
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

        gaspi::group::Rank rank;
        std::size_t number_ranks;
        std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
        std::vector<std::unique_ptr<TargetBuffer>> target_buffers;
        std::vector<ConnectHandle> source_handles;
        std::vector<ConnectHandle> target_handles;
        std::vector<T> data_for_1rank_case;
      
        std::size_t current_step;
        std::size_t steps_per_stage;
        std::size_t send_buffer_index;
        std::size_t receive_buffer_index;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        // algorithm-specific methods
        auto algorithm_get_current_stage() const;
        bool algorithm_is_finished() const;
        void algorithm_reset_state();
        void apply_reduce_op(SourceBuffer& source_comm, TargetBuffer& target_comm);
        void copy_to_source(SourceBuffer& source_comm, TargetBuffer& target_comm);
    };

    template<typename T>
    AllreduceLowLevel<T, AllreduceAlgorithm::RING>::AllreduceLowLevel(
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      ReductionOp reduction_op)
    : AllreduceCommon(group, number_elements, reduction_op),
      rank(group.rank()),
      number_ranks(group.size()),
      source_buffers(), target_buffers(),
      source_handles(), target_handles(),
      data_for_1rank_case(),
      steps_per_stage(group.size()-1)
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        auto const left_neighbor = (rank - 1 + number_ranks) % number_ranks;
        auto const right_neighbor = (rank + 1) % number_ranks;

        auto const number_elements_padded = number_elements / number_ranks +
                                            (number_elements % number_ranks != 0 ? 1 : 0);
        auto const size_padded = sizeof(T) * number_elements_padded;

        for (auto i = 0UL; i < number_ranks; ++i)
        {
          source_buffers.push_back(
            std::make_unique<SourceBuffer>(size_padded));
          target_buffers.push_back(
            std::make_unique<TargetBuffer>(size_padded));
        }

        for (auto i = 0UL; i < number_ranks; ++i)
        {
          SourceBuffer::Tag source_tag = i;
          TargetBuffer::Tag target_tag = i;
          source_handles.push_back(
            source_buffers[i]->connectToRemoteTarget(group, left_neighbor, source_tag));
          target_handles.push_back(
            target_buffers[i]->connectToRemoteSource(group, right_neighbor, target_tag));
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
      for (auto& handle : source_handles)
      {
        handle.waitForCompletion();
      }
      for (auto& handle : target_handles)
      {
        handle.waitForCompletion();
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::startImpl()
    {
      if (number_elements > 0 && number_ranks > 1)
      {
        algorithm_reset_state();
        send_buffer_index = (send_buffer_index + 1) % number_ranks;
        source_buffers[send_buffer_index]->initTransfer();
      }
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::triggerProgressImpl()
    {
      if (number_elements == 0 || number_ranks == 1) { return true; }

      if (algorithm_is_finished()) { return true; }

      receive_buffer_index = (receive_buffer_index + 1) % number_ranks;
      target_buffers[receive_buffer_index]->waitForCompletion();

      switch(algorithm_get_current_stage())
      {
        case RingStage::REDUCE:
        {
          apply_reduce_op(*source_buffers[receive_buffer_index], *target_buffers[receive_buffer_index]);
          break;
        }
        case RingStage::GATHER:
        {
          copy_to_source(*source_buffers[receive_buffer_index], *target_buffers[receive_buffer_index]);
          break;
        }
      }

      current_step++;

      // acknowledge data transfer in the last step
      if (algorithm_is_finished())
      {
        target_buffers[receive_buffer_index]->ackTransfer();
        source_buffers[send_buffer_index]->waitForTransferAck();
      }
      else
      {
        send_buffer_index = (send_buffer_index + 1) % number_ranks;
        source_buffers[send_buffer_index]->initTransfer();
      }
      return algorithm_is_finished();
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
        for (auto& buffer : source_buffers)
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
        for (auto& buffer : source_buffers)
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
      return ((steps_per_stage > 0) && (current_step / steps_per_stage == 0))
             ? RingStage::REDUCE : RingStage::GATHER;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::algorithm_is_finished() const
    {
      // check whether the algorithm reached the final step of the second stage
      return (algorithm_get_current_stage() == RingStage::GATHER &&
              current_step == 2 * steps_per_stage);
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::algorithm_reset_state()
    {
      current_step = 0;
      receive_buffer_index = rank.get();
      send_buffer_index = (number_ranks + receive_buffer_index - 1) % number_ranks;
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::apply_reduce_op(
              SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
      auto const source_begin = static_cast<T*>(source_comm.address());
      auto const source_end = source_begin +
                              source_comm.description().size()/sizeof(T);
      auto const target_begin = static_cast<T*>(target_comm.address());
      std::transform(source_begin, source_end, target_begin,
                     source_begin, std::plus<T>());
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::copy_to_source(
              SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
      auto const target_begin = static_cast<T*>(target_comm.address());
      auto const target_end = target_begin +
                              source_comm.description().size()/sizeof(T);
      auto const source_begin = static_cast<T*>(source_comm.address());
      std::copy(target_begin, target_end, source_begin);
    }
  }
}
