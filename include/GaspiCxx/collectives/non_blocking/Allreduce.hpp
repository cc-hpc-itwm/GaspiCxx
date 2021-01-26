#pragma once

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/LocalBuffer.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceLowLevel.hpp>

#include <atomic>
#include <memory>
#include <stdexcept>
#include <vector>

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
        {}

        void start(void* inputs) override
        {
           this->copy_in(inputs);
           start();
        }

        void waitForCompletion(void* output) override
        {
          waitForCompletion();
          this->copy_out(output);
        }

      private:
        using AllreduceLowLevel<T, Algorithm>::start;
        using AllreduceLowLevel<T, Algorithm>::waitForCompletion;
    };

  
  }
}