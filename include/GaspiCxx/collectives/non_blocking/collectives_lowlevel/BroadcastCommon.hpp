#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/segment/Segment.hpp>

namespace gaspi
{
  namespace collectives
  {
    enum class BroadcastAlgorithm
    {
      SEND_TO_ALL,
    };

    class BroadcastCommon : public CollectiveLowLevel
    {
      public:
        BroadcastCommon(gaspi::segment::Segment& segment,
                        gaspi::group::Group const& group,
                        std::size_t number_elements,
                        gaspi::group::Rank const& root_rank);

      protected:
        gaspi::segment::Segment& segment;
        gaspi::Context context;
        std::size_t number_elements;
        gaspi::group::Rank root_rank;
    };

    template<typename T, BroadcastAlgorithm Algorithm>
    class BroadcastLowLevel : public BroadcastCommon
    { };
  }
}
