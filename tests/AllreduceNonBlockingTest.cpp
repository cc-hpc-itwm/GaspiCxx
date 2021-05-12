/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019
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
 * AllreduceNonBlockingTest.cpp
 *
 */

#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Allreduce.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceRing.hpp>
#include <GaspiCxx/group/Group.hpp>

#include "parametrized_test_utilities.hpp"
#include "collectives_utilities.hpp"

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    std::vector<AllreduceAlgorithm> const allreduceAlgorithms{AllreduceAlgorithm::RING};

    template<typename T>
    class AllreduceFactory
    {
      public:
        static auto factory(AllreduceAlgorithm alg,
                            gaspi::group::Group const& group, std::size_t num_elements,
                            ReductionOp red_op)
        {
          std::unordered_map<AllreduceAlgorithm,
                            std::unique_ptr<Collective>> mapping;
          mapping.insert(generate_map_element<AllreduceAlgorithm, Allreduce,
                                              T, AllreduceAlgorithm::RING>(
                                              group, num_elements, red_op));

          return std::move(mapping[alg]);
        }
    };

    using TestCase = std::tuple<AllreduceAlgorithm, DataSize, ElementType>;
    class AllreduceTest : public CollectivesFixture,
                          public testing::WithParamInterface<TestCase>
    {
      protected:
        AllreduceTest()
        : algorithm(std::get<0>(GetParam())),
          num_elements(std::get<1>(GetParam())),
          elem_type_string(std::get<2>(GetParam())),
          reduction_op(ReductionOp::SUM)
        {}

        auto make_allreduce()
        {
          auto factory = select_factory_by_type<AllreduceFactory>(elem_type_string);
          return factory(algorithm, group_all, num_elements, reduction_op);
        }

        auto make_data()
        {
          auto factory = select_factory_by_type<DataFactory>(elem_type_string);
          return factory(num_elements);
        }

        AllreduceAlgorithm algorithm;
        DataSize num_elements;
        ElementType elem_type_string;
        ReductionOp const reduction_op;
    };


    TEST_P(AllreduceTest, one_allreduce)
    {
      auto allreduce = make_allreduce();
      auto inputs = make_data();
      auto outputs = make_data();
      auto expected = make_data();

      std::vector<double> input_values(inputs->get_num_elements());
      std::iota(input_values.begin(), input_values.end(), 42.22);
      inputs->fill_from_list(input_values);

      expected->fill_from_list_and_scale(input_values, group_all.size());
      outputs->fill(0);

      allreduce->start(inputs->get_data());
      allreduce->waitForCompletion(outputs->get_data());

      ASSERT_EQ(*outputs, *expected);
    }

    std::vector<ElementType> const elementTypes{"int", "float", "double"};
    std::vector<DataSize> const dataSizes{0, 1, 5, 32, 1003};
    INSTANTIATE_TEST_SUITE_P(Coll, AllreduceTest,
                             testing::Combine(testing::ValuesIn(allreduceAlgorithms),
                                              testing::ValuesIn(dataSizes),
                                              testing::ValuesIn(elementTypes)));
  }
}
