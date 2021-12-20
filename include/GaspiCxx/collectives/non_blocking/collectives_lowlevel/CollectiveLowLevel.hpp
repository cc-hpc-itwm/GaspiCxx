/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
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
 * CollectiveLowLevel.hpp
 *
 */

#pragma once

#include <atomic>
#include <mutex>

namespace gaspi {
namespace collectives {

class CollectiveLowLevel {

public:
  enum class State
  {
    UNINITIALIZED,  // object created but connections not yet established
    INITIALIZED,    // connections established but input data not yet available
    READY,          // data copied in; ready to start communication
    RUNNING,        // executing communication
    FINISHED        // communication finished and ready to read out results
  };

  static constexpr auto NO_DATA = nullptr;

  // Instantiation phase
  // ===================
  // -> local allocation of resources
  CollectiveLowLevel();
  virtual ~CollectiveLowLevel() = default;

  // Setup phase
  // ===========
  // -> exchange of meta information
  // -> collective operation, i.e. needs to be
  // invoked by all members in the group
  // Has to be invoked exactly once during life time
  // `waitForSetup` changes state from UNITIALIZED to INITIALIZED
  void waitForSetup();

  // Execution phase
  // ===============
  // Copy data to communication buffers before execution
  // Changes state from INITIALIZED to READY
  void copyIn(void const* inputs);

  // Start collective operation
  // Only one thread should invoke `start`, which will initiate the 
  // algorithm and change the state from INITIALIZED to RUNNING.
  void start();

  // Implements generation of progress.
  // Must be NON-BLOCKING (implemented based on `check*` buffer primitives)
  // to be compatible with a `ProgressEngine`
  // Can be called in any state, but only one thread will
  // trigger progress if and only if state equals RUNNING.
  // Changes state from RUNNING to FINISHED if
  // the generated progress completes the collective.
  bool triggerProgress();

  // Write results to `outputs` as a contiguous buffer.
  // Changes state from FINISHED to INITIALIZED,
  // which allows to re-execute the collective
  // (after resetting the input data with `copyIn`).
  void copyOut(void* outputs);

  // Convenience functions
  // =====================
  // Checks whether the operation has completed.
  bool checkForCompletion();

  // If in state RUNNING, blocks until completion of the collective
  // (through calling trigger progress).
  // If called in another state, does nothing.
  // May be invoked by more than a single thread.
  // The thread that triggered the final step of the algorithm will 
  // return `true`, all others will return `false`.
  bool waitForCompletion();

protected:
  virtual void waitForSetupImpl() = 0;
  virtual void startImpl() = 0;
  virtual bool triggerProgressImpl() = 0;

  virtual void copyInImpl(void const*) = 0;
  virtual void copyOutImpl(void*) = 0;

  std::atomic<State> _state;
  std::mutex _state_mutex;
};

}
}
