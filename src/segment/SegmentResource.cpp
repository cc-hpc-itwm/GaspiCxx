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
 * SegmentResource.cpp
 *
 */

#include <GaspiCxx/segment/SegmentResource.hpp>
#include <GaspiCxx/utility/Macros.hpp>

extern "C" {
#include <GASPI.h>
#include <GASPI_Ext.h>
}

namespace gaspi {
namespace segment {

SegmentID
SegmentResource
  ::getFreeLocalSegmentId()
{
  gaspi_segment_id_t avail_seg_id;

  GASPI_CHECK
    ( gaspi_segment_avail_local (&avail_seg_id) );

  return avail_seg_id;
}

SegmentResource
  ::SegmentResource
    ( std::size_t segmentSize )
: SegmentResource
    ( SegmentResource::getFreeLocalSegmentId()
    , segmentSize )
{}

SegmentResource
  ::SegmentResource
    ( SegmentID segmentId
    , std::size_t segmentSize)
: _segmentId( segmentId )
{
  GASPI_CHECK
      ( gaspi_segment_alloc( _segmentId
                            , segmentSize
                            , GASPI_MEM_INITIALIZED ) );

  gaspi_rank_t rank;
  GASPI_CHECK
        ( gaspi_proc_rank( &rank ) );

  remoteRegistration( rank );
}

SegmentResource
  ::~SegmentResource
    ()
{
  GASPI_CHECK_NOTHROW(gaspi_segment_delete(_segmentId));
}

SegmentID
SegmentResource
  ::id
   () const
{
  return _segmentId;
}

void
SegmentResource
  ::remoteRegistration
   ( group::GlobalRank const& rank )
{
  GASPI_CHECK
      ( gaspi_segment_register ( _segmentId
                               , rank
                               , GASPI_BLOCK) );
}

#undef GASPI_CHECK

} // namespace segment
} // namespace gaspi
