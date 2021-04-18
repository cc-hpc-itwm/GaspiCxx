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
 * SegmentManager.hpp
 *
 */

#ifndef SEGMENT_MANAGER_HPP_
#define SEGMENT_MANAGER_HPP_

#include <memory>
#include <GaspiCxx/segment/Allocator.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/Types.hpp>

namespace gaspi {
namespace segment {

class SegmentManager
{
  public:

    SegmentManager
      () = delete;

    SegmentManager
      ( SegmentID segmentID );

    SegmentManager
      ( SegmentManager const& ) = delete;

    SegmentManager
      ( SegmentManager&& );

    virtual ~SegmentManager
      () = default;

    SegmentID
    id
      () const;

    std::size_t
    size
      () const;

    std::size_t
    pointerToOffset
      ( void const * const ) const;

    Allocator<char>
    allocator
      ();

    bool hasFreeMemory( std::size_t );

    Notification
    acquire_notification
      ();

    void
    release_notification
      (Notification const & notification);

private:

    SegmentID                            _segmentID;
    std::unique_ptr<MemoryManager>       _memoryManager;
    std::unique_ptr<NotificationManager> _notifyManager;

};

}
}

#endif /* SEGMENT_HPP_ */
