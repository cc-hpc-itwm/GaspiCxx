#pragma once

#include <GaspiCxx/segment/SegmentPool.hpp>

#include <list>
#include <mutex>

namespace gaspi
{
  namespace segment
  {
    class DynamicSegmentPool : public SegmentPool
    {
      public:
        DynamicSegmentPool();
        DynamicSegmentPool(std::size_t);
        DynamicSegmentPool(DynamicSegmentPool const&) = delete;
        DynamicSegmentPool(DynamicSegmentPool&&) = default;
        ~DynamicSegmentPool() = default;

        Segment& getSegment(std::size_t) override;

      private:

        std::size_t	       _sizePerSegment;
        std::list<Segment> _segments;
        std::mutex         _mutex;
    };
  }
}
