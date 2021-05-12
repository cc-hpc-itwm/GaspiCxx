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
 * SegmentMemoryManagerTest.cpp
 *
 */

#include <stdexcept>
#include <gtest/gtest.h>
#include <memory>
#include <GaspiCxx/segment/MemoryManager.hpp>

namespace gaspi {
namespace segment {

class MemoryManagerTest : public ::testing::Test
{
  protected:

  std::size_t const               _memory_block_size;
  void * const                    _memory_block_pointer;

  MemoryManagerTest()
  : _memory_block_size(1000)
  , _memory_block_pointer
      (reinterpret_cast<void*>(new char[_memory_block_size]))
  { }

  ~MemoryManagerTest()
  {
    delete[] reinterpret_cast<char*>(_memory_block_pointer);
  }
};

TEST_F(MemoryManagerTest, RandomDeallocate)
{
  MemoryManager memory_manager
    ( _memory_block_pointer
    , _memory_block_size );

  void *p1 = memory_manager.allocate(1);
  void *p2 = memory_manager.allocate(1);
  void *p3 = memory_manager.allocate(1);


  memory_manager.deallocate(p2,1);
  memory_manager.deallocate(p1,1);
  memory_manager.deallocate(p3,1);

  void * p4(nullptr);
  EXPECT_NO_THROW ( p4 = memory_manager.allocate(_memory_block_size) );

  memory_manager.deallocate(p4,_memory_block_size);
}

TEST_F(MemoryManagerTest, fragmentation)
{
  MemoryManager memory_manager
    ( _memory_block_pointer
    , _memory_block_size );

  void *p1 = memory_manager.allocate(1);
  void *p2 = memory_manager.allocate(1);
  void *p3 = memory_manager.allocate(1);
  memory_manager.deallocate(p2,1);
  void *p4 = memory_manager.allocate(1);
  // Make sure that the gap between p1 and p3 is re-used.
  EXPECT_EQ(p2, p4);

  memory_manager.deallocate(p1,1);
  memory_manager.deallocate(p3,1);
  memory_manager.deallocate(p4,1);
}

TEST_F(MemoryManagerTest, outOfMemory)
{
  MemoryManager memory_manager
      ( _memory_block_pointer
      , _memory_block_size );

  EXPECT_THROW( memory_manager.allocate(_memory_block_size + 1)
              , std::bad_alloc );
}

TEST_F(MemoryManagerTest, InconsistentDeallocation)
{
  MemoryManager memory_manager
      ( _memory_block_pointer
      , _memory_block_size );

  void *p1 = memory_manager.allocate(2);

  EXPECT_THROW( memory_manager.deallocate(p1,1)
              , std::runtime_error );

  memory_manager.deallocate(p1,2);
}

TEST_F(MemoryManagerTest, allocateNULL)
{
  MemoryManager memory_manager
        ( _memory_block_pointer
        , _memory_block_size );

  void * p1(memory_manager.allocate(0));

  EXPECT_EQ(p1,nullptr);
}

TEST_F(MemoryManagerTest, deallocateNULL)
{
  MemoryManager memory_manager
        ( _memory_block_pointer
        , _memory_block_size );

  EXPECT_NO_THROW( memory_manager.deallocate(NULL,0) );
}


} // namespace segment
} // namespace gaspi
