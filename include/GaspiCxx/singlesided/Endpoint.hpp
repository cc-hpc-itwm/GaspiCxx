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
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Enpoint.hpp
 *
 */

#ifndef ENDPOINT_HPP_
#define ENDPOINT_HPP_

#include <cstdlib>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>


namespace gaspi {
namespace singlesided {


class Endpoint : public Buffer {

  public:

    using Tag = int;

    class ConnectHandle {

      public :

        ConnectHandle
          ( Endpoint & commBuffer
          , std::unique_ptr<Buffer> pSendBuffer
          , std::unique_ptr<Buffer> pRecvBuffer );

        void
        waitForCompletion();

      private:

        Endpoint & _commBuffer;
        std::unique_ptr<Buffer> _pSendBuffer;
        std::unique_ptr<Buffer> _pRecvBuffer;

    };

    Endpoint
      ( segment::Segment & segment
      , std::size_t size );

    Endpoint
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    Endpoint
      ( segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    Endpoint
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    void
    setRemotePartner
      ( BufferDescription const & partnerDescription );

    // bilateral function
    // needs to be invoked by the correspondent
    // WriteTargetBuffer having the same size
    ConnectHandle
    connectToRemotePartner
      ( Context & context
      , group::Rank & rank
      , Tag & tag );

    bool
    isConnected
      () const;

  protected:

    BufferDescription   _localBufferDesc;
    BufferDescription   _otherBufferDesc;
    bool                _isConnected;

};

} // namespace singlesided
} // namespace gaspi

#endif /* COMMBUFFER_HPP_ */
