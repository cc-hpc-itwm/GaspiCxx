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

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervCommon.hpp>
#include <GaspiCxx/group/Utilities.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

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
        std::vector<ConnectHandle> handles;
        std::vector<T> data_for_1rank_case;
        std::size_t current_step;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        bool algorithm_is_finished() const;
        void algorithm_reset_state();
    };

    template<typename T>
    AllgathervLowLevel<T, AllgathervAlgorithm::RING>::AllgathervLowLevel(
                      gaspi::group::Group const& group,
                      std::vector<std::size_t> const& counts)
    : AllgathervCommon(group, counts),
      rank(group.rank()),
      number_ranks(group.size()),
      source_buffers(), target_buffers(),
      handles(),
      data_for_1rank_case(),
      current_step(0)
    {
      if(number_ranks > 1)
      {
        gaspi::group::Rank const left_neighbor(group::decrementRankOnRing(rank, number_ranks));
        gaspi::group::Rank const right_neighbor(group::incrementRankOnRing(rank, number_ranks));
      
        source_buffers.push_back(std::make_unique<SourceBuffer>(counts[rank.get()]*sizeof(T)));

        std::size_t receive_index = group::decrementIndexOnRing(rank.get(), number_ranks);
        for (auto i = 0UL; i < number_ranks - 1; ++i)
        {
          target_buffers.push_back(
              std::make_unique<TargetBuffer>(counts[receive_index]*sizeof(T)));
          receive_index = group::decrementIndexOnRing(receive_index, number_ranks);

          if(i > 0)
          {
            source_buffers.push_back(std::make_unique<SourceBuffer>(*target_buffers[i-1]));
          }
        }

        for (auto i = 0UL; i < number_ranks - 1; ++i)
        {
          SourceBuffer::Tag const source_tag = i;
          TargetBuffer::Tag const target_tag = i;
          handles.push_back(
              source_buffers[i]->connectToRemoteTarget(group, right_neighbor, source_tag));
          handles.push_back(
              target_buffers[i]->connectToRemoteSource(group, left_neighbor, target_tag));
        }
      }
      else
      {
        data_for_1rank_case.resize(number_elements); 
      }
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::waitForSetupImpl()
    {
      for (auto& handle : handles)
      {
        handle.waitForCompletion();
      }
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::startImpl()
    {
      if(number_ranks > 1)
      {
        current_step = 0;
        source_buffers[current_step]->initTransfer();
      }
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::copyInImpl(void const* inputs)
    {
      auto current_begin = static_cast<T const*>(inputs);
      
      if (number_ranks == 1)
      {
        std::copy(current_begin, current_begin + number_elements,
                  data_for_1rank_case.begin());
      }
      else
      {
        auto elements_to_copy = source_buffers[0]->description().size()/sizeof(T);
        auto const current_end = current_begin + elements_to_copy;

        std::copy(current_begin, current_end, static_cast<T*>(source_buffers[0]->address()));
      }
    }

    template<typename T>
    bool AllgathervLowLevel<T, AllgathervAlgorithm::RING>::triggerProgressImpl()
    {
      if (current_step == number_ranks - 1) { return true; }

      target_buffers[current_step]->waitForCompletion();
      current_step++;

      if (current_step == number_ranks - 1)
      {
        target_buffers[current_step-1]->ackTransfer();
        source_buffers[current_step-1]->waitForTransferAck();
        return true;
      }
      else
      {
        source_buffers[current_step]->initTransfer();
      }
      return false;
    }

    template<typename T>
    void AllgathervLowLevel<T, AllgathervAlgorithm::RING>::copyOutImpl(void* outputs)
    {
      auto head = static_cast<T*>(outputs);
      if (number_ranks == 1)
      {
        std::copy(data_for_1rank_case.begin(), data_for_1rank_case.end(),
                  head);
      }
      else
      {
        std::size_t send_index = rank.get();
        std::size_t receive_index = group::decrementIndexOnRing(send_index, number_ranks);
      
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
          receive_index = group::decrementIndexOnRing(receive_index, number_ranks);
        }
      }
    }
  }
}
