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
 * RoundRobinDedicatedThread.cpp
 *
 */

#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>

namespace gaspi
{
  namespace progress_engine
  {
    RoundRobinDedicatedThread::RoundRobinDedicatedThread()
    : current_handle(0UL),
      terminate_man_thread(false),
      management_thread(&RoundRobinDedicatedThread::generate_progress, this)
    {}

    RoundRobinDedicatedThread::~RoundRobinDedicatedThread()
    {
      {
        std::lock_guard<std::mutex> lock(operators_mutex);
        terminate_man_thread = true;
      }
      condition.notify_one();
      if (management_thread.joinable())
      {
        management_thread.join();
      }
    }

    void RoundRobinDedicatedThread::generate_progress()
    {
      while (!terminate_man_thread)
      {
        std::unique_lock<std::mutex> lock(operators_mutex);
        condition.wait(lock, [&done = terminate_man_thread,
                              &operators = operators]
                              {
                                return (operators.size()>0) || done;
                              });
        if (!terminate_man_thread)
        {
          for (auto& handle_and_operator : operators)
          {
            handle_and_operator.second->triggerProgress();
          }
        }
        lock.unlock();
      }
    }

    ProgressEngine::CollectiveHandle
    RoundRobinDedicatedThread::register_collective(
              std::shared_ptr<collectives::CollectiveLowLevel> collective)
    {
      {
        std::lock_guard<std::mutex> const lock(operators_mutex);
        ++current_handle;
        bool const is_okay = operators.insert({current_handle, collective}).second;
        if(!is_okay)
        {
          throw std::logic_error("ProgressEngine: Handle has already been used.");
        }
      }
      condition.notify_one();
      return (current_handle);
    }

    void RoundRobinDedicatedThread::deregister_collective(
                                    CollectiveHandle const& handle)
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
