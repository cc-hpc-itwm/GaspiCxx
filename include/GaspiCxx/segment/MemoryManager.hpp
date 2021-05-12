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
 * MemoryManger.hpp
 *
 */

#ifndef MEMORYSEGMENTMANAGER_HPP_
#define MEMORYSEGMENTMANAGER_HPP_

#include <pthread.h>
#include <list>
#include <ostream>

namespace gaspi {
namespace segment {

class MemoryManager {

public:

  MemoryManager
    ( void * const memory_segment_ptr
    , std::size_t  memory_segment_size );

  ~MemoryManager();

  void *
  allocate
    ( std::size_t size );

  bool
  canAllocate
    ( std::size_t size );

  void
  deallocate
    ( void * ptr
    , std::size_t size );

  friend std::ostream&
  operator<<
    ( std::ostream& os
    , const MemoryManager& man );

private:

  MemoryManager
    ( MemoryManager const & ) = delete;

  MemoryManager &
  operator=
    ( MemoryManager const & ) = delete;

  struct MemoryBlock
  {
         void   *g_start;
         size_t size;
         bool   free;
  };

  std::list<MemoryBlock> _blocks;
  pthread_mutex_t        _mutex;

};

}
}

#endif /* MEMORYSEGMENTMANAGER_HPP_ */
