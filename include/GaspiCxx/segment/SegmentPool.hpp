#pragma once

#include <GaspiCxx/segment/Segment.hpp>

namespace gaspi
{
  namespace segment
  {
    // A `SegmentPool` provides an abstraction
    // of the GPI segments, in order to
    // simplify the creation of `Buffer`s
    // used for communication.
    // 
    // It manages one or several segments,
    // and returns references to one of them
    // that currently has enough free memory.
    class SegmentPool
    {
      public:
        // Return a reference to a `Segment` that
        // has at least `size` bytes of memory available
        virtual Segment& getSegment(std::size_t size) = 0;

        virtual ~SegmentPool() = default;
    };
  }
}