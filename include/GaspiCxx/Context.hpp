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
 * Context.hpp
 *
 */

#ifndef COMMUNICATOR_HPP_
#define COMMUNICATOR_HPP_

#include <memory>
#include <GaspiCxx/type_defs.hpp>

// fowrward declarations
namespace gaspi {

namespace group {

class Group;
class Rank;

}

namespace singlesided {

class BufferDescription;
class Queue;

}
}


namespace gaspi {


class Context
{
  private:

    //! The rank of "this" process
    gaspi_rank_t _rank;

    std::unique_ptr<group::Group>       _pGroup;
    std::unique_ptr<singlesided::Queue> _pQueue;

    //! A communicator cannot be copied.
    Context
      (const Context&) = delete;

    Context&
    operator=
      (const Context&) = delete;

  public:

    // default context, all ranks
    Context
      ();

    Context
      (group::Group && group);

    virtual
    ~Context();

    /// Returns the rank of this process in the communicator
    group::Rank
    rank
      () const;

    /// Returns the size of this communicator
    group::Rank
    size
      () const;

    group::Group const &
    group
      () const;

    void
    write
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription ) const;

    void
    writePart
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription
      , std::size_t size
      , std::size_t offset ) const;

    void
    notify
      ( singlesided::BufferDescription targetBufferDescription ) const;

    bool
    checkForBufferNotification
      ( singlesided::BufferDescription targetBufferDescription ) const;

    bool
    waitForBufferNotification
      ( singlesided::BufferDescription targetBufferDescription ) const;

    void
    flush
      () const;

    /// Collective barrier call for all processes in `this` communicator.
    void
    barrier() const;
};

} /* namespace gaspi */

#endif /* COMMUNICATOR_HPP_ */
