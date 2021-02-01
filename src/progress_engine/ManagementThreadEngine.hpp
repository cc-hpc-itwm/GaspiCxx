#pragma once

#include <progress_engine/ProgressEngine.hpp>

#include <atomic>
#include <thread>

namespace gaspi
{

  class ManagementThreadEngine : public ProgressEngine
  {
    public:
      ManagementThreadEngine();
      ~ManagementThreadEngine();

    private:
      void generate_progress() override;
  
      std::atomic<bool> terminate_man_thread;
      std::thread management_thread;
  };

}