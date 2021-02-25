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
 * CommunicationContext.cpp
 *
 */

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {

CommunicationContext
  ::CommunicationContext
    ()
{ }

void
CommunicationContext
  ::write
    ( singlesided::BufferDescription sourceBufferDescription
    , singlesided::BufferDescription targetBufferDescription )
{
  writePart
    ( sourceBufferDescription
    , targetBufferDescription
    , sourceBufferDescription.size()
    , 0 );
}

bool
CommunicationContext
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
CommunicationContext
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

} /* namespace gaspi */
