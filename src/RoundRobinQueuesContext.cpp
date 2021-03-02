#include <GaspiCxx/RoundRobinQueuesContext.hpp>
#include <GaspiCxx/utility/Macros.hpp>

#include <algorithm>

namespace gaspi
{
  namespace
  {
    std::size_t get_number_allocated_gaspi_queues()
    {
      gaspi_number_t number_queues;
      GASPI_CHECK(gaspi_queue_num(&number_queues));
      return static_cast<std::size_t>(number_queues);
    }

    std::size_t get_number_queues_allowed(std::size_t num_queues)
    {
      gaspi_number_t max_number_queues_allowed;
      GASPI_CHECK(gaspi_queue_max(&max_number_queues_allowed));

      auto number_queues_allowed = max_number_queues_allowed -
                                    get_number_allocated_gaspi_queues();
      if (num_queues > number_queues_allowed)
      {
        throw std::runtime_error("[RoundRobinQueuesContext] Cannot create " +
                                  std::to_string(num_queues) + " queues");
      }
      return static_cast<std::size_t>(num_queues);
    }
  }

  // Assumptions:
  // * there might be a number of preallocated queues
  // * the `QueueManager` will not use any preallocated queues
  // * it will allocate a number of queues until it reaches the
  //   maximum number of allowed queues
  RoundRobinQueuesContext::RoundRobinQueuesContext(std::size_t num_queues)
  : num_queues(get_number_queues_allowed(num_queues)),
    gaspi_queues(num_queues),
    queue_full_counter(0)
  {
    if (num_queues <= 0)
    {
      throw std::runtime_error(
      "[RoundRobinQueuesContext] Context creation requires at least one queue");
    }
  }

  RoundRobinQueuesContext::~RoundRobinQueuesContext()
  {
    // deallocate queues in decreasing ID order as required by
    // the GPI implementation
    while(!gaspi_queues.empty())
    {
      gaspi_queues.pop_back();
    }
  }


  void
  RoundRobinQueuesContext
    ::writePart
      ( singlesided::BufferDescription sourceBufferDescription
      , singlesided::BufferDescription targetBufferDescription
      , std::size_t size
      , std::size_t offset )
  {
    if (size == 0)
    {
      notify(targetBufferDescription);
      return;
    }
    if (offset + size > sourceBufferDescription.size())
    {
      throw std::runtime_error(
      "[RoundRobinQueuesContext:writePart] Write size larger than provided source buffer");
    }
    if (offset + size > targetBufferDescription.size())
    {
      throw std::runtime_error(
      "[RoundRobinQueuesContext:writePart] Write size larger than provided target buffer");
    }
    
    gaspi_notification_t const notification_value = 1;
    while (true)
    {
      auto ret = gaspi_write_notify(sourceBufferDescription.segmentId(),
                                    sourceBufferDescription.offset() + offset,
                                    targetBufferDescription.rank(),
                                    targetBufferDescription.segmentId(),
                                    targetBufferDescription.offset(),
                                    size,
                                    targetBufferDescription.notificationId(),
                                    notification_value,
                                    get_queue().get(),
                                    GASPI_BLOCK);

      if (ret == GASPI_SUCCESS) { break; }
      if (ret == GASPI_QUEUE_FULL) { select_available_queue(); }
      else { GASPI_CHECK(ret); }
    }
  }


  void
  RoundRobinQueuesContext
    ::notify
      ( singlesided::BufferDescription targetBufferDescription )
  {
    gaspi_notification_t const notification_value = 1;      
    while (true)
    {
      auto ret = gaspi_notify(targetBufferDescription.segmentId(),
                              targetBufferDescription.rank(),
                              targetBufferDescription.notificationId(),
                              notification_value,
                              get_queue().get(),
                              GASPI_BLOCK);

      if (ret == GASPI_SUCCESS) { break; }
      if (ret == GASPI_QUEUE_FULL) { select_available_queue(); }
      else { GASPI_CHECK(ret); }
    }
  }

  singlesided::Queue& RoundRobinQueuesContext::get_queue()
  {
    return gaspi_queues.at(queue_full_counter % gaspi_queues.size());
  }
  
  void RoundRobinQueuesContext::select_available_queue()
  {
    // use a counter variable instead of an index in the queues array
    // to be able to use an atomic variable
    // this might result in skipping over queues in a multi-threaded environment
    queue_full_counter++;
    get_queue().flush();
  }

  void RoundRobinQueuesContext::flush()
  {
    for(auto& q : gaspi_queues) q.flush();
  }
}
