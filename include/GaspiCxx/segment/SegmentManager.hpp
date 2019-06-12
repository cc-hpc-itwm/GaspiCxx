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

// forward declarations

namespace gaspi {
namespace segment {

template <typename T>
class Allocator;

class MemoryManager;
class NotificationManager;

}
}

namespace gaspi {
namespace segment {

class SegmentManager
{
  public:

    using SegmentID = int;
    using Notification = int;

    SegmentManager
      () = delete;

    SegmentManager
      ( SegmentID segmentID );

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
