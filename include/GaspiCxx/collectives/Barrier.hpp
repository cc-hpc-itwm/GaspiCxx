#pragma once

#include <GaspiCxx/collectives/Collective.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/segment/Segment.hpp>

namespace gaspi {
namespace collectives {
namespace blocking {

class Barrier : public Collective
{
public:

  Barrier(gaspi::segment::Segment&,
          gaspi::group::Group const&);
  ~Barrier() = default;

  void execute() override;

private:

};

}
}
}
