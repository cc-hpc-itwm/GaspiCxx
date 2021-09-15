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
 * Allgatherv.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervCommon.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervRing.hpp>
#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/Runtime.hpp>

#include <memory>
#include <vector>
#include <iostream>

namespace gaspi
{
  namespace collectives
  {

    template<typename T, AllgathervAlgorithm Algorithm>
    class Allgatherv : public Collective
    { 
      public:
        Allgatherv(gaspi::group::Group const& group,
                  std::size_t const count,
                  progress_engine::ProgressEngine& progress_engine);
        Allgatherv(gaspi::group::Group const& group,
                  std::size_t const count);
        ~Allgatherv();

        void start(void const* inputs) override;
        void start(std::vector<T> const& inputs);

        void waitForCompletion(void* outputs) override;
        void waitForCompletion(std::vector<T>& outputs);

      private:
        progress_engine::ProgressEngine& progress_engine;
        progress_engine::ProgressEngine::CollectiveHandle handle;
        std::shared_ptr<AllgathervLowLevel<T, Algorithm>> allgatherv_impl;
    };

    template<typename T, AllgathervAlgorithm Algorithm>
    Allgatherv<T, Algorithm>::Allgatherv(
      gaspi::group::Group const& group,
      std::size_t const count,
      progress_engine::ProgressEngine& progress_engine)
    : progress_engine(progress_engine),
      handle()
    {
      std::vector<std::size_t> counts(group.size(), 0);
      
      std::vector<std::size_t> counter(group.size(), 1);
      AllgathervLowLevel<std::size_t, Algorithm> allgatherv_count(group, counter);
      allgatherv_count.waitForSetup();
      allgatherv_count.copyIn(&count);
      allgatherv_count.start();
      allgatherv_count.waitForCompletion();
      allgatherv_count.copyOut(counts.data());

      allgatherv_impl = std::make_shared<AllgathervLowLevel<T, Algorithm>>(
                     group, counts);
      allgatherv_impl->waitForSetup();
      handle = progress_engine.register_collective(allgatherv_impl);
    }

    template<typename T, AllgathervAlgorithm Algorithm>
    Allgatherv<T, Algorithm>::Allgatherv(
      gaspi::group::Group const& group,
      std::size_t const count)
    : Allgatherv(group, count,
                 gaspi::getRuntime().getDefaultProgressEngine())
    { }

    template<typename T, AllgathervAlgorithm Algorithm>
    Allgatherv<T, Algorithm>::~Allgatherv()
    {
      progress_engine.deregister_collective(handle);
    }

    template<typename T, AllgathervAlgorithm Algorithm>
    void Allgatherv<T, Algorithm>::start(void const* inputs)
    {
      allgatherv_impl->copyIn(inputs);
      allgatherv_impl->start();
    }

    template<typename T, AllgathervAlgorithm Algorithm>
    void Allgatherv<T, Algorithm>::start(std::vector<T> const& inputs)
    {
      start(static_cast<void const *>(inputs.data()));
    }

    template<typename T, AllgathervAlgorithm Algorithm>
    void Allgatherv<T, Algorithm>::waitForCompletion(void* outputs)
    {
      allgatherv_impl->waitForCompletion();
      allgatherv_impl->copyOut(outputs);
    }
  
    template<typename T, AllgathervAlgorithm Algorithm>
    void Allgatherv<T, Algorithm>::waitForCompletion(std::vector<T>& outputs)
    {
      waitForCompletion(static_cast<void*>(outputs.data()));
    }
  }
}