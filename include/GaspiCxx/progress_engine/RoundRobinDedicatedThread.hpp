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
 * RoundRobinDedicatdThread.hpp
 *
 */

#pragma once

#include <GaspiCxx/progress_engine/ProgressEngine.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace gaspi
{
  namespace progress_engine
  {
    class RoundRobinDedicatedThread : public ProgressEngine
    {
      public:
        RoundRobinDedicatedThread();
        ~RoundRobinDedicatedThread();

        CollectiveHandle register_collective(
                std::shared_ptr<collectives::CollectiveLowLevel>) override;
        void deregister_collective(CollectiveHandle const&) override;

      private:
        void generate_progress() override;

        std::mutex operators_mutex;
        std::condition_variable condition;

        CollectiveHandle current_handle;
        std::unordered_map<CollectiveHandle,
                           std::shared_ptr<gaspi::collectives::CollectiveLowLevel>>
                                           operators;
        std::atomic<bool> terminate_man_thread;
        std::thread management_thread;
    };
  }
}
