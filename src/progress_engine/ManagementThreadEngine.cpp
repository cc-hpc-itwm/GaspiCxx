#include <progress_engine/ManagementThreadEngine.hpp>

namespace gaspi
{

  ManagementThreadEngine::ManagementThreadEngine()
  : terminate_man_thread(false),
    management_thread(&ManagementThreadEngine::generate_progress, this)
  {}

  ManagementThreadEngine::~ManagementThreadEngine()
  {
    terminate_man_thread = true;
    if (management_thread.joinable())
    {
      management_thread.join();
    }
  }

  void ManagementThreadEngine::generate_progress()
  {
    while (!terminate_man_thread)
    {
      while (true)
      {
        if (terminate_man_thread)
        {
          break;
        }
        {
          std::lock_guard<std::mutex> const lock(operators_mutex);
          for (auto& handle_and_operator : operators)
          {
            handle_and_operator.second->triggerProgress();
          }
        }
      }
    }
  }

}