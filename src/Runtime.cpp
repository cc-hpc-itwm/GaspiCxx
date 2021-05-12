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
 * Runtime.cpp
 *
 */
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/RuntimeConfiguration.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/utility/Filesystem.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {

RuntimeBase
  ::RuntimeBase
    ()
{
  GASPI_CHECK
    ( gaspi_proc_init(GASPI_BLOCK) );
}

RuntimeBase
  ::~RuntimeBase
    ()
{
  GASPI_CHECK_NOTHROW(gaspi_proc_term(GASPI_BLOCK));
}

Runtime
  ::Runtime
   ()
: RuntimeBase()
, SingleQueueContext()
, _group_all()
, _segmentSize(1024*1024)
, _psegment(std::make_unique<segment::Segment>(_segmentSize))
, _ppassive(std::make_unique<passive::Passive>( *_psegment
                                              , *this ) )
, _psegment_pool()
, _pengine()
, _pcomm_context(Runtime::configuration.get_communication_context())
, _pglobal_barrier()
{ }

Runtime
  ::~Runtime()
{
  _pglobal_barrier->execute();
}

segment::Segment &
Runtime
  ::segment
    ()
{
  return *_psegment;
}

passive::Passive &
Runtime
  ::passive
    ()
{
  return *_ppassive;
}

segment::Segment &
Runtime
  ::getFreeSegment
    (std::size_t size)
{
  if (!_psegment_pool)
  {
    _psegment_pool = Runtime::configuration.get_segment_pool();
  }
  if (!_psegment_pool)
  {
    throw std::runtime_error(
          "[Runtime::getFreeSegment] Segment Pool undefined.");
  }
  return _psegment_pool->getSegment(size);
}

progress_engine::ProgressEngine &
Runtime
  ::getDefaultProgressEngine
    ()
{
  if (!_pengine)
  {
    _pengine = Runtime::configuration.get_progress_engine();
  }
  if (!_pengine)
  {
    throw std::runtime_error(
          "[Runtime::getDefaultProgressEngine] Progress engine undefined.");
  }
  return *_pengine;
}

CommunicationContext &
Runtime
  ::getDefaultCommunicationContext
    ()
{
  return *_pcomm_context;
}

void
Runtime
  ::synchCurrentWorkingDirectory
    ()
{
  if(global_rank() == group::GlobalRank(0)) {

    std::string const dir(getCurrentWorkingDirectory());

    for( group::GlobalRank targetRank(1);targetRank < size(); ++targetRank) {
      _ppassive->sendMessg
        ( dir.c_str()
        , dir.size()
        , targetRank );
    }
  }
  else {

    std::vector<char> buffer;

    int srcRank;

    _ppassive->recvMessg
      ( buffer
      , srcRank );

    std::string dir(buffer.data(),buffer.size());

    setCurrentWorkingDirectory(dir);

  }
}

Runtime &
Runtime
  ::getRuntime
    ()
{
  static auto instance = new Runtime();
  return *instance;
}

void
Runtime
  ::barrier
    ()
{
  if (!_pglobal_barrier)
  {
    _pglobal_barrier = std::make_unique<gaspi::collectives::blocking::Barrier>(_group_all);
  }
  _pglobal_barrier->execute();
}

group::GlobalRank
Runtime
  ::global_rank()
{
  return _group_all.global_rank();
}

std::size_t
Runtime
  ::size()
{
  return _group_all.size();
}

Runtime &
getRuntime()
{
  return Runtime::getRuntime();
}

void
initGaspiCxx()
{
  // Initialize GPI, create management segment
  // and setup passive communication
  Runtime::getRuntime();
}

void
initGaspiCxx(RuntimeConfiguration const& config)
{
  // Set customized configuration,
  // initialize GPI, create management segment
  // and setup passive communication
  Runtime::configuration = config;
  Runtime::getRuntime();
}

} // namespace gaspi
