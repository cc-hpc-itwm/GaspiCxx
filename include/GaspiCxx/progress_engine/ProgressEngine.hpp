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

        virtual ~ProgressEngine() = default;

        virtual CollectiveHandle register_collective(
                std::shared_ptr<collectives::CollectiveLowLevel>) = 0;
        virtual void deregister_collective(CollectiveHandle const&) = 0;

      protected:
        virtual void generate_progress() = 0;
    };
  }
}