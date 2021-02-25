#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

namespace gaspi
{
  namespace collectives
  {
    AllreduceCommon::AllreduceCommon(gaspi::group::Group const& group,
                                     std::size_t number_elements,
                                     ReductionOp reduction_op,
                                     gaspi::CommunicationContext& comm_context)
    : group(group),
      number_elements(number_elements),
      reduction_op(reduction_op),
      comm_context(comm_context)
    { }

  }
}
