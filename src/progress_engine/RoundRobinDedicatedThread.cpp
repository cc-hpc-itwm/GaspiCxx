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
 * RoundRobinDedicatedThread.cpp
 *
 */

#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>

#include <algorithm>
#include <vector>

namespace gaspi
{
  namespace progress_engine
  {
    RoundRobinDedicatedThread::RoundRobinDedicatedThread()
    : updated_operators(false),
      current_handle(0UL),
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
      std::vector<std::shared_ptr<gaspi::collectives::CollectiveLowLevel>> current_operators;

      while (!terminate_man_thread)
      {
        {
          std::unique_lock<std::mutex> lock(operators_mutex);
          condition.wait(lock, [&done = terminate_man_thread,
                                &operators = operators]
                                {
                                  return (operators.size()>0) || done;
                                });
          if (updated_operators)
          {
            current_operators.clear();
            std::transform(operators.begin(), operators.end(), std::back_inserter(current_operators),
                           [](auto const& p) { return p.second; });
            updated_operators = false;
          }
          lock.unlock();
        }

        if (!terminate_man_thread)
        {
          for (auto& op : current_operators)
          {
            op->triggerProgress();
          }
        }
      }
    }

    ProgressEngine::CollectiveHandle
    RoundRobinDedicatedThread::register_collective(
              std::shared_ptr<collectives::CollectiveLowLevel> collective)
    {
      {
        std::lock_guard<std::mutex> const lock(operators_mutex);
        updated_operators = true;
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
      updated_operators = true;
      auto const count = operators.erase(handle);
      if(count != 1)
      {
        throw std::logic_error("ProgressEngine: Could not remove operator");
      }
    }
  }
}
