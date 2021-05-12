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
 * RuntimeConfiguration.cpp
 *
 */

#include <GaspiCxx/RuntimeConfiguration.hpp>

#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>

#include <GaspiCxx/segment/SegmentPool.hpp>
#include <GaspiCxx/segment/SingleSegmentPool.hpp>
#include <GaspiCxx/segment/DynamicSegmentPool.hpp>

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/SingleQueueContext.hpp>
#include <GaspiCxx/RoundRobinQueuesContext.hpp>

namespace gaspi
{
  namespace
  {
    class SegmentPoolFactory
    {
      public:
        static std::unique_ptr<segment::SegmentPool>
              createSegmentPool(SegmentPoolType segment_pool_type)
        {
          std::size_t const default_size(100L*1024L*1024L);

          switch (segment_pool_type)
          {
            case SegmentPoolType::SingleSegment:
            {
              return std::make_unique<segment::SingleSegmentPool>(default_size);
            }
            case SegmentPoolType::DynamicSegmentPool:
            {
              return std::make_unique<segment::DynamicSegmentPool>(default_size);
            }
            default:
            { return nullptr; }
          }
        }
    };

    class ProgressEngineFactory
    {
      public:
        static std::unique_ptr<progress_engine::ProgressEngine>
              createProgressEngine(ProgressEngineType progress_engine_type)
        {
          switch (progress_engine_type)
          {
            case ProgressEngineType::RoundRobinDedicatedThread:
            {
              return std::make_unique<progress_engine::RoundRobinDedicatedThread>();
            }
            default:
            { return nullptr; }
          }
        }
    };

    class CommunicationContextFactory
    {
      public:
        static std::unique_ptr<CommunicationContext>
              createCommunicationContext(CommunicationContextType
                                         communication_context_type)
        {
          switch (communication_context_type)
          {
            case CommunicationContextType::SingleQueue:
            {
              return std::make_unique<SingleQueueContext>();
            }
            case CommunicationContextType::RoundRobinQueues:
            {
              std::size_t num_queues = 4;
              return std::make_unique<RoundRobinQueuesContext>(num_queues);
            }
            default:
            { return nullptr; }
          }
        }
    };
  }
  
  RuntimeConfiguration::RuntimeConfiguration(
        SegmentPoolType segment_pool_type,
        ProgressEngineType progress_engine_type,
        CommunicationContextType communication_context_type)
  : segment_pool_type(segment_pool_type),
    progress_engine_type(progress_engine_type),
    communication_context_type(communication_context_type)
  { }

  std::unique_ptr<segment::SegmentPool>
  RuntimeConfiguration::get_segment_pool() const
  {
    return SegmentPoolFactory::createSegmentPool(segment_pool_type);
  }
  
  std::unique_ptr<progress_engine::ProgressEngine>
  RuntimeConfiguration::get_progress_engine() const
  {
    return ProgressEngineFactory::createProgressEngine(progress_engine_type);
  }

  std::unique_ptr<CommunicationContext>
  RuntimeConfiguration::get_communication_context() const
  {
    return CommunicationContextFactory::createCommunicationContext(
                                        communication_context_type);
  }
}
