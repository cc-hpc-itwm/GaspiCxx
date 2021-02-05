#pragma once

#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

namespace gaspi
{
  namespace collectives
  {

    template<typename T, AllreduceAlgorithm Algorithm>
    class Allreduce : public Collective, 
                      private AllreduceLowLevel<T, Algorithm>
    { 
      public:
        Allreduce(gaspi::segment::Segment& segment,
                          gaspi::group::Group const& group,
                          std::size_t number_elements,
                          ReductionOp reduction_op)
        : AllreduceLowLevel<T, Algorithm>(segment, group, number_elements, reduction_op)
        {
          waitForSetup();
          // TODO here register with the progress engine
        }

        void start(void* inputs) override
        {
           copyIn(inputs);
           start();
        }

        void waitForCompletion(void* output) override
        {
          waitForCompletion();
          copyOut(output);
        }

      private:
        using AllreduceLowLevel<T, Algorithm>::waitForSetup;
        using AllreduceLowLevel<T, Algorithm>::start;
        using AllreduceLowLevel<T, Algorithm>::waitForCompletion;
        using AllreduceLowLevel<T, Algorithm>::copyIn;
        using AllreduceLowLevel<T, Algorithm>::copyOut;
    };

  
  }
}
