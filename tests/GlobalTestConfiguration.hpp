/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
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
 */


#ifndef GLOBALTESTCONFIGURATION_HPP
#define GLOBALTESTCONFIGURATION_HPP

#include <GASPI.h>
#include <GaspiCxx/Runtime.hpp>

const gaspi_size_t DEFAULT_SEGMENT_SIZE = 1024*10;
const int DEFAULT_NUM_THREADS = 4;

// This class initializes GASPI and a segment on
// a group of processes. It is used to set up
// GASPI in all tests.
class GlobalTestConfiguration
{
private:

  gaspi::Runtime &     _runtime;

//  gaspi_group_t      _group;
//  bool               _customGroup;
//  gaspi_segment_id_t _segmentID;
//  gaspi_size_t       _segmentSize;
//  gaspi_queue_id_t   _queue;
//  int                _numThreads;

public:
  GlobalTestConfiguration(int argc, char *argv[]);
  ~GlobalTestConfiguration();

//  gaspi_group_t      group() const;
//  gaspi_rank_t       groupRank() const;
//  gaspi_number_t     groupSize() const;
//  gaspi_segment_id_t segmentID() const;
//  gaspi_size_t       segmentSize() const;
//  gaspi_queue_id_t   queue() const;
//  int                numThreads() const;
};

#endif // GLOBALTESTCONFIGURATION_HPP

