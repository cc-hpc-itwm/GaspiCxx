/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
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
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * Runtime.cpp
 *
 */


#include <GaspiCxx/Runtime.hpp>
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
  GASPI_CHECK(gaspi_proc_term(GASPI_BLOCK));
}

static Runtime * pGRuntime = nullptr;

Runtime
  ::Runtime
   ()
: RuntimeBase()
, Context()
, _psegment()
{

  if( pGRuntime != nullptr ) {
    throw std::runtime_error
      (CODE_ORIGIN + "Only a single Instance allowed at a time");
  }
  pGRuntime = this;

  gaspi_size_t       segmentSize(1024*1024);

  _psegment.reset
    ( new segment::Segment(segmentSize) );

  _ppassive.reset
    ( new passive::Passive( *_psegment
                          , *this ) );

  if(rank() == group::Rank(0)) {

    std::string const dir(getCurrentWorkingDirectory());

    for( group::Rank targetRank(1);targetRank < size(); ++targetRank) {
      _ppassive->sendMessg
        ( dir.c_str()
        , dir.size()
        , targetRank.get() );
    }
  }
  else {

    std::vector<char> buffer;

    int srcRank;

    _ppassive->recvMessg
      ( buffer
      , srcRank );

    std::string dir(buffer.data());

    setCurrentWorkingDirectory(dir);

  }

}

Runtime
  ::~Runtime
   ()
{
  _ppassive.reset( nullptr );

  _psegment.reset( nullptr );

  pGRuntime = nullptr;
}

bool
isRuntimeAvailable()
{
  return !(pGRuntime == nullptr);
}

Runtime &
getRuntime()
{
  if( pGRuntime == nullptr ) {
    throw std::runtime_error
      (CODE_ORIGIN + "Runtime has not been initialized yet");
  }
  return *pGRuntime;
}

} // namespace gaspi
