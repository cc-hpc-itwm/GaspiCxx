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
 * Buffer.hpp
 *
 */

#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <cstdlib>

#include <GaspiCxx/segment/Types.hpp>

// forward declaration
namespace gaspi {

namespace segment {

class Segment;

}

namespace singlesided {

class BufferDescription;
class MemoryAllocation;
class NotifyAllocation;

}
}


namespace gaspi {
namespace singlesided {

class Buffer {

  public:

    Buffer
      ( std::size_t size );

    Buffer
      ( segment::Segment & segment
      , std::size_t size );

    Buffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    Buffer
      ( std::size_t size
      , segment
          ::Notification notification );

    Buffer
      ( segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification );

    Buffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification );

    ~Buffer
      ();

    BufferDescription
    description
      () const;

    void *
    address
      () const;

    // Checks for notification
    // return true if thread got notification (only a single thread gets the
    //        notification)
    bool
    checkForNotification
      ();

    // Waits for notification
    // return true if thread got notification (only a single thread gets the
    //        notification)
    bool
    waitForNotification
      ();

  protected:

    std::shared_ptr<MemoryAllocation>  _allocMemory;
    std::unique_ptr<NotifyAllocation>  _allocNotify;

    void * const        _pointer;
    std::size_t         _size;
    segment
      ::Notification    _notification;
    segment::Segment &  _segment;

};

} // namespace singlesided
} // namespace gaspi

#endif /* BUFFER_HPP_ */
