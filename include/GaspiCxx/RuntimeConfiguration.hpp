#pragma once

#include <GaspiCxx/CommunicationContext.hpp>
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

enum class CommunicationContextType
{
  None,
  SingleQueue,
  RoundRobinQueues
};

class RuntimeConfiguration
{
  public:
    explicit RuntimeConfiguration(
      SegmentPoolType, ProgressEngineType, CommunicationContextType);

    RuntimeConfiguration(RuntimeConfiguration const&) = default;
    ~RuntimeConfiguration() = default;

    std::unique_ptr<segment::SegmentPool> get_segment_pool() const;
    std::unique_ptr<progress_engine::ProgressEngine> get_progress_engine() const;
    std::unique_ptr<CommunicationContext> get_communication_context() const;

  private:
    SegmentPoolType segment_pool_type;
    ProgressEngineType progress_engine_type;
    CommunicationContextType communication_context_type;

};

} // namespace gaspi
