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

#ifndef COLLECTIVES_COLLECTIVE_LOWLEVEL_HPP_
#define COLLECTIVES_COLLECTIVE_LOWLEVEL_HPP_

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

  CollectiveLowLevel();
  virtual ~CollectiveLowLevel() = default;

  void setup();
  void start();
  bool checkForCompletion();
  bool waitForCompletion();
  bool triggerProgress();
  void copy_in(void* inputs);
  void copy_out(void* outputs);

protected:
  virtual void setup_impl() = 0;
  virtual void init_communication_impl() = 0;
  virtual bool trigger_progress_impl() = 0;

  virtual void copy_in_impl(void*) = 0;
  virtual void copy_out_impl(void*) = 0;

  State _state; // atomic? (because of waitForcompletion)
  std::mutex _state_mutex;
};

}
}

#endif
