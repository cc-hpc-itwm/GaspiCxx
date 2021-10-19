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
 * Allreduce.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceRing.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceRecursiveDoubling.hpp>
#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/Runtime.hpp>

#include <memory>
#include <vector>

namespace gaspi
{
  namespace collectives
  {

    template<typename T, AllreduceAlgorithm Algorithm>
    class Allreduce : public Collective
    { 
      public:
        Allreduce(gaspi::group::Group const& group,
                  std::size_t number_elements,
                  ReductionOp reduction_op,
                  progress_engine::ProgressEngine& progress_engine);
        Allreduce(gaspi::group::Group const& group,
                  std::size_t number_elements,
                  ReductionOp reduction_op);
        ~Allreduce();

        void start(void const* inputs) override;
        void start(std::vector<T> const& inputs);

        void waitForCompletion(void* outputs) override;
        void waitForCompletion(std::vector<T>& outputs);
        std::vector<T> waitForCompletion();

      private:
        progress_engine::ProgressEngine& progress_engine;
        progress_engine::ProgressEngine::CollectiveHandle handle;
        std::shared_ptr<AllreduceLowLevel<T, Algorithm>> allreduce_impl;
    };

    template<typename T, AllreduceAlgorithm Algorithm>
    Allreduce<T, Algorithm>::Allreduce(
      gaspi::group::Group const& group,
      std::size_t number_elements,
      ReductionOp reduction_op,
      progress_engine::ProgressEngine& progress_engine)
    : progress_engine(progress_engine),
      handle(),
      allreduce_impl(std::make_shared<AllreduceLowLevel<T, Algorithm>>(
                     group, number_elements, reduction_op))
    {
      allreduce_impl->waitForSetup();
      handle = progress_engine.register_collective(allreduce_impl);
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    Allreduce<T, Algorithm>::Allreduce(
      gaspi::group::Group const& group,
      std::size_t number_elements,
      ReductionOp reduction_op)
    : Allreduce(group, number_elements, reduction_op,
                gaspi::getRuntime().getDefaultProgressEngine())
    { }

    template<typename T, AllreduceAlgorithm Algorithm>
    Allreduce<T, Algorithm>::~Allreduce()
    {
      progress_engine.deregister_collective(handle);
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::start(void const* inputs)
    {
      allreduce_impl->copyIn(inputs);
      allreduce_impl->start();
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::start(std::vector<T> const& inputs)
    {
      start(static_cast<void const *>(inputs.data()));
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::waitForCompletion(void* outputs)
    {
      allreduce_impl->waitForCompletion();
      allreduce_impl->copyOut(outputs);
    }
  
    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::waitForCompletion(std::vector<T>& outputs)
    {
      waitForCompletion(static_cast<void*>(outputs.data()));
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    std::vector<T> Allreduce<T, Algorithm>::waitForCompletion()
    {
      std::vector<T> outputs(allreduce_impl->get_output_count());
      waitForCompletion(static_cast<void*>(outputs.data()));
      return outputs;
    }
  }
}
