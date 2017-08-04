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
 */

#ifndef SEGMENTNOTIFICATIONMANAGER_HPP_
#define SEGMENTNOTIFICATIONMANAGER_HPP_

#include <pthread.h>
#include <list>
#include <ostream>

namespace gaspi {
namespace segment {

class NotificationManager {

public:

  NotificationManager
    ( std::size_t  num_notification_max ) throw();

  ~NotificationManager();

  std::size_t
  allocate( std::size_t );

  void
  deallocate( std::size_t );

  friend std::ostream&
  operator<<
    ( std::ostream& os
    , const NotificationManager& man );

private:

  struct NotificationBlock
  {
         size_t g_start;
         size_t size;
         bool   free;
  };
  std::list<NotificationBlock> _blocks;
  pthread_mutex_t              _mutex;

};

}
}

#endif /* SEGMENTNOTIFICATIONMANAGER_HPP_ */
