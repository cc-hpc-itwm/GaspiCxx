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
 * Runtime.cpp
 *
 */

#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/type_defs.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>
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
, Context()
, _segmentSize(1024*1024)
, _psegment(std::make_unique<segment::Segment>(_segmentSize))
, _ppassive(std::make_unique<passive::Passive>( *_psegment
                                              , *this ) )
, _pengine(std::make_unique<progress_engine::RoundRobinDedicatedThread>())
{ }

void
Runtime
  ::synchCurrentWorkingDirectory
    ()
{
  if(group().toGlobalRank(rank()) == group::GlobalRank(0)) {

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

} // namespace gaspi
