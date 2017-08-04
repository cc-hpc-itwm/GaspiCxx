/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
 *
 * This file is part of GaspiCommLayer.
 *
 * GaspiCommLayer is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * GaspiCommLayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaspiCommLayer. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SEGMENT_RESOURCE_HPP_
#define SEGMENT_RESOURCE_HPP_

#include <segment/SegmentManager.hpp>
#include <type_defs.hpp>

namespace gaspi {
namespace segment {

class SegmentResource
{
  public:

    using SegmentID = SegmentManager::SegmentID;

    SegmentResource
      ( std::size_t segmentSize );

    SegmentResource
      ( SegmentID segmentId
      , std::size_t segmentSize );

    ~SegmentResource
      ();

    SegmentID
    id
      () const;

    void
    remoteRegistration
      ( Rank );

    static SegmentID
    getFreeLocalSegmentId();

private:

    SegmentID _segmentId;

};

}
}

#endif /* SEGMENT_RESOURCE_HPP_ */
