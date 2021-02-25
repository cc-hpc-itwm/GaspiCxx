#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>

namespace gaspi
{
  namespace collectives
  {
    BroadcastCommon::BroadcastCommon(gaspi::group::Group const& group,
                                     std::size_t number_elements,
                                     gaspi::group::Rank const& root,
                                     gaspi::CommunicationContext& comm_context)
    : group(group),
      number_elements(number_elements),
      root(root),
      comm_context(comm_context)
    { }

  }
}
