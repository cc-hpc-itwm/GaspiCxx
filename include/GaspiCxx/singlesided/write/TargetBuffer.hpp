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
 * TargetBuffer.hpp
 *
 */

#ifndef TARGETBUFFER_HPP_
#define TARGETBUFFER_HPP_

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/singlesided/Endpoint.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

class TargetBuffer : public Endpoint {

  public:

    using Tag = int;

    TargetBuffer
      ( std::size_t size );

    TargetBuffer
      ( segment::Segment & segment
      , std::size_t size );

    TargetBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    TargetBuffer
      ( std::size_t size
      , segment
          ::Notification notification );

    TargetBuffer
      ( segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification );

    TargetBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification );

    // Copy constructor creates a new `TargetBuffer`,
    // which points to the same (segment) memory,
    // but creates new notifications
    TargetBuffer(TargetBuffer const&);

    // bilateral function
    // needs to be invoked by the correspondent
    // WriteTargetBuffer having the same size
    Endpoint::ConnectHandle
    connectToRemoteSource
      ( group::Group const&
      , group::Rank const&
      , Tag const& );

    bool
    waitForCompletion
      ();

    bool
    checkForCompletion
      ();

    void
    ackTransfer
      ();
};

} // namespace write
} // namespace singlesided
} // namespace gaspi

#endif /* TARGETBUFFER_HPP_ */
