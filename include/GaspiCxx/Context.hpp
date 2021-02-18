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
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/type_defs.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>

// fowrward declarations
namespace gaspi {

namespace group {
class Rank;
}

namespace singlesided {
class BufferDescription;
}
}


namespace gaspi {


class Context
{
  private:

    group::Group                        _group;
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
      (group::Group const & group);

    virtual
    ~Context() = default;

    /// Returns the rank of this process in the communicator
    group::Rank
    rank
      () const;

    group::GlobalRank
    global_rank
      () const;

    /// Returns the size of this communicator
    std::size_t
    size
      () const;

    group::Group
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
};

} /* namespace gaspi */

#endif /* COMMUNICATOR_HPP_ */
