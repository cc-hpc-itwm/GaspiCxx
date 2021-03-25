#include <GaspiCxx/segment/DynamicSegmentPool.hpp>

#include <algorithm>
#include <stdexcept>

namespace gaspi
{
  namespace segment
  {
    DynamicSegmentPool::DynamicSegmentPool(std::size_t sizePerSegment)
    : _sizePerSegment(sizePerSegment)
    , _segments()
	, _mutex()
    { }

    DynamicSegmentPool::DynamicSegmentPool()
    : DynamicSegmentPool(1024UL * 1024UL) // 1MiB
    { }

    Segment& DynamicSegmentPool::getSegment(std::size_t size)
    {
   	  if (size > _sizePerSegment)
	  {
		throw std::runtime_error(
		  "DynamicSegmentPool::getSegment: Requested size larger than size per segment");
	  }

   	  std::lock_guard<std::mutex> lock(_mutex);

      auto iter = std::find_if
		  ( _segments.begin()
		  , _segments.end()
		  , [size](Segment & segment)
			  {return segment.hasFreeMemory(size);}
		  );

      if(iter == _segments.end()) {
    	iter = _segments.emplace(_segments.begin(), _sizePerSegment);
      }

      return *iter;
    }
  }
}
