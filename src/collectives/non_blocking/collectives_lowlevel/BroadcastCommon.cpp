#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>
namespace gaspi
{
  namespace collectives
  {
    BroadcastCommon::BroadcastCommon(gaspi::group::Group const& group,
                                     std::size_t number_elements,
                                     gaspi::group::Rank const& root_rank)
    : context(group),
      number_elements(number_elements),
      root_rank(root_rank)
    { }

  }
}
