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
