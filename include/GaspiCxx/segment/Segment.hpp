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
 * Segment.hpp
 *
 */

#ifndef SEGMENT_HPP_
#define SEGMENT_HPP_

#include <GaspiCxx/segment/SegmentManager.hpp>
#include <GaspiCxx/segment/SegmentResource.hpp>
#include <GaspiCxx/segment/Types.hpp>

namespace gaspi {
namespace segment {

class Segment : public SegmentResource
              , public SegmentManager
{
public:

  Segment
    () = delete;

  Segment
    ( std::size_t );

  Segment
    ( SegmentID
    , std::size_t );

  Segment
    ( Segment const& ) = delete;

  Segment
    ( Segment&& ) = default;

  SegmentID
  id
    () const;

private:

};

}
}

#endif /* SEGMENT_HPP_ */
