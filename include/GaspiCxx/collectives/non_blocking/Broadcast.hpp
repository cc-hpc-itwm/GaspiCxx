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
 * Broadcast.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>
#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastSendToAll.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastBasicLinear.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {

    template<typename T, BroadcastAlgorithm Algorithm>
    class Broadcast : public RootedSendCollective
    { 
      public:
        Broadcast(gaspi::group::Group const& group,
                  std::size_t number_elements,
                  gaspi::group::Rank const& root_rank,
                  gaspi::progress_engine::ProgressEngine& progress_engine);
        Broadcast(gaspi::group::Group const& group,
                  std::size_t number_elements,
                  gaspi::group::Rank const& root_rank);
        ~Broadcast();

        void start(void const* inputs) override;
        void start(std::vector<T> const& inputs);
        void start() override;

        void waitForCompletion(void* output) override;
        void waitForCompletion(std::vector<T>& output);
        std::vector<T> waitForCompletion();

      private:
        progress_engine::ProgressEngine& progress_engine;
        progress_engine::ProgressEngine::CollectiveHandle handle;
        std::shared_ptr<BroadcastLowLevel<T, Algorithm>> broadcast_impl;

        gaspi::group::Rank root_rank;
        gaspi::group::Rank rank;
    };

    template<typename T, BroadcastAlgorithm Algorithm>
    Broadcast<T, Algorithm>::Broadcast(
      gaspi::group::Group const& group,
      std::size_t number_elements,
      gaspi::group::Rank const& root_rank,
      gaspi::progress_engine::ProgressEngine& progress_engine)
    : progress_engine(progress_engine),
      handle(),
      broadcast_impl(std::make_shared<BroadcastLowLevel<T, Algorithm>>(
        group, number_elements, root_rank)),
      root_rank(root_rank),
      rank(group.rank())
    {
      broadcast_impl->waitForSetup();
      handle = progress_engine.register_collective(broadcast_impl);
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    Broadcast<T, Algorithm>::Broadcast(
      gaspi::group::Group const& group,
      std::size_t number_elements,
      gaspi::group::Rank const& root_rank)
    : Broadcast(group, number_elements, root_rank,
                gaspi::getRuntime().getDefaultProgressEngine())
    { }

    template<typename T, BroadcastAlgorithm Algorithm>
    Broadcast<T, Algorithm>::~Broadcast()
    {
      progress_engine.deregister_collective(handle);
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    void Broadcast<T, Algorithm>::start(void const* inputs)
    {
      if(rank != root_rank)
      {
        throw std::logic_error(
          "Broadcast: start(void const* inputs) may only be called on root rank.");
      }
      broadcast_impl->copyIn(inputs);
      broadcast_impl->start();
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    void Broadcast<T, Algorithm>::start(std::vector<T> const& inputs)
    {
      start(static_cast<void const *>(inputs.data()));
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    void Broadcast<T, Algorithm>::start()
    {
      if(rank == root_rank)
      {
        throw std::logic_error("Broadcast: start() may only be called on non-root ranks.");
      }
      broadcast_impl->copyIn(CollectiveLowLevel::NO_DATA);
      broadcast_impl->start();
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    void Broadcast<T, Algorithm>::waitForCompletion(void* output)
    {
      broadcast_impl->waitForCompletion();
      broadcast_impl->copyOut(output);
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    void Broadcast<T, Algorithm>::waitForCompletion(std::vector<T>& output)
    {
      waitForCompletion(static_cast<void*>(output.data()));
    }

    template<typename T, BroadcastAlgorithm Algorithm>
    std::vector<T> Broadcast<T, Algorithm>::waitForCompletion()
    {
      std::vector<T> output(broadcast_impl->get_output_count());
      waitForCompletion(static_cast<void*>(output.data()));
      return output;
    }
  }
}
