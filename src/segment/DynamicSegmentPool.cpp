#include <GaspiCxx/segment/DynamicSegmentPool.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace gaspi
{
  namespace segment
  {
    DynamicSegmentPool::DynamicSegmentPool(std::size_t sizePerSegment)
    : _sizePerSegment(sizePerSegment)
	, _maxSegmentSize(4UL*1024UL*1024UL*1024UL)
    , _segments()
	, _mutex()
    {
    	if(_sizePerSegment > _maxSegmentSize)
    	{
    	  throw std::runtime_error(
			"DynamicSegmentPool: Requested size per segment larger than maximally allowed size per segment");
    	}
    }

    DynamicSegmentPool::DynamicSegmentPool()
    : DynamicSegmentPool(1024UL * 1024UL) // 1MiB
    { }

    Segment& DynamicSegmentPool::getSegment(std::size_t size)
    {
   	  if (size > _maxSegmentSize)
	  {
		throw std::runtime_error(
		  "DynamicSegmentPool::getSegment: Requested size larger than maximally allowed size per segment");
	  }

   	  if (size > _sizePerSegment)
	  {
		// adjust sizePerSegment to closest power of two
   		_sizePerSegment = std::pow(2, std::ceil(std::log(size)/std::log(2)));
   		_sizePerSegment = std::min( _sizePerSegment, _maxSegmentSize);
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
