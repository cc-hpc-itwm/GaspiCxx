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
 * CommBuffer.hpp
 *
 */

#ifndef COMMBUFFER_HPP_
#define COMMBUFFER_HPP_

#include <cstdlib>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

class CommBuffer {

  public:

    using Tag = int;

    class ConnectHandle {

      public :

        ConnectHandle
          ( CommBuffer & commBuffer
          , std::unique_ptr<TargetBuffer> pSendBuffer
          , std::unique_ptr<TargetBuffer> pRecvBuffer );

        void
        waitForCompletion();

      private:

        CommBuffer & _commBuffer;
        std::unique_ptr<TargetBuffer> _pSendBuffer;
        std::unique_ptr<TargetBuffer> _pRecvBuffer;

    };

    CommBuffer
      ( segment::Segment & segment
      , std::size_t size );

    CommBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    CommBuffer
      ( segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    CommBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    ~CommBuffer
      ();

    BufferDescription
    description
      () const;

    void *
    address
      () const;

    // bilateral function
    // needs to be invoked by the correspondent
    // WriteTargetBuffer having the same size
    ConnectHandle
    connectToRemotePartner
      ( Context & context
      , group::Rank & rank
      , Tag & tag );

  protected:

    bool                _allocMemory;
    bool                _allocNotify;

    void * const        _pointer;
    std::size_t         _size;
    segment::Segment
      ::Notification    _notification;
    segment::Segment &  _segment;

    BufferDescription   _localBufferDesc;
    BufferDescription   _otherBufferDesc;

};

} // namespace write
} // namespace singlesided
} // namespace gaspi

#endif /* COMMBUFFER_HPP_ */
