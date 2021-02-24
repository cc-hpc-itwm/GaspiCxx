#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/group/Group.hpp>

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
        BroadcastCommon(gaspi::group::Group const& group,
                        std::size_t number_elements,
                        gaspi::group::Rank const& root);

      protected:
        gaspi::group::Group group;
        std::size_t number_elements;
        gaspi::group::Rank root;
    };

    template<typename T, BroadcastAlgorithm Algorithm>
    class BroadcastLowLevel : public BroadcastCommon
    { };
  }
}
