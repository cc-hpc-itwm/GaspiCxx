/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019
 *
 * This file is part of GaspiCxx.
 *
 * GaspiCxx is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * GaspiCxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Collective.hpp
 *
 */

// Collective communication is defined as communication that involves a group of processes.
//
// A list of collective operations is e.g.
// 	- Barrier synchronization across all group members
//  - Broadcast from one member to all members of a group (same data for all)
//  - Gather data from all group members to one member
//  - Scatter data from one member to all members of a group (specific data for each)
//  - A variation on Gather where all members of the group receive the result (Allgather)
//  - Scatter/Gather data from all members to all members of a group (also called complete
//    exchange or all-to-all)
//  - Global reduction operations such as sum, max, min, or user-defined functions, where the
//    result is returned to all group members and a variation where the result is returned to
//    only one member.
//  - A combined reduction and scatter operation.
//  - Scan across all members of a group (also called prefix)

// Depending  on  the  communication data flow each collective can either be rooted or non-rooted.
// • Rooted Collectives - data being communicated from or converged into one node by many other
//   participating nodes in the collective.  Example collective operations include Broadcast, Gather
//   Scatter, Reduce and Scan
// • Non Rooted Collectives - data being communicated be-tween many nodes at the same time.  These
//   collective operations does not originate or destined towards one particular node.  Example collective
//   operations include Allgather, AllScatter, Allreduce and Barrier

// Design principles:
//
//  Peformance
//
//	Separation of setup / execution phase
//		-> do expensive init once, reuse infrastructure multiple times
//
//	Separation of communication initiation from check for completion
//		-> allow for overlap
//
//	Check for completion may eventually generate progress
//
//	Thread safe
//
//	Allow for zero copy
//
//	-> GPI / Gaspi design principles are used
//
//  Convenience
//
//	Allow for multiple collectives of same type at the same time
//
//  Should follow the semantics of GaspiCxx point to point communication
//  (Source / Target buffer)
//
//What follows is:
//
//	0.) Collectives are C++ objects
//		-> Encapsulation of resources -> allow for multiple collectives at the same time
//
//	Setup phase:
//
//
//
//	1.) local allocation of resources
//
//		possibility to work on exsiting pgas memory -> zero copy
//
//	2.) global connection (collective operation)
//
//		(similar to source/target buffer connectivity/allow to get independent
//       of the order of instantiation)
//
//
//	Execution phase

#ifndef COLLECTIVES_COLLECTIVE_BASE_HPP_
#define COLLECTIVES_COLLECTIVE_BASE_HPP_

#include <mutex>

namespace gaspi {
namespace collectives {

class CollectiveLowLevel {

public:

	enum class State
	{
	  UNINITIALIZED,
	  READY,
	  RUNNING,
    FINISHED
	};

	// Returned by setup in order to be able to separate
	// initiation of setup process from its completion
	// Required to be able to avoid deadlock in case that
	// there is no globally consistent order of setup
	// calls across different collectives.

	//
	class SetupHandle
	{
		public:

		virtual
		~SetupHandle() = default;

		virtual void
		waitForCompletion() = 0;

	};

	// Instantiation
	// -> local allocation of resources
	CollectiveLowLevel()
	: _state(State::UNINITIALIZED)
	{}


	virtual
	~CollectiveLowLevel() = default;

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
	void setup()
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state != State::UNINITIALIZED)
    {
      throw std::logic_error("[CollectiveLowLevel::setup] Collective already initialized.");
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
	start()
	{
    std::lock_guard<std::mutex> const lock(_state_mutex);

	  if(_state != State::READY)
    {
      throw std::logic_error("[CollectiveLowLevel::start] Collective already started or not initialized.");
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
    checkForCompletion()
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
	waitForCompletion()
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
	triggerProgress()
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

  void copy_in(void* inputs)
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state == State::READY)
    {
      copy_in_impl(inputs);
    }
  }

  void copy_out(void* outputs)
  {
    std::lock_guard<std::mutex> const lock(_state_mutex);
    if(_state == State::FINISHED)
    {
      copy_out_impl(outputs);
      _state = State::READY;
    }
  }

protected:
	virtual void
	setup_impl() = 0;
  
  virtual void
	init_communication_impl() = 0;

  virtual bool
  trigger_progress_impl() = 0;

  virtual void copy_in_impl(void*) = 0;
  virtual void copy_out_impl(void*) = 0;

	State _state; // atomic? (because of waitForcompletion)
  std::mutex _state_mutex;
};


}
}

#endif
