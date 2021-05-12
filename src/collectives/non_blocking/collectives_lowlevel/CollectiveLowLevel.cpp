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
 * CollectiveLowLevel.cpp
 *
 */

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>

namespace gaspi {
namespace collectives {

  CollectiveLowLevel::CollectiveLowLevel()
  : _state(State::UNINITIALIZED)
  {}

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

  void CollectiveLowLevel::copyIn(void const* inputs)
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

  bool CollectiveLowLevel::checkForCompletion()
  {
    return (_state == State::FINISHED);
  }
  
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
