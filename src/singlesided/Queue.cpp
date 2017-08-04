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
 * Queue.cpp
 *
 */

#include <singlesided/Queue.hpp>
#include <utility/Macros.hpp>

namespace gaspi {
namespace singlesided {

Queue
  ::Queue()
: _queue_id()
{
  GASPI_CHECK
    (gaspi_queue_create
      (&_queue_id,GASPI_BLOCK));
}

Queue
  ::~Queue()
{
  GASPI_CHECK_NOTHROW
    (gaspi_queue_delete(_queue_id));
}

void
Queue
  ::flush
    () const
{
  GASPI_CHECK
    (gaspi_wait(_queue_id, GASPI_BLOCK));
}

gaspi_queue_id_t const &
Queue
  ::get
   () const
{
  return _queue_id;
}

} /* namespace singlesided */
} /* namespace gaspi */
