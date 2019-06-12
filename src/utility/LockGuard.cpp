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
 * LockGuard.cpp
 *
 */

#include <GaspiCxx/utility/LockGuard.hpp>

namespace gaspi {

LockGuard
  ::LockGuard(pthread_mutex_t &mutex)
: _mutex(&mutex)
{
  pthread_mutex_lock(_mutex);
}

LockGuard::~LockGuard()
{
  pthread_mutex_unlock(_mutex);
}

}

