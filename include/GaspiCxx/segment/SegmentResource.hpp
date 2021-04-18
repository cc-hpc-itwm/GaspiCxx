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
 * SegmentResource.hpp
 *
 */

#ifndef SEGMENT_RESOURCE_HPP_
#define SEGMENT_RESOURCE_HPP_

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/segment/SegmentManager.hpp>
#include <GaspiCxx/segment/Types.hpp>

namespace gaspi {
namespace segment {

class SegmentResource
{
  public:

    SegmentResource
      ( std::size_t segmentSize );

    SegmentResource
      ( SegmentID segmentId
      , std::size_t segmentSize );

    SegmentResource
      ( SegmentResource const& ) = delete;

    SegmentResource
      ( SegmentResource&& );

    virtual ~SegmentResource
      ();

    SegmentID
    id
      () const;

    void
    remoteRegistration
      ( group::GlobalRank const&);

    static SegmentID
    getFreeLocalSegmentId();

private:
    SegmentID _segmentId;
    bool _contains_valid_segment;
};

}
}

#endif /* SEGMENT_RESOURCE_HPP_ */
