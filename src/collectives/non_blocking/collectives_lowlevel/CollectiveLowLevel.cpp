#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>

namespace gaspi {
namespace collectives {

  // Instantiation phase
  // ===================
  // -> local allocation of resources
  CollectiveLowLevel::CollectiveLowLevel()
  : _state(State::UNINITIALIZED)
  {}

  // Setup phase
  // ===========
  // -> exchange of meta information
  // -> collective operation, i.e. needs to be
  // invoked by all members in the group
  // Has to be invoked exactly once during life time
  // `waitForSetup` changes state from UNITIALIZED to INITIALIZED
  void CollectiveLowLevel::waitForSetup()
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state != State::UNINITIALIZED)
    {
      throw std::logic_error(
        "[CollectiveLowLevel::waitForSetup] Collective already initialized.");
    }
    waitForSetupImpl();
    _state = State::INITIALIZED;
  }

  // Execution phase
  // ===============
  // Copy data to communication buffers before execution
  // Changes state from INITIALIZED to READY
  void CollectiveLowLevel::copyIn(void* inputs)
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state != State::INITIALIZED)
    {
      throw std::logic_error(
        "[CollectiveLowLevel::copyIn] Collective is not in the INITIALIZED state.");
    }
    copyInImpl(inputs);
    _state = State::READY;
  }

  // Start collective operation
  // Only one thread should invoke `start`, which will initiate the 
  // algorithm and change the state from INITIALIZED to RUNNING.
  void CollectiveLowLevel::start()
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);

    if(_state != State::READY)
    {
      throw std::logic_error(
        "[CollectiveLowLevel::start] Collective already started or not initialized.");
    }
    startImpl();
    _state = State::RUNNING;
  }

  // Implements generation of progress.
  // Can be called in any state, but only one thread will
  // trigger progress if and only if state equals RUNNING.
  // Changes state from RUNNING to FINISHED if
  // the generated progress completes the collective.
  bool CollectiveLowLevel::triggerProgress()
  {
    std::unique_lock<std::mutex> lock(_state_mutex, std::defer_lock);
    bool isCompleted = false;

    if(lock.try_lock())
    {
      if(_state == State::RUNNING) {
        isCompleted = triggerProgressImpl();
        if (isCompleted)
        {
          _state = State::FINISHED;
        }
      }
    }
    return isCompleted;
  }

  // Write results to `outputs` as a contiguous buffer.
  // Changes state from FINISHED to INITIALIZED,
  // which allows to re-execute the collective
  // (after resetting the input data with `copyIn`).
  void CollectiveLowLevel::copyOut(void* outputs)
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state != State::FINISHED)
    {
      throw std::logic_error(
        "[CollectiveLowLevel::copyOut] Collective is not in the FINISHED state.");
    }
    copyOutImpl(outputs);
    _state = State::INITIALIZED;
  }

  // Convenience functions
  // =====================
  // Checks whether the operation has completed.
  bool CollectiveLowLevel::checkForCompletion()
  {
    return (_state == State::FINISHED);
  }
  
  // If in state RUNNING, blocks until completion of the collective
  // (through calling trigger progress).
  // If called in another state, does nothing.
  // May be invoked by more than a single thread.
  // The thread that triggered the final step of the algorithm will 
  // return `true`, all others will return `false`.
  bool CollectiveLowLevel::waitForCompletion()
  {
    bool thisThreadCompletes (false);
    while(!thisThreadCompletes && _state == State::RUNNING)
    {
      thisThreadCompletes = triggerProgress();
    };
    return thisThreadCompletes;
  }

}
}