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
 * AllgathervCommon.hpp
 *
 */
#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/group/Group.hpp>

#include <array>
#include <unordered_map>

namespace gaspi
{
  namespace collectives
  {
    class AllgathervInfo
    {
      public:
        enum class Algorithm
        {
          RING,
        };
        static inline std::unordered_map<Algorithm, std::string> names
                      { {Algorithm::RING, "ring" } };
        static inline constexpr std::array<Algorithm, 1> implemented
                      { Algorithm::RING };
    };
    using AllgathervAlgorithm = AllgathervInfo::Algorithm;

    class AllgathervCommon : public CollectiveLowLevel
    {
      public:
        AllgathervCommon(gaspi::group::Group const& group,
                         std::vector<std::size_t> const& counts);
        virtual ~AllgathervCommon() = default;
        std::size_t getOutputCount() override;

      protected:
        gaspi::group::Group group;
        std::vector<std::size_t> counts;
        std::vector<std::size_t> offsets;
        std::size_t number_elements;
    };

    template<typename T, AllgathervAlgorithm Algorithm>
    class AllgathervLowLevel : public AllgathervCommon
    { };
  }
}