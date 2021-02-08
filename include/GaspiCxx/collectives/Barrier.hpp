#pragma once

#include <GaspiCxx/collectives/Collective.hpp>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

#include <memory>

namespace gaspi {
namespace collectives {
namespace blocking {

class Barrier : public Collective
{
  using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
  using TargetBuffer = gaspi::singlesided::write::TargetBuffer;

  public:

    Barrier(gaspi::segment::Segment&,
            gaspi::group::Group const&);
    ~Barrier() = default;

    void execute();

  private:
    gaspi::group::Group group;
    gaspi::Context context;
    std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
    std::vector<std::unique_ptr<TargetBuffer>> target_buffers;

};

}
}
}
