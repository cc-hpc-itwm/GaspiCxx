#pragma once

#include <GaspiCxx/progress_engine/ProgressEngine.hpp>

#include <atomic>
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

      private:
        void generate_progress() override;
    
        std::atomic<bool> terminate_man_thread;
        std::thread management_thread;
    };
  }
}