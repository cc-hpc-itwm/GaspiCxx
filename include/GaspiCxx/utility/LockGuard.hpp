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
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * LockGuard.hpp
 *
 */

#ifndef LOCKGUARD_HPP_
#define LOCKGUARD_HPP_

#include <pthread.h>

namespace gaspi {

//! Simple lock guard for Pthread mutexes
class LockGuard
{
public:
  LockGuard(pthread_mutex_t &mutex);

  ~LockGuard();

private:
  pthread_mutex_t *_mutex;
};

}

#endif /* LOCKGUARD_HPP_ */
