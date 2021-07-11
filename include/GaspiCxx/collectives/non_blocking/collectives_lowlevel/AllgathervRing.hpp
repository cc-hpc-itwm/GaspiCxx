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
 * AllgathervRing.hpp
 *
 */

#pragma once

#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervCommon.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    template<typename T>
    class AllgathervLowLevel<T, AllgathervAlgorithm::RING> : public AllgathervCommon
    {
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;
      using ConnectHandle = gaspi::singlesided::Endpoint::ConnectHandle;

      public:
        using AllgathervCommon::AllgathervCommon;

        AllgathervLowLevel(gaspi::group::Group const& group,
                           std::vector<std::size_t> const& counts);
        
      private:
        gaspi::group::Rank rank;
        std::size_t number_ranks;
        std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
        std::vector<std::unique_ptr<TargetBuffer>> target_buffers;
        std::vector<ConnectHandle> source_handles;
        std::vector<ConnectHandle> target_handles;
        
        std::size_t send_buffer_index;
        std::size_t receive_buffer_index;
        std::size_t current_step;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        bool algorithm_is_finished() const;
        void algorithm_reset_state();
        void copy_to_source(SourceBuffer& source_comm, TargetBuffer& target_comm);
    };

    template<typename T>
    AllgathervLowLevel<T, AllgathervAlgorithm::RING>::AllgathervLowLevel(
                      gaspi::group::Group const& group,
                      std::vector<std::size_t> const& counts)
    : AllgathervCommon(group, counts),
      rank(group.rank()),
      number_ranks(group.size()),
      source_buffers(), target_buffers(),
      source_handles(), target_handles(),
      current_step(0)
    {
      auto const left_neighbor = (rank - 1 + number_ranks) % number_ranks;
      auto const right_neighbor = (rank + 1) % number_ranks;
      
      std::size_t send_index = rank.get();
      std::size_t receive_index = (number_ranks + send_index - 1) % number_ranks;

      for (auto i = 0UL; i < number_ranks - 1; ++i)
      {
        source_buffers.push_back(
            std::make_unique<SourceBuffer>(counts[send_index]*sizeof(T)));
        target_buffers.push_back(
            std::make_unique<TargetBuffer>(counts[receive_index]*sizeof(T)));

        SourceBuffer::Tag source_tag = i;
        TargetBuffer::Tag target_tag = i;
        source_handles.push_back(
            source_buffers[i]->connectToRemoteTarget(group, right_neighbor, source_tag));
        target_handles.push_back(
            target_buffers[i]->connectToRemoteSource(group, left_neighbor, target_tag));
        
        send_index = (send_index - 1) % number_ranks;
        receive_index = (receive_index - 1) % number_ranks;
      }

    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::waitForSetupImpl()
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
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::algorithm_reset_state()
    {
      current_step = 0;
    }

    template<typename T>
    bool AllgathervLowLevel<T, AllgathervAlgorithm::RING>::algorithm_is_finished() const
    {
      return (current_step == number_ranks - 1);
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::startImpl()
    {
      algorithm_reset_state();
      source_buffers[current_step]->initTransfer();
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::copyInImpl(void const* inputs)
    {
      auto current_begin = static_cast<T const*>(inputs);
      
      auto& buffer = source_buffers[current_step];
      auto elements_to_copy = buffer->description().size()/sizeof(T);
      auto const current_end = current_begin + elements_to_copy;

      std::copy(current_begin, current_end, static_cast<T*>(buffer->address()));
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::copy_to_source(
              SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
      auto const target_begin = static_cast<T*>(target_comm.address());
      auto const target_end = target_begin +
                              source_comm.description().size()/sizeof(T);
      auto const source_begin = static_cast<T*>(source_comm.address());
      std::copy(target_begin, target_end, source_begin);
    }

    template<typename T>
    bool AllgathervLowLevel<T, AllgathervAlgorithm::RING>::triggerProgressImpl()
    {
      if (algorithm_is_finished()) { return true; }

      target_buffers[current_step]->waitForCompletion();

      current_step++;

      if (algorithm_is_finished())
      {
        target_buffers[current_step-1]->ackTransfer();
        source_buffers[current_step-1]->waitForTransferAck();
      }
      else
      {
        copy_to_source(*source_buffers[current_step], *target_buffers[current_step - 1]);
        source_buffers[current_step]->initTransfer();
      }

      return algorithm_is_finished();
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::copyOutImpl(void* outputs)
    {
      std::size_t send_index = rank.get();
      std::size_t receive_index = (number_ranks + send_index - 1) % number_ranks;
      
      auto head = static_cast<T*>(outputs);
      auto current_begin = head + offsets[send_index];
      auto source_begin = static_cast<T*>(source_buffers[0]->address());
      auto source_end = source_begin + counts[send_index];
      std::copy(source_begin, source_end, current_begin);

      for (auto i = 0UL; i < number_ranks - 1; ++i)
      {
        current_begin = head + offsets[receive_index];
        source_begin = static_cast<T*>(target_buffers[i]->address());
        source_end = source_begin + counts[receive_index];
        std::copy(source_begin, source_end, current_begin);
        receive_index = (receive_index - 1) % number_ranks;
      }
    }
  }
}