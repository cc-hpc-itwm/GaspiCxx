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
 * SourceBuffer.hpp
 *
 */

#ifndef SOURCEBUFFER_HPP_
#define SOURCEBUFFER_HPP_

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/singlesided/Endpoint.hpp>

// forward declarations
namespace gaspi {

namespace group { class Rank; }

}

namespace gaspi {
namespace singlesided {
namespace write {

class SourceBuffer : public Endpoint {

  public:

    using Tag = unsigned int;

    SourceBuffer
      ( std::size_t size );

    SourceBuffer
      ( segment::Segment & segment
      , std::size_t size );

    SourceBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    SourceBuffer
      ( std::size_t size
      , segment
          ::Notification notification );

    SourceBuffer
      ( segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification );

    SourceBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification );

    SourceBuffer(Endpoint const&);

    SourceBuffer
      (const SourceBuffer&);
    SourceBuffer&
      operator=(const SourceBuffer&) = delete;
    ~SourceBuffer
      () override = default;

    // bilateral function
    // needs to be invoked by the correspondent
    // WriteTargetBuffer having the same size
    Endpoint::ConnectHandle
    connectToRemoteTarget
      ( group::Group const&
      , group::Rank const&
      , Tag const& );

    void
    initTransfer
      ();

    void
    initTransfer
      ( CommunicationContext& );

    void
    initTransferPart
      ( std::size_t size
      , std::size_t offset = 0 );

    void
    initTransferPart
      ( CommunicationContext& 
      , std::size_t size
      , std::size_t offset = 0 );

    bool
    checkForTransferAck
      ();

    bool
    waitForTransferAck
      ();

};

} // namespace write
} // namespace singlesided
} // namespace gaspi

#endif /* SOURCEBUFFER_HPP_ */
