#pragma once 

#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/singlesided/Queue.hpp>

#include <memory>

namespace gaspi {

class SingleQueueContext : public CommunicationContext
{
  private:
    std::unique_ptr<singlesided::Queue> _pQueue;
  
  public:

    SingleQueueContext
      ();

    virtual
    ~SingleQueueContext() = default;

    void
    writePart
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription
      , std::size_t size
      , std::size_t offset ) const override;

    void
    notify
      ( singlesided::BufferDescription targetBufferDescription ) const override;

    void
    flush
      () const override;
};

} /* namespace gaspi */
