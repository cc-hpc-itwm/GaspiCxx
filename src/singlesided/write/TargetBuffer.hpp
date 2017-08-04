/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiLS.
 * 
 * GaspiLS is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 * 
 * GaspiLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * TargetBuffer.hpp
 *
 */

#ifndef TARGETBUFFER_HPP_
#define TARGETBUFFER_HPP_

#include <Context.hpp>
#include <group/Rank.hpp>
#include <singlesided/BufferDescription.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

class TargetBuffer {

  public:

    using Tag = int;

    // Allocates a buffer of
    TargetBuffer
      ( segment::Segment & segment
      , std::size_t size );

    TargetBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    TargetBuffer
      ( segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    TargetBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    ~TargetBuffer
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
    std::unique_ptr<TargetBuffer>
    connectToRemoteSource
      ( Context & context
      , group::Rank & rank
      , Tag & tag );

    void
    waitForCompletion
      ();

    bool
    checkForCompletion
      ();

  private:

    bool                _allocMemory;
    bool                _allocNotify;

    void * const        _pointer;
    std::size_t         _size;
    segment::Segment
      ::Notification    _notification;

    segment::Segment &  _segment;

};

} // namespace write
} // namespace singlesided
} // namespace gaspi

#endif /* TARGETBUFFER_HPP_ */
