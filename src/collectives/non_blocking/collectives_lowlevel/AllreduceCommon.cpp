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
 * AllreduceCommon.cpp
 *
 */

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

namespace gaspi
{
  namespace collectives
  {
    AllreduceCommon::AllreduceCommon(gaspi::group::Group const& group,
                                     std::size_t number_elements,
                                     ReductionOp reduction_op)
    : group(group),
      number_elements(number_elements),
      reduction_op(reduction_op)
    { }

    std::size_t AllreduceCommon::getOutputCount()
    {
      return number_elements;
    }
  }
}
