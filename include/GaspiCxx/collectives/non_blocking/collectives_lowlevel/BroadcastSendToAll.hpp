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
 * BroadcastSendToAll.hpp
 *
 */

#pragma once

#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>

#include <algorithm>
#include <cstring>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    template<typename T>
    class BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL> : public BroadcastCommon
    {
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;
      using ConnectHandle = gaspi::singlesided::Endpoint::ConnectHandle;

      public:
        using BroadcastCommon::BroadcastCommon;

        BroadcastLowLevel(gaspi::group::Group const& group,
                          std::size_t number_elements,
                          gaspi::group::Rank const& root);

      private:
        gaspi::group::Rank rank;
        std::size_t number_ranks;
        std::size_t buffer_size_bytes;

        std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
        std::unique_ptr<TargetBuffer> target_buffer;

        std::vector<ConnectHandle> source_handles;
        std::unique_ptr<ConnectHandle> target_handle;
        std::size_t transfers_in_progress;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        // implementation details
        void check_root_is_in_group() const;
        std::vector<gaspi::group::Rank> generate_non_root_ranks() const;
        void create_and_connect_first_source_buffer(std::size_t,
                                                    gaspi::group::Rank const&);
        void create_and_connect_remaining_source_buffers(
          std::vector<gaspi::group::Rank> const&);
        void create_and_connect_target_buffer(std::size_t);
    };

    template<typename T>
    BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::BroadcastLowLevel(
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      gaspi::group::Rank const& root)
    : BroadcastCommon(group, number_elements, root),
      rank(group.rank()),
      number_ranks(group.size()),
      buffer_size_bytes(sizeof(T) * number_elements),
      source_buffers(),
      target_buffer(),
      source_handles(),
      target_handle()
    {
      check_root_is_in_group();

      if (number_ranks == 1)
      {
        // construct memory to save inputs
        source_buffers.push_back(
            std::make_unique<SourceBuffer>(buffer_size_bytes));
        return;
      }

      if (rank == root)
      {
        auto non_root_ranks = generate_non_root_ranks();
        create_and_connect_first_source_buffer(buffer_size_bytes,
                                               non_root_ranks.back());
        non_root_ranks.pop_back();
        create_and_connect_remaining_source_buffers(non_root_ranks);
      }
      else
      {
        create_and_connect_target_buffer(buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::waitForSetupImpl()
    {
      if (number_ranks == 1) return;

      if (rank == root)
      {
        for (auto& source_handle : source_handles)
        {
          source_handle.waitForCompletion();
        }
      }
      else
      {
        target_handle->waitForCompletion();
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::startImpl()
    {
      if (number_ranks == 1) return;

      if (rank == root)
      {
        for (auto& source_buffer : source_buffers)
        {
          source_buffer->initTransfer();
        }
        transfers_in_progress = source_buffers.size();
      }
    }

    template<typename T>
    bool BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::triggerProgressImpl()
    {
      if (number_ranks == 1) return true;

      if (rank == root)
      {
        for (auto& source_buffer : source_buffers)
        {
          auto done = source_buffer->checkForTransferAck();
          if (done) transfers_in_progress--;
        }
        if (transfers_in_progress == 0) { return true; }
      }
      else
      {
        auto done = target_buffer->checkForCompletion();
        if (done)
        {
          target_buffer->ackTransfer();
          return true;
        }
      }
      return false;
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::copyInImpl(void const* inputs)
    {
      if (rank == root)
      {
        std::memcpy(source_buffers.front()->address(), inputs, buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::copyOutImpl(void* outputs)
    {
      if (rank == root)
      {
        std::memcpy(outputs, source_buffers.front()->address(), buffer_size_bytes);
      }
      else
      {
        std::memcpy(outputs, target_buffer->address(), buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::check_root_is_in_group() const
    {
      if (root >= gaspi::group::Rank(number_ranks))
      {
        throw std::logic_error(
          "BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>: "
          "`group` must contain `root`");
      }
    }

    template<typename T>
    std::vector<gaspi::group::Rank>
    BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::generate_non_root_ranks() const
    {
      std::vector<gaspi::group::Rank> ranks;
      for (auto i = 0UL; i < number_ranks; ++i)
      {
        if(root != gaspi::group::Rank(i))
        {
          ranks.push_back(gaspi::group::Rank(i));
        }
      }
      return ranks;
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::create_and_connect_first_source_buffer(
                      std::size_t size_bytes,
                      gaspi::group::Rank const& other_rank)
    {
      SourceBuffer::Tag const tag = other_rank.get();
      source_buffers.push_back(
          std::make_unique<SourceBuffer>(size_bytes));
      source_handles.push_back(
          source_buffers.front()->connectToRemoteTarget(
              group, other_rank, tag));
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::create_and_connect_remaining_source_buffers(
        std::vector<gaspi::group::Rank> const& ranks)
    {
      for (auto const& rank : ranks)
      {
        SourceBuffer::Tag const tag = rank.get();
        source_buffers.push_back(
            std::make_unique<SourceBuffer>(*source_buffers.front()));
        source_handles.push_back(
            source_buffers.back()->connectToRemoteTarget(group, rank, tag));
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::create_and_connect_target_buffer(std::size_t size_bytes)
    {
      TargetBuffer::Tag const tag = rank.get();
      target_buffer = std::make_unique<TargetBuffer>(size_bytes);
      target_handle = std::make_unique<ConnectHandle>(
        target_buffer->connectToRemoteSource(group, root, tag));
    }
  }
}
