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
 * Enpoint.hpp
 *
 */

#ifndef ENDPOINT_HPP_
#define ENDPOINT_HPP_

#include <cstdlib>
#include <memory>

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>

// forward declarations

namespace gaspi {

namespace singlesided {

class BufferDescription;

}

}

namespace gaspi {
namespace singlesided {

class Endpoint : public Buffer {

  public:

    using Tag = unsigned int;

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

    enum Type {SOURCE, TARGET, GENERIC};

    Endpoint
      ( std::size_t size
      , Type type = GENERIC );

    Endpoint
      ( segment::Segment & segment
      , std::size_t size
      , Type type = GENERIC );

    Endpoint
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , Type type = GENERIC);

    Endpoint
      ( std::size_t size
      , segment
          ::Notification notification
      , Type type = GENERIC);

    Endpoint
      ( segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification
      , Type type = GENERIC);

    Endpoint
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment
          ::Notification notification
      , Type type = GENERIC);

    // Copy constructor creates a new `Endpoint`
    // from an existing one that will point
    // to the same (segment) memory,
    // but creates new notifications
    Endpoint(Endpoint const&);

    Endpoint& operator=(const Endpoint&) = delete;
    virtual ~Endpoint() override = default;

    void
    setRemotePartner
      ( BufferDescription const & partnerDescription );

    // bilateral function
    // needs to be invoked by the correspondent
    // WriteTargetBuffer having the same size
    ConnectHandle
    connectToRemotePartner
      ( group::Group const& group
      , group::Rank const& rank
      , Tag const& tag );

    bool
    isConnected
      () const;

  protected:

    Type _type;

    BufferDescription &
    localBufferDesc();

    BufferDescription const &
    localBufferDesc() const;

    BufferDescription &
    otherBufferDesc();

    BufferDescription const &
    otherBufferDesc() const;

  private:

    std::unique_ptr<BufferDescription>   _pLocalBufferDesc;
    std::unique_ptr<BufferDescription>   _pOtherBufferDesc;
    bool                                 _isConnected;

};

} // namespace singlesided
} // namespace gaspi

#endif /* COMMBUFFER_HPP_ */
