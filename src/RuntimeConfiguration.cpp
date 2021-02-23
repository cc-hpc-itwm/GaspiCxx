#include <GaspiCxx/RuntimeConfiguration.hpp>

#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>

#include <GaspiCxx/segment/SegmentPool.hpp>
#include <GaspiCxx/segment/SingleSegmentPool.hpp>

namespace gaspi
{
  namespace
  {
    class SegmentPoolFactory
    {
      public:
        static std::unique_ptr<segment::SegmentPool>
              createSegmentPool(SegmentPoolType segment_pool_type)
        {
          switch (segment_pool_type)
          {
            case SegmentPoolType::SingleSegment:
            {
              return std::make_unique<segment::SingleSegmentPool>(100*1024*1024);
            }
            default:
            { return nullptr; }
          }
        }
    };

    class ProgressEngineFactory
    {
      public:
        static std::unique_ptr<progress_engine::ProgressEngine>
              createProgressEngine(ProgressEngineType progress_engine_type)
        {
          switch (progress_engine_type)
          {
            case ProgressEngineType::RoundRobinDedicatedThread:
            {
              return std::make_unique<progress_engine::RoundRobinDedicatedThread>();
            }
            default:
            { return nullptr; }
          }
        }
    };
  }
  
  RuntimeConfiguration::RuntimeConfiguration(
        SegmentPoolType segment_pool_type,
        ProgressEngineType progress_engine_type)
  : segment_pool_type(segment_pool_type),
    progress_engine_type(progress_engine_type)
  { }

  std::unique_ptr<segment::SegmentPool>
  RuntimeConfiguration::get_segment_pool() const
  {
    return SegmentPoolFactory::createSegmentPool(segment_pool_type);
  }
  
  std::unique_ptr<progress_engine::ProgressEngine>
  RuntimeConfiguration::get_progress_engine() const
  {
    return ProgressEngineFactory::createProgressEngine(progress_engine_type);
  }
}
