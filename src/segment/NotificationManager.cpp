/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
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
 * NotificationManager.cpp
 *
 */

#include <GaspiCxx/segment/NotificationManager.hpp>
#include <stdexcept>
#include <GaspiCxx/utility/LockGuard.hpp>

namespace gaspi {
namespace segment {

NotificationManager
  ::NotificationManager
    ( std::size_t  notification_size ) throw()
{
  NotificationBlock b;
  b.g_start = 0;
  b.size  = notification_size;
  b.free  = true;
  _blocks.push_back(b);

  pthread_mutex_init(&_mutex, NULL);
}

NotificationManager
  ::~NotificationManager()
{
  pthread_mutex_destroy(&_mutex);
}

size_t
NotificationManager
  ::allocate( std::size_t size)
{
  if ( size == 0 ) {
    throw std::runtime_error("Requested 0 notifications");
  }

  LockGuard guard(_mutex);

  for(std::list<NotificationBlock>::iterator it = _blocks.begin();
      it != _blocks.end();
      it++) {
    if(it->free && it->size >= size) {

      if(it->size > size ) {
	NotificationBlock b;
	b.g_start = it->g_start;
	b.size  = size;
	b.free  = false;
	_blocks.insert(it, b);
	it->g_start
	  = it->g_start + size;
	it->size -= size;

	// Go back to the new memory block.
	it--;
      }
      else
      {
	it->free = false;
      }

      return it->g_start;
    }
  }

  // Not enough free memory!
  throw std::bad_alloc();
}

 void
NotificationManager
  ::deallocate( size_t g_ptr )
{
//   if(g_ptr == NULL) {
//     return;
//   }

   LockGuard guard(_mutex);
   for(std::list<NotificationBlock>::iterator it = _blocks.begin();
       it != _blocks.end();
       it++) {
     if(it->g_start == g_ptr) {
       // Is the next block free?
       std::list<NotificationBlock>::iterator next = it;
       next++;
       if(next != _blocks.end() && next->free) {
         // Merge with next block
         it->size += next->size;
         _blocks.erase(next);
       }

       // Is the previous block free?
       if(it != _blocks.begin()) {
         std::list<NotificationBlock>::iterator prev = it;
         prev--;
         if(prev->free) {
           // Merge with previous block
           it->g_start = prev->g_start;
           it->size += prev->size;
           _blocks.erase(prev);
         }
       }
       it->free = true;
       return;
     }
   }

   throw std::bad_alloc();
}

std::ostream&
operator<<
  ( std::ostream& os
  , const NotificationManager& man)
{
  typedef std::list<NotificationManager::NotificationBlock>
    ::const_iterator Iter;

  int iblock(0);
  for( Iter it ( man._blocks.begin() )
     ;      it != man._blocks.end()
     ;    ++it) {

    os << "block " << iblock << ": "
       << "start " << it->g_start << ", "
       << "size " << it->size << ", "
       << "state " << ( it->free ? "free" : "alloc'd" )
      << std::endl;

    ++iblock;
  }

  return os;
}

}
}


