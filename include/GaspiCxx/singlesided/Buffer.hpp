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
 * Buffer.hpp
 *
 */

#ifndef BUFFER_HPP_
#define BUFFER_HPP_

#include <cstdlib>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/utility/ScopedAllocation.hpp>

namespace gaspi {
namespace singlesided {

class MemoryAllocation;
class NotifyAllocation;

class Buffer {

  public:

    Buffer
      ( segment::Segment & segment
      , std::size_t size );

    Buffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    Buffer
      ( segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    Buffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment::Segment
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

    std::unique_ptr<MemoryAllocation>  _allocMemory;
    std::unique_ptr<NotifyAllocation>  _allocNotify;

    void * const        _pointer;
    std::size_t         _size;
    segment::Segment
      ::Notification    _notification;
    segment::Segment &  _segment;

};

} // namespace singlesided
} // namespace gaspi

#endif /* BUFFER_HPP_ */
