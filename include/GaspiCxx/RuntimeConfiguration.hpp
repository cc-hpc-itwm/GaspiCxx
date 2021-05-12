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
 * RuntimeConfiguration.hpp
 *
 */

#pragma once

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/segment/SegmentPool.hpp>

namespace gaspi {

enum class SegmentPoolType
{
  None,
  SingleSegment,
  DynamicSegmentPool
};

enum class ProgressEngineType
{
  None,
  RoundRobinDedicatedThread
};

enum class CommunicationContextType
{
  None,
  SingleQueue,
  RoundRobinQueues
};

class RuntimeConfiguration
{
  public:
    explicit RuntimeConfiguration(
      SegmentPoolType, ProgressEngineType, CommunicationContextType);

    RuntimeConfiguration(RuntimeConfiguration const&) = default;
    ~RuntimeConfiguration() = default;

    std::unique_ptr<segment::SegmentPool> get_segment_pool() const;
    std::unique_ptr<progress_engine::ProgressEngine> get_progress_engine() const;
    std::unique_ptr<CommunicationContext> get_communication_context() const;

  private:
    SegmentPoolType segment_pool_type;
    ProgressEngineType progress_engine_type;
    CommunicationContextType communication_context_type;

};

} // namespace gaspi
