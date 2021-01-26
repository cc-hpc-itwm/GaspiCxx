#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>

namespace gaspi {
namespace collectives {

	// Instantiation
	// -> local allocation of resources
	CollectiveLowLevel::CollectiveLowLevel()
	: _state(State::UNINITIALIZED)
	{}

	// Setup phase
	// -> exchange of meta information
	// -> local
	// setup is a collective operation, i.e. it needs to be
	// invoked by all members in the group
	// Can be invoked only once during life time
	// Has to be invoked
	// SetupHandle::waitForCompletion changes state
	// from UNITIALIZED to READY
	// virtual std::unique_ptr<SetupHandle>
	void CollectiveLowLevel::setup()
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state != State::UNINITIALIZED)
    {
      throw std::logic_error(
        "[CollectiveLowLevel::setup] Collective already initialized.");
    }
    setup_impl();
    _state = State::READY;
  }

	// Execution phase

	// Start collective operation
	// Has to be invoked by a single thread only in
	// each member of the group (collective operation)
	// After start has been invoked, eventual source buffers
	// may not be overwritten and eventual target buffers may
	// not be read until check for completion returns true
	// or waitForCompletion has been invoked
	void
	CollectiveLowLevel::start()
	{
    std::lock_guard<std::mutex> const lock(_state_mutex);

	  if(_state != State::READY)
    {
      throw std::logic_error(
        "[CollectiveLowLevel::start] Collective already started or not initialized.");
    }
    init_communication_impl();
    _state = State::RUNNING;
	}

	// After successful completion, i.e. return value true,
	// eventual target buffers may be read and eventual source
	// buffers may be overwritten
	// May be invoked by more than a single thread. However, only
	// a single thread completes
	// Might require several invocations before successful completion
	bool
    CollectiveLowLevel::checkForCompletion()
	{
    std::unique_lock<std::mutex> lock(_state_mutex, std::defer_lock);
    bool isCompleted = false;

		if(lock.try_lock())
    {
		  isCompleted = (_state == State::FINISHED);
    }
		return isCompleted;
	}
  
	// After completion, eventual target buffers may be read and
	// eventual source buffers may be overwritten
	// May be invoked by more than a single thread.
	// Return value true is only given by a single thread
	bool
	CollectiveLowLevel::waitForCompletion()
	{
	  bool thisThreadCompletes (false);
	  while(!thisThreadCompletes && _state == State::RUNNING)
	  {
      thisThreadCompletes = triggerProgress();
	  };
	  return thisThreadCompletes;
	}

	// Implements generation of progress
	// Invoked by a single thread only.
	// It is ensured that there is only a single exclusive call of
	// trigger_progress at any time
	// It is only invoked when state is RUNNING
	// Has to changes state from RUNNING to READY if
	// the generated progress completes the collective
	// Has to ensure that eventual output buffers are ready,
	// i.e. contain valid data, if the generated progress completes
	// the collective
  bool
	CollectiveLowLevel::triggerProgress()
  {
    std::unique_lock<std::mutex> lock(_state_mutex, std::defer_lock);
    bool isCompleted = false;

		if(lock.try_lock())
    {
		  if(_state == State::RUNNING) {
        isCompleted = trigger_progress_impl();
        if (isCompleted)
        {
          _state = State::FINISHED;
        }
		  }
    }
    return isCompleted;
  }

  void CollectiveLowLevel::copy_in(void* inputs)
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state == State::READY)
    {
      copy_in_impl(inputs);
    }
  }

  void CollectiveLowLevel::copy_out(void* outputs)
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state == State::FINISHED)
    {
      copy_out_impl(outputs);
      _state = State::READY;
    }
  }

  }
  }