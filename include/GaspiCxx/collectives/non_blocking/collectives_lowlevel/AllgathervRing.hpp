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
 * AllreduceRing.hpp
 *
 */

#pragma once

#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervCommon.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    template<typename T>
    class AllgathervLowLevel<T, AllgathervAlgorithm::RING> : public AllgathervCommon
    {
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;
      using ConnectHandle = gaspi::singlesided::Endpoint::ConnectHandle;

      public:
        using AllgathervCommon::AllgathervCommon;

        AllgathervLowLevel(gaspi::group::Group const& group,
                          std::size_t const* counts);
        
      private:
        gaspi::group::Rank rank;
        std::size_t number_ranks;
        std::unique_ptr<SourceBuffer> source_buffer;
    }
  }
}