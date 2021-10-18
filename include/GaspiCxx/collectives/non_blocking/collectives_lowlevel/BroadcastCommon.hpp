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
 * BroadcastCommon.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/group/Group.hpp>

namespace gaspi
{
  namespace collectives
  {
    enum class BroadcastAlgorithm
    {
      BASIC_LINEAR,
      SEND_TO_ALL,
    };

    class BroadcastCommon : public CollectiveLowLevel
    {
      public:
        BroadcastCommon(gaspi::group::Group const& group,
                        std::size_t number_elements,
                        gaspi::group::Rank const& root);
        std::size_t get_output_count();

      protected:
        gaspi::group::Group group;
        std::size_t number_elements;
        gaspi::group::Rank root;
    };

    template<typename T, BroadcastAlgorithm Algorithm>
    class BroadcastLowLevel : public BroadcastCommon
    { };
  }
}
