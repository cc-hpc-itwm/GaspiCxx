#pragma once

#include <GaspiCxx/progress_engine/ProgressEngine.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace gaspi
{
  namespace progress_engine
  {
    class RoundRobinDedicatedThread : public ProgressEngine
    {
      public:
        RoundRobinDedicatedThread();
        ~RoundRobinDedicatedThread();

        CollectiveHandle register_collective(
                std::shared_ptr<collectives::CollectiveLowLevel>) override;
        void deregister_collective(CollectiveHandle const&) override;

      private:
        void generate_progress() override;

        std::mutex operators_mutex;
        std::condition_variable condition;

        CollectiveHandle current_handle;
        std::unordered_map<CollectiveHandle,
                           std::shared_ptr<gaspi::collectives::CollectiveLowLevel>>
                                           operators;
        std::atomic<bool> terminate_man_thread;
        std::thread management_thread;
    };
  }
}
