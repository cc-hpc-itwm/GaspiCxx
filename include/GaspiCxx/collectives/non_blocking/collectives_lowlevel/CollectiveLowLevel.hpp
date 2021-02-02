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
 * CollectiveLowLevel.hpp
 *
 */

#pragma once

#include <atomic>
#include <mutex>
#include <vector>

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

  CollectiveLowLevel();
  virtual ~CollectiveLowLevel() = default;

  void waitForSetup();
  void start();
  bool checkForCompletion();
  bool waitForCompletion();
  bool triggerProgress();
  void copyIn(void* inputs);
  template<typename T>
  void copyIn(std::vector<T> const& inputs);
  void copyOut(void* outputs);
  template<typename T>
  void copyOut(std::vector<T>& outputs);

protected:
  virtual void waitForSetupImpl() = 0;
  virtual void startImpl() = 0;
  virtual bool triggerProgressImpl() = 0;

  virtual void copyInImpl(void*) = 0;
  virtual void copyOutImpl(void*) = 0;

  std::atomic<State> _state;
  std::mutex _state_mutex;
};

}
}
