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
 * CommunicationContext.hpp
 *
 */

#ifndef COMMUNICATOR_HPP_
#define COMMUNICATOR_HPP_

#include <memory>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>

namespace gaspi {

class CommunicationContext
{
  private:

    //! A communicator cannot be copied.
    CommunicationContext
      (const CommunicationContext&) = delete;

    CommunicationContext&
    operator=
      (const CommunicationContext&) = delete;

  public:

    // default context, all ranks
    CommunicationContext
      ();

    virtual
    ~CommunicationContext() = default;

    void
    write
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription );

    bool
    checkForBufferNotification
      ( singlesided::BufferDescription targetBufferDescription ) const;

    bool
    waitForBufferNotification
      ( singlesided::BufferDescription targetBufferDescription ) const;

    virtual void
    writePart
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription
      , std::size_t size
      , std::size_t offset ) = 0;

    virtual void
    notify
      ( singlesided::BufferDescription targetBufferDescription ) = 0;

    virtual void
    flush
      () = 0;
};

} /* namespace gaspi */

#endif /* COMMUNICATOR_HPP_ */
