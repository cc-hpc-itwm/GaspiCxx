/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019
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
 * BroadcastBasicLinear.hpp
 *
 */

#pragma once

#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>

#include <memory>
#include <stdexcept>

namespace gaspi
{
  namespace collectives
  {
    template<typename T>
    class BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR> : public BroadcastCommon
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
        std::size_t number_ranks;
        gaspi::group::Rank rank;
        gaspi::group::Rank first;
        gaspi::group::Rank rank_after_first;
        gaspi::group::Rank second_last;
        gaspi::group::Rank last;
        gaspi::group::Rank left_neighbour;
        gaspi::group::Rank right_neighbour;

        std::size_t iteration;
        std::size_t last_iteration;
        std::size_t buffer_size_bytes;

        std::unique_ptr<SourceBuffer> source_buffer;
        std::unique_ptr<TargetBuffer> target_buffer;

        std::unique_ptr<ConnectHandle> source_handle;
        std::unique_ptr<ConnectHandle> target_handle;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;
    };

    template<typename T>
    BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>::BroadcastLowLevel(
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      gaspi::group::Rank const& root)
    : BroadcastCommon(group, number_elements, root),
      number_ranks(group.size()),
      rank(group.rank()),
      first(root),
      rank_after_first((number_ranks + rank.get() - first.get()) % number_ranks),
      second_last((number_ranks + first.get() - 2) % number_ranks),
      last((number_ranks + first.get() - 1) % number_ranks),
      left_neighbour((number_ranks + rank.get() - 1) % number_ranks),
      right_neighbour((number_ranks + rank.get() + 1) % number_ranks),
      iteration(0UL),
      last_iteration(number_ranks),
      buffer_size_bytes(sizeof(T) * number_elements),
      source_buffer(),
      target_buffer(),
      source_handle(),
      target_handle()
    {
      if (!group.contains_rank(root))
      {
        throw std::logic_error(
          "BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>: "
          "`group` must contain `root`");
      }

      if (number_elements == 0)
      {
        return;
      }

      if (number_ranks == 1)
      {
        source_buffer = std::make_unique<SourceBuffer>(buffer_size_bytes);
        return;
      }

      if (rank == first)
      {
        SourceBuffer::Tag const tag = right_neighbour.get();
        source_buffer = std::make_unique<SourceBuffer>(buffer_size_bytes);
        source_handle = std::make_unique<ConnectHandle>(
          source_buffer->connectToRemoteTarget(group, right_neighbour, tag));
      }

      if (rank != first)
      {
        TargetBuffer::Tag const tag = rank.get();
        target_buffer = std::make_unique<TargetBuffer>(buffer_size_bytes);
        target_handle = std::make_unique<ConnectHandle>(
          target_buffer->connectToRemoteSource(group, left_neighbour, tag));
      }

      if (rank != first && rank != last)
      {
        SourceBuffer::Tag const tag = right_neighbour.get();
        source_buffer = std::make_unique<SourceBuffer>(*target_buffer);
        source_handle = std::make_unique<ConnectHandle>(
          source_buffer->connectToRemoteTarget(group, right_neighbour, tag));
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>::waitForSetupImpl()
    {
      if (source_handle) source_handle->waitForCompletion();
      if (target_handle) target_handle->waitForCompletion();
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>::startImpl()
    {
      if (number_ranks == 1 || number_elements == 0) return;

      iteration = 0;
      if (rank == first) source_buffer->initTransfer();
      iteration++;
    }

    template<typename T>
    bool BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>::triggerProgressImpl()
    {
      if (number_ranks == 1 || number_elements == 0) return true;

      // Rank with position `iteration` after `first` receives data,
      // and forwards it to the next rank, except if it is `last` rank,
      // in which case, it sends acknowledgement to `second_last` instead
      if (rank_after_first == group::Rank(iteration))
      {
        target_buffer->waitForCompletion();

        if (rank != last)
        {
          source_buffer->initTransfer();
        }
        else
        {
          target_buffer->ackTransfer();
        }
      }

      // Every rank goes to next iteration
      iteration++;

      // After reaching `last_iteration` either wait for
      // acknowledgment (`second_last` rank), or exit
      // unconditionally
      if (iteration == last_iteration)
      {
        if (rank == second_last) source_buffer->waitForTransferAck();
        return true;
      }
      else
      {
        return false;
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>::copyInImpl(void const* inputs)
    {
      if (number_elements == 0) return;

      if (rank == first)
      {
        std::memcpy(source_buffer->address(), inputs, buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::BASIC_LINEAR>::copyOutImpl(void* outputs)
    {
      if (number_elements == 0) return;

      if (rank == first)
      {
        std::memcpy(outputs, source_buffer->address(), buffer_size_bytes);
      }
      else
      {
        std::memcpy(outputs, target_buffer->address(), buffer_size_bytes);
      }
    }
  }
}
