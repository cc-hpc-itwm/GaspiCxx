
#include <GaspiCxx/progress_engine/ProgressEngine.hpp>

namespace gaspi
{
  namespace progress_engine
  {
    ProgressEngine::ProgressEngine()
    : current_handle(0UL)
    {}
    
    ProgressEngine::CollectiveHandle ProgressEngine::register_collective(
              std::shared_ptr<collectives::CollectiveLowLevel> collective)
    {
      std::lock_guard<std::mutex> const lock(operators_mutex);
      ++current_handle;
      bool const is_okay = operators.insert({current_handle, collective}).second;
      if(!is_okay)
      {
        throw std::logic_error("ProgressEngine: Handle has already been used.");
      }
      return (current_handle);
    }

    void ProgressEngine::deregister_collective(CollectiveHandle const& handle)
    {
      std::lock_guard<std::mutex> const lock(operators_mutex);
      auto const count = operators.erase(handle);
      if(count != 1)
      {
        throw std::logic_error("ProgressEngine: Could not remove operator");
      }
    }
  }
}