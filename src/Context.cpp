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
 * Context.cpp
 *
 */

#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {

Context
  ::Context
    ()
: Context(group::Group())
{

}

Context
  ::Context
    (group::Group && group)
: _rank(0)
, _pGroup(std::make_unique<group::Group>(std::move(group)))
, _pQueue(std::make_unique<singlesided::Queue>())
{
  GASPI_CHECK
    (gaspi_proc_rank(&_rank));
}

Context
  ::~Context
   ()
{

}

group::Group const &
Context
  ::group() const
{
  return *_pGroup;
}

/// Returns the rank of this process in the communicator
group::Rank
Context
  ::rank
    () const
{
  return _pGroup->rank();
}

/// Returns the size of this communicator
group::Rank
Context
  ::size
    () const
{
  return _pGroup->size();
}

void
Context
  ::write
    ( singlesided::BufferDescription sourceBufferDescription
    , singlesided::BufferDescription targetBufferDescription ) const
{
  if( sourceBufferDescription.size() >
      targetBufferDescription.size() ) {

    std::stringstream ss;

    ss << CODE_ORIGIN
       << "size of source buffer ("
       << sourceBufferDescription.size()
       << " byte) > size of target buffer ("
       << targetBufferDescription.size()
       << " byte)";

    throw std::runtime_error
      (ss.str());
  }

  if( sourceBufferDescription.size() > 0 ) {

    gaspi_return_t ret(GASPI_ERROR);

    while( ( ret =  gaspi_write_notify
                     ( sourceBufferDescription.segmentId()
                     , sourceBufferDescription.offset()
                     , targetBufferDescription.rank()
                     , targetBufferDescription.segmentId()
                     , targetBufferDescription.offset()
                     , targetBufferDescription.size()
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
Context
  ::notify
    ( singlesided::BufferDescription targetBufferDescription ) const
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

bool
Context
  ::checkForBufferNotification
   ( singlesided::BufferDescription targetBufferDescription ) const
{
  bool ret(false);

  gaspi_notification_id_t id;
  gaspi_notification_t    value;

  gaspi_return_t gaspi_return
    (gaspi_notify_waitsome
       ( targetBufferDescription.segmentId()
       , targetBufferDescription.notificationId()
       , 1
       , &id
       , GASPI_TEST ) );

  if( gaspi_return != GASPI_TIMEOUT) {

    GASPI_CHECK
      (gaspi_return);

    GASPI_CHECK
      (gaspi_notify_reset
         ( targetBufferDescription.segmentId()
         , id
         , &value) );

    if( value != 0 ) {
      ret = true;
    }
  }

  return ret;
}

bool
Context
  ::waitForBufferNotification
   ( singlesided::BufferDescription targetBufferDescription ) const
{
  bool ret(false);

  gaspi_notification_id_t id;
  gaspi_notification_t    value;

  GASPI_CHECK
    (gaspi_notify_waitsome
       ( targetBufferDescription.segmentId()
       , targetBufferDescription.notificationId()
       , 1
       , &id
       , GASPI_BLOCK ) );

  GASPI_CHECK
    (gaspi_notify_reset
       ( targetBufferDescription.segmentId()
       , id
       , &value) );

  if( value != 0 ) {
    ret = true;
  }

  return ret;
}


void
Context
  ::flush
   () const
{
  _pQueue->flush();
}

/// Collective barrier call for all processes in `this` communicator.
void
Context
  ::barrier
    () const
{
  if(size().get() > 1) {
    GASPI_CHECK
      (gaspi_barrier(_pGroup->group(), GASPI_BLOCK));
  }
}

} /* namespace gaspi */
