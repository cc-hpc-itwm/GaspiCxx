/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
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
 * SourceBuffer.hpp
 *
 */

#ifndef SOURCEBUFFER_HPP_
#define SOURCEBUFFER_HPP_

#include <cstdlib>
#include <Context.hpp>
#include <group/Rank.hpp>
#include <segment/Segment.hpp>
#include <singlesided/BufferDescription.hpp>

namespace gaspi {
namespace singlesided {
namespace write {

class SourceBuffer {

  public:

    using Tag = int;

    SourceBuffer
      ( segment::Segment & segment
      , std::size_t size );

    SourceBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size );

    SourceBuffer
      ( segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    SourceBuffer
      ( void * const ptr
      , segment::Segment & segment
      , std::size_t size
      , segment::Segment
          ::Notification notification );

    ~SourceBuffer
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
    void
    connectToRemoteTarget
      ( Context & context
      , group::Rank & rank
      , Tag & tag );

    void
    initTransfer
      ( Context & context );

  private:

    bool                _allocMemory;
    bool                _allocNotify;

    void * const        _pointer;
    std::size_t         _size;
    segment::Segment
      ::Notification    _notification;
    segment::Segment &  _segment;

    BufferDescription   _targetBufferDesc;

};

} // namespace write
} // namespace singlesided
} // namespace gaspi

#endif /* SOURCEBUFFER_HPP_ */
