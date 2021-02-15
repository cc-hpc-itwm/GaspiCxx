#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>

namespace gaspi
{
  namespace progress_engine
  {
    RoundRobinDedicatedThread::RoundRobinDedicatedThread()
    : terminate_man_thread(false),
      management_thread(&RoundRobinDedicatedThread::generate_progress, this)
    {}

    RoundRobinDedicatedThread::~RoundRobinDedicatedThread()
    {
      terminate_man_thread = true;
      if (management_thread.joinable())
      {
        management_thread.join();
      }
    }

    void RoundRobinDedicatedThread::generate_progress()
    {
      while (!terminate_man_thread)
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