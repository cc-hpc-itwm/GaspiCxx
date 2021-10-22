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
 * AllgatherCommon.cpp
 *
 */

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervCommon.hpp>
#include <numeric>

namespace gaspi
{
  namespace collectives
  {
    AllgathervCommon::AllgathervCommon(gaspi::group::Group const& group,
                                       std::vector<std::size_t> const& counts)
    : group(group),
      counts(counts),
      offsets(counts.size(),0),
      number_elements(std::accumulate(counts.begin(), counts.end(), 0))
    {
      if(counts.size() > 1) 
        std::partial_sum(counts.begin(), counts.end() - 1, 
                         offsets.begin() + 1);
    }

    std::size_t AllgathervCommon::getOutputCount()
    {
      return number_elements;
    }
  }
}
