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
 * Collective.hpp
 *
 */

#pragma once

namespace gaspi {
namespace collectives {

//! Generic non-blocking, asynchronous collective interface
//!
//! Design principle:
//! Do GPI-related resource setup only once (in constructor)
//! and execute collective multiple times with different data
class Collective
{
  public:
    //! Start non-blocking collective operation
    //! with data provided in `inputs`
    virtual void start(void const* inputs) = 0;

    //! Blocking wait until the execution is finished
    //! and copy results to `outputs`
    virtual void waitForCompletion(void* outputs) = 0;

    virtual ~Collective() = default;
};

//! Generic non-blocking, asynchronous collective interface
//! for rooted operations, in which the root rank distributes
//! data (e.g., Broadcast, Scatter)
class RootedSendCollective : public Collective
{
  public:
    //! Start non-blocking collective on the root rank
    virtual void start(void const* inputs) = 0;
    //! Start non-blocking collective on all non-root ranks
    virtual void start() = 0;

    virtual ~RootedSendCollective() = default;
};

//! Generic non-blocking, asynchronous collective interface
//! for rooted operations, in which the root rank collects
//! data (e.g., Reduce, Gather)
class RootedReceiveCollective : public Collective
{
  public:
    //! Blocking wait until the execution is finished on the root rank
    virtual void waitForCompletion(void* outputs) = 0;
    //! Blocking wait until the execution is finished on all non-root ranks
    virtual void waitForCompletion() = 0;

    virtual ~RootedReceiveCollective() = default;
};

}
}
