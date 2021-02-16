#include <GaspiCxx/segment/SingleSegmentPool.hpp>

#include <stdexcept>

namespace gaspi
{
  namespace segment
  {
    SingleSegmentPool::SingleSegmentPool(std::size_t size)
    : segment(size)
    { }

    SingleSegmentPool::SingleSegmentPool()
    : SingleSegmentPool(1024UL * 1024UL) // 1MiB
    { }

    Segment& SingleSegmentPool::getSegment(std::size_t size)
    {
      if (!segment.hasFreeMemory(size))
      {
        throw std::runtime_error(
          "SingleSegmentPool::getSegment: Not enough memory available");
      }
      return segment;
    }
  }
}