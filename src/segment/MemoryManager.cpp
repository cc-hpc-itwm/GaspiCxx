/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
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
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * MemoryManager.cpp
 *
 */

#include <iostream>
#include <GaspiCxx/utility/LockGuard.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <sstream>

namespace gaspi {
namespace segment {

MemoryManager
  ::MemoryManager
    ( void * const memory_segment_ptr
    , std::size_t  memory_segment_size )
{
  MemoryBlock b;
  b.g_start = memory_segment_ptr;
  b.size  = memory_segment_size;
  b.free  = true;
  _blocks.push_back(b);

  pthread_mutex_init(&_mutex, NULL);
}

MemoryManager
  ::~MemoryManager()
{
  pthread_mutex_destroy(&_mutex);

  if( _blocks.size() > 1 ) {
    std::stringstream ss;
    ss << "~MemoryManager: Managed memory still in use at destruction"
       << std::endl
       << *this
       << std::endl;
    std::cerr << ss.str() << std::endl;
  }

  if( !_blocks.front().free ) {
    std::stringstream ss;
    ss << "~MemoryManager: Managed memory still in use at destruction"
       << std::endl
       << *this
       << std::endl;
    std::cerr << ss.str() << std::endl;
  }
}

void *
MemoryManager
  ::allocate( std::size_t size)
{
  if ( size == 0 ) {
    return NULL;
  }

  LockGuard guard(_mutex);

  for(std::list<MemoryBlock>::iterator it = _blocks.begin();
      it != _blocks.end();
      it++) {
    if(it->free && it->size >= size) {

      if(it->size > size ) {
        MemoryBlock b;
        b.g_start = it->g_start;
        b.size  = size;
        b.free  = false;
        _blocks.insert(it, b);
        it->g_start
          = reinterpret_cast<void*>(reinterpret_cast<size_t>(it->g_start)
                                    + size);
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
MemoryManager
  ::deallocate
   ( void * g_ptr
   , std::size_t g_size )
{
   if(g_ptr == NULL) {
     return;
   }

   LockGuard guard(_mutex);
   for(std::list<MemoryBlock>::iterator it = _blocks.begin();
       it != _blocks.end();
       it++) {
     if(it->g_start == g_ptr) {
       // Check that we do not have a memory leak
       if(it->size != g_size ) {
         throw std::runtime_error("Not released");
       }
       // Is the next block free?
       std::list<MemoryBlock>::iterator next = it;
       next++;
       if(next != _blocks.end() && next->free) {
         // Merge with next block
         it->size += next->size;
         _blocks.erase(next);
       }

       // Is the previous block free?
       if(it != _blocks.begin()) {
         std::list<MemoryBlock>::iterator prev = it;
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
  , const MemoryManager& man)
{
  typedef std::list<MemoryManager::MemoryBlock>::const_iterator Iter;

  int iblock(0);
  for( Iter it ( man._blocks.begin() )
     ;      it != man._blocks.end()
     ;    ++it) {

    os << "block " << iblock << ": "
       << "start " << it->g_start << ", "
       << "size " << it->size << ", "
       << "state " << ( it->free ? "free" : "allocated" )
      << std::endl;

    ++iblock;
  }

  return os;
}

}
}
