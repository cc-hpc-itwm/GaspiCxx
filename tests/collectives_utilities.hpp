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
 * collectives_utilities.hpp
 *
 */

#include <gtest/gtest.h>

#include <GaspiCxx/group/Group.hpp>

namespace gaspi {
  namespace collectives {

    class CollectivesFixture : public ::testing::Test
    {
      protected:
        CollectivesFixture()
        : group_all()
        {
          getRuntime().barrier();
        }

        ~CollectivesFixture()
        {
          getRuntime().barrier();
        }
        gaspi::group::Group const group_all;
    };

    template<typename AlgType, template <typename, AlgType> typename CollectiveOp,
             typename T,
             AlgType Algorithm,
             typename... Args>
    auto generate_map_element(Args&&... args)
    {
      return std::make_pair(Algorithm,
                            std::make_unique<CollectiveOp<T, Algorithm>>(std::forward<Args>(args)...));
    }
  }
}
