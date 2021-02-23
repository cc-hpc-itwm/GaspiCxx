#pragma once

#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/segment/SegmentPool.hpp>

namespace gaspi {

enum class SegmentPoolType
{
  None,
  SingleSegment // FIXME: Add a dynamic SegmentPool implementation
};

enum class ProgressEngineType
{
  None,
  RoundRobinDedicatedThread
};

class RuntimeConfiguration
{
  public:
    explicit RuntimeConfiguration(SegmentPoolType, ProgressEngineType);

    RuntimeConfiguration(RuntimeConfiguration const&) = default;
    ~RuntimeConfiguration() = default;

    std::unique_ptr<segment::SegmentPool> get_segment_pool() const;
    std::unique_ptr<progress_engine::ProgressEngine> get_progress_engine() const;

  private:
    SegmentPoolType segment_pool_type;
    ProgressEngineType progress_engine_type;
};

} // namespace gaspi
