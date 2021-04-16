#pragma once

#include <GaspiCxx/segment/SegmentPool.hpp>

namespace gaspi
{
  namespace segment
  {
    class SingleSegmentPool : public SegmentPool
    {
      public:
        SingleSegmentPool();
        SingleSegmentPool(std::size_t);
        SingleSegmentPool(SingleSegmentPool const&) = delete;
        SingleSegmentPool(SingleSegmentPool&&) = default;
        ~SingleSegmentPool() = default;

        Segment& getSegment(std::size_t) override;

      private:
        Segment segment;
    };
  }
}
