#pragma once

#include <GaspiCxx/collectives/non_blocking/Collective.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastLowLevel.hpp>

#include <stdexcept>

namespace gaspi
{
  namespace collectives
  {

    template<typename T, BroadcastAlgorithm Algorithm>
    class Broadcast : public RootedSendCollective, 
                      private BroadcastLowLevel<T, Algorithm>
    { 
      public:
        Broadcast(gaspi::segment::Segment& segment,
                  gaspi::group::Group const& group,
                  std::size_t number_elements,
                  gaspi::group::Rank const& root_rank)
        : BroadcastLowLevel<T, Algorithm>(segment, group, number_elements, root_rank)
        {
          waitForSetup();
          // TODO here register with the progress engine
        }

        void start(void* inputs) override
        {
          if(group.rank() != root_rank)
          {
            throw std::logic_error("Broadcast: start(void* inputs) may only be called on root rank.");
          }
          copyIn(inputs);
          start();
        }

        void start() override
        {
          if(group.rank() == root_rank)
          {
            throw std::logic_error("Broadcast: start() may only be called on non-root ranks.");
          }
          copyIn(CollectiveLowLevel::NO_DATA);
          start();
        }

        void waitForCompletion(void* output) override
        {
          waitForCompletion();
          copyOut(output);
        }

      private:
        using BroadcastLowLevel<T, Algorithm>::waitForSetup;
        using BroadcastLowLevel<T, Algorithm>::start;
        using BroadcastLowLevel<T, Algorithm>::waitForCompletion;
        using BroadcastLowLevel<T, Algorithm>::copyIn;
        using BroadcastLowLevel<T, Algorithm>::copyOut;
    };

  
  }
}
