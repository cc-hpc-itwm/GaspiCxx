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

#ifndef SEGMENT_HPP_
#define SEGMENT_HPP_

#include <segment/SegmentManager.hpp>
#include <segment/SegmentResource.hpp>

namespace gaspi {
namespace segment {

class Segment : public SegmentResource
              , public SegmentManager
{
  public:

    using SegmentID = SegmentManager::SegmentID;
    using Notification = SegmentManager::Notification;

    Segment
      () = delete;

    Segment
      ( std::size_t );

    Segment
      ( SegmentID
      , std::size_t );

    SegmentID
    id
      () const
    {
      return SegmentResource::id();
    }

private:

};

}
}

#endif /* SEGMENT_HPP_ */
