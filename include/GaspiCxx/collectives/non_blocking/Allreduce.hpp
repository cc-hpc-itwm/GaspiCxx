#pragma once

#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

#include <vector>

namespace gaspi
{
  namespace collectives
  {

    template<typename T, AllreduceAlgorithm Algorithm>
    class Allreduce : public Collective
    { 
      public:
        Allreduce(gaspi::segment::Segment& segment,
                  gaspi::group::Group const& group,
                  std::size_t number_elements,
                  ReductionOp reduction_op);

        void start(void const* inputs) override;
        void start(std::vector<T> const& inputs);

        void waitForCompletion(void* outputs) override;
        void waitForCompletion(std::vector<T>& outputs);

      private:
        AllreduceLowLevel<T, Algorithm> allreduce_impl;
    };

    template<typename T, AllreduceAlgorithm Algorithm>
    Allreduce<T, Algorithm>::Allreduce(gaspi::segment::Segment& segment,
                                       gaspi::group::Group const& group,
                                       std::size_t number_elements,
                                       ReductionOp reduction_op)
    : allreduce_impl(segment, group, number_elements, reduction_op)
    {
      allreduce_impl.waitForSetup();
      // TODO here register with the progress engine
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::start(void const* inputs)
    {
      allreduce_impl.copyIn(inputs);
      allreduce_impl.start();
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::start(std::vector<T> const& inputs)
    {
      start(static_cast<void const *>(inputs.data()));
    }

    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::waitForCompletion(void* outputs)
    {
      allreduce_impl.waitForCompletion();
      allreduce_impl.copyOut(outputs);
    }
  
    template<typename T, AllreduceAlgorithm Algorithm>
    void Allreduce<T, Algorithm>::waitForCompletion(std::vector<T>& outputs)
    {
      waitForCompletion(static_cast<void*>(outputs.data()));
    }
  }
}
