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
 * RoundRobinQueuesContext.hpp
 *
 */

#pragma once

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>

#include <atomic>
#include <mutex>
#include <random>
#include <vector>

namespace gaspi {

using QueueID = gaspi_queue_id_t;

class RoundRobinQueuesContext : public CommunicationContext
{
  public:

    RoundRobinQueuesContext(std::size_t);

    RoundRobinQueuesContext(RoundRobinQueuesContext const&) = delete;
    RoundRobinQueuesContext& operator=(RoundRobinQueuesContext const&) = delete;
    ~RoundRobinQueuesContext();

    void
    writePart
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription
      , std::size_t size
      , std::size_t offset ) override;

    void
    notify
      ( singlesided::BufferDescription targetBufferDescription ) override;

    void
    flush
      () override;

  private:
  
    std::size_t const num_queues;
    std::vector<singlesided::Queue> gaspi_queues;
    std::mutex queue_index_mutex;
    std::atomic<std::size_t> queue_index;
    
    std::size_t
	select_available_queue
		(std::size_t const & full_queue_index);
};
}
