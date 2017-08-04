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


#include <GlobalTestConfiguration.hpp>
#include <cstring>
#include <cstdlib>
#include <ctime>

GlobalTestConfiguration
  ::GlobalTestConfiguration
   ( int /*argc*/
   , char */*argv*/[] )
: _runtime()
{
//  // Initialize with default configuration.
//  _numThreads  = DEFAULT_NUM_THREADS;
//  _group       = GASPI_GROUP_ALL;
//  _customGroup = false;
//  _segmentID   = 1;
//  _segmentSize = DEFAULT_SEGMENT_SIZE;
//  _queue       = 0;
//
//  // Read configuration from command line...
//  for(int i = 1; i < argc; i += 2) {
//    // read number of threads
//    if(std::strcmp(argv[i], "--num_threads") == 0) {
//      _numThreads = atoi(argv[i + 1]);
//    }
//    // read segment ID
//    if(std::strcmp(argv[i], "--segment_id") == 0) {
//      _segmentID = atoi(argv[i + 1]);
//    }
//    // read segment size
//    if(std::strcmp(argv[i], "--segment_size") == 0) {
//      _segmentSize = atol(argv[i + 1]);
//    }
//    // read GASPI queue
//    if(std::strcmp(argv[i], "--queue") == 0) {
//      _queue = atoi(argv[i + 1]);
//    }
//    // read group
//    if(std::strcmp(argv[i], "--group") == 0) {
//      // all ranks in group
//      if(std::strcmp(argv[i + 1], "all") == 0) {
//        _group = GASPI_GROUP_ALL;
//      }
//      // even ranks in group
//      if(std::strcmp(argv[i + 1], "even") == 0) {
//        _customGroup = true;
//        gaspi_group_create(&_group);
//        gaspi_rank_t n;
//        gaspi_proc_num(&n);
//        for(gaspi_rank_t r = 0; r < n; r += 2) {
//          gaspi_group_add(_group, r);
//        }
//        gaspi_group_commit(_group, GASPI_BLOCK);
//      }
//      // odd ranks in group
//      if(std::strcmp(argv[i + 1], "odd") == 0) {
//        _customGroup = true;
//        gaspi_group_create(&_group);
//        gaspi_rank_t n;
//        gaspi_proc_num(&n);
//        for(gaspi_rank_t r = 1; r < n; r += 2) {
//          gaspi_group_add(_group, r);
//        }
//        gaspi_group_commit(_group, GASPI_BLOCK);
//      }
//      // random ranks in group
//      if(std::strcmp(argv[i + 1], "random") == 0) {
//        _customGroup = true;
//        gaspi_group_create(&_group);
//        gaspi_rank_t n;
//        gaspi_proc_num(&n);
//        std::srand(std::time(NULL));
//        for(gaspi_rank_t r = 1; r < n; r++) {
//          if(std::rand() % 2) {
//            gaspi_group_add(_group, r);
//          }
//        }
//        gaspi_group_commit(_group, GASPI_BLOCK);
//      }
//    }
//  }
//
//  gaspi_segment_create(_segmentID,
//                       _segmentSize,
//                       _group,
//                       GASPI_BLOCK,
//                       GASPI_MEM_INITIALIZED);
}

GlobalTestConfiguration::~GlobalTestConfiguration()
{
//  gaspi_segment_delete(_segmentID);
//  if(_customGroup) {
//    gaspi_group_delete(_group);
//  }
//  gaspi_proc_term(GASPI_BLOCK);
}

//gaspi_group_t GlobalTestConfiguration::group() const
//{
//  return _group;
//}
//
//gaspi_rank_t GlobalTestConfiguration::groupRank() const
//{
//  gaspi_rank_t r;
//  gaspi_proc_rank(&r);
//  return r;
//}
//
//gaspi_number_t GlobalTestConfiguration::groupSize() const
//{
//  gaspi_number_t s;
//  gaspi_group_size(_group, &s);
//  return s;
//}
//
//gaspi_segment_id_t GlobalTestConfiguration::segmentID() const
//{
//  return _segmentID;
//}
//
//gaspi_size_t GlobalTestConfiguration::segmentSize() const
//{
//  return _segmentSize;
//}
//
//gaspi_queue_id_t GlobalTestConfiguration::queue() const
//{
//  return _queue;
//}
//
//int GlobalTestConfiguration::numThreads() const
//{
//  return _numThreads;
//}

