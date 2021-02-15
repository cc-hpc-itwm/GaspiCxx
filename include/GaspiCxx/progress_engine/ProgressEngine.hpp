#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace gaspi
{
  namespace progress_engine
  {
    class ProgressEngine
    {
      public:
        using CollectiveHandle = std::size_t;

        ProgressEngine();
        virtual ~ProgressEngine() = default;

        CollectiveHandle register_collective(std::shared_ptr<collectives::CollectiveLowLevel>);
        void deregister_collective(CollectiveHandle const&);

      protected:
        std::mutex operators_mutex;
        CollectiveHandle current_handle; 
        std::unordered_map<CollectiveHandle, 
                          std::shared_ptr<gaspi::collectives::CollectiveLowLevel>>
                                          operators;

        virtual void generate_progress() = 0;
    };
  }
}