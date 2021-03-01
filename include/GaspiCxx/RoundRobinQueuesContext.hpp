#pragma once

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>
#include <random>
#include <vector>

namespace gaspi {

using QueueID = gaspi_queue_id_t;

class RoundRobinQueuesContext : public CommunicationContext
{
  public:

    RoundRobinQueuesContext(std::size_t);

    RoundRobinQueuesContext(RoundRobinQueuesContext const&) = delete;
    RoundRobinQueuesContext& operator=(RoundRobinQueuesContext const&) = delete;
    ~RoundRobinQueuesContext();

    void
    writePart
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription
      , std::size_t size
      , std::size_t offset ) override;

    void
    notify
      ( singlesided::BufferDescription targetBufferDescription ) override;

    void
    flush
      () override;

  private:
  
    std::size_t const num_queues;
    std::vector<singlesided::Queue> gaspi_queues;
    std::vector<gaspi::singlesided::Queue>::iterator current_queue;

    singlesided::Queue&
    get_queue
      () const;
    
    void
    wait_and_flush_queue
      (singlesided::Queue&);
    
    void reset_queue();
};
}
