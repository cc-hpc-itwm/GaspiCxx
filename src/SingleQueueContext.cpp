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
 * SingleQueueContext.cpp
 *
 */

#include <GaspiCxx/SingleQueueContext.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {

SingleQueueContext
  ::SingleQueueContext()
: _pQueue(std::make_unique<singlesided::Queue>())
{ }

void
SingleQueueContext
  ::writePart
    ( singlesided::BufferDescription sourceBufferDescription
    , singlesided::BufferDescription targetBufferDescription
    , std::size_t size
    , std::size_t offset )
{
  if( (offset + size) > sourceBufferDescription.size() ) {
    std::stringstream ss;

    ss << CODE_ORIGIN
       << "Offset ("
       << offset
       << " byte) + size ("
       << size
       << " byte) > size of source buffer ("
       << sourceBufferDescription.size()
       << " byte)";

    throw std::runtime_error
      (ss.str());
  }

  if( size > targetBufferDescription.size() ) {
    std::stringstream ss;

    ss << CODE_ORIGIN
       << "size ("
       << size
       << " byte) > size of target buffer ("
       << targetBufferDescription.size()
       << " byte)";

    throw std::runtime_error
      (ss.str());
  }

  if( size > 0 ) {

    gaspi_return_t ret(GASPI_ERROR);

    while( ( ret =  gaspi_write_notify
                     ( sourceBufferDescription.segmentId()
                     , sourceBufferDescription.offset() + offset
                     , targetBufferDescription.rank()
                     , targetBufferDescription.segmentId()
                     , targetBufferDescription.offset()
                     , size
                     , targetBufferDescription.notificationId()
                     , 1
                     , _pQueue->get()
                     , GASPI_BLOCK) ) == GASPI_QUEUE_FULL ) {
      _pQueue->flush();
    }

    try {
      GASPI_CHECK(ret);
    } catch(std::runtime_error & e) {

      std::stringstream ss;

      ss << CODE_ORIGIN
         << e.what();
      ss << "! source: " << sourceBufferDescription
         << ", target: " << targetBufferDescription;

      throw std::runtime_error(ss.str());
    }

  }
  else {
    notify( targetBufferDescription );
  }

}

void
SingleQueueContext
  ::notify
    ( singlesided::BufferDescription targetBufferDescription )
{
  gaspi_return_t ret(GASPI_ERROR);

  while( ( ret =  gaspi_notify
                   ( targetBufferDescription.segmentId()
                   , targetBufferDescription.rank()
                   , targetBufferDescription.notificationId()
                   , 1
                   , _pQueue->get()
                   , GASPI_BLOCK) ) == GASPI_QUEUE_FULL ) {
    _pQueue->flush();
  }

  try {
    GASPI_CHECK(ret);
  } catch(std::runtime_error & e) {

    std::stringstream ss;

    ss << CODE_ORIGIN
       << e.what();
    ss << "! target: " << targetBufferDescription;

    throw std::runtime_error(ss.str());
  }

}

void
SingleQueueContext
  ::flush
   ()
{
  _pQueue->flush();
}

} /* namespace gaspi */
