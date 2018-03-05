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

//  gaspi_segment_id_t segmentId(0);
  gaspi_size_t       segmentSize(1024*1024);
//
//  GASPI_CHECK
//      ( gaspi_segment_create( segmentId
//                            , segmentSize
//                            , GASPI_GROUP_ALL
//                            , GASPI_BLOCK
//                            , GASPI_MEM_INITIALIZED ) );

  _psegment.reset
    ( new segment::Segment(segmentSize) );

  _ppassive.reset
    ( new passive::Passive( *_psegment
                          , *this ) );
}

Runtime
  ::~Runtime
   ()
{
  _ppassive.reset( nullptr );

//  gaspi_segment_id_t segmentId(_psegment->id() );

  _psegment.reset( nullptr );

//  GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL,GASPI_BLOCK));
//
//  GASPI_CHECK(gaspi_segment_delete(segmentId));

  pGRuntime = nullptr;
}

//Runtime::Rank
//Runtime
//  ::proc_rank
//     () const
//{
//  gaspi_rank_t rank;
//  GASPI_CHECK(gaspi_proc_rank(&rank));
//
//  return static_cast<Rank>(rank);
//}
//
//Runtime::Rank
//Runtime
//  ::proc_size
//     () const
//{
//  gaspi_rank_t size;
//  GASPI_CHECK(gaspi_proc_num(&size));
//
//  return static_cast<Rank>(size);
//}
//
//void
//Runtime
//  ::barrier
//     () const
//{
//  GASPI_CHECK
//    ( gaspi_barrier
//        ( GASPI_GROUP_ALL
//        , GASPI_BLOCK ) );
//}

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
