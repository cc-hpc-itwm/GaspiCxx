#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/group/Group.hpp>

namespace gaspi
{
  namespace collectives
  {
    enum class ReductionOp
    {
      SUM,
      AVERAGE,
    };

    enum class AllreduceAlgorithm
    {
      RING,
    };

    class AllreduceCommon : public CollectiveLowLevel
    {
      public:
        AllreduceCommon(gaspi::group::Group const& group,
                        std::size_t number_elements,
                        ReductionOp reduction_op);

      protected:
        gaspi::group::Group group;
        std::size_t number_elements;
        ReductionOp reduction_op;
    };

    template<typename T, AllreduceAlgorithm Algorithm>
    class AllreduceLowLevel : public AllreduceCommon
    { };
  }
}
