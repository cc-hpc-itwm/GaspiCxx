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
 * Queue.hpp
 *
 */

#ifndef QUEUE_HPP_
#define QUEUE_HPP_

extern "C" {
#include <GASPI.h>
}

namespace gaspi {
namespace singlesided {

class Queue
{
  private:

    gaspi_queue_id_t _queue_id;

  public:

    Queue();
    virtual
    ~Queue();

    void
    flush
      () const;

    gaspi_queue_id_t const &
    get
      () const;
};

} /* namespace singlesided */
} /* namespace gaspi */

#endif /* QUEUE_HPP_ */
