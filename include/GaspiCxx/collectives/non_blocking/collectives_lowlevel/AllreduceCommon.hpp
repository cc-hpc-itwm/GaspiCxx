#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/segment/Segment.hpp>

namespace gaspi
{
  namespace collectives
  {
    enum class ReductionOp
    {
      SUM,
      AVERAGE,
    };

    class AllreduceCommon : public CollectiveLowLevel
    {
      public:
        AllreduceCommon(gaspi::segment::Segment& segment,
                        gaspi::group::Group const& group,
                        std::size_t number_elements,
                        ReductionOp reduction_op);

      protected:
        gaspi::segment::Segment& segment;
        gaspi::group::Group const group;
        gaspi::Context context;
        std::size_t number_elements;
        ReductionOp reduction_op;
    };
  }
}
