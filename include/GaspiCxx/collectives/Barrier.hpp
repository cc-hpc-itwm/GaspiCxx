/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
 *
 * This file is part of GaspiCxx.
 *
 * GaspiCxx is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * GaspiCxx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Barrier.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/Collective.hpp>
#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/group/Group.hpp>
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

    Barrier(gaspi::group::Group const&);
    ~Barrier() override = default;

    void execute();

  private:
    gaspi::CommunicationContext& comm_context;
    std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
    std::vector<std::unique_ptr<TargetBuffer>> target_buffers;
    std::size_t number_steps;
};

}
}
}
