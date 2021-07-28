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
 * AllgatherNonBlockingTest.cpp
 *
 */

#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Allgatherv.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervRing.hpp>
#include <GaspiCxx/group/Group.hpp>

#include "parametrized_test_utilities.hpp"
#include "collectives_utilities.hpp"

#include <numeric>
#include <algorithm> 
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    std::vector<AllgathervAlgorithm> const allgathervAlgorithms{AllgathervAlgorithm::RING};

    template<typename T>
    class AllgathervFactory
    {
      public:
        static auto factory(AllgathervAlgorithm alg,
                            gaspi::group::Group const& group, 
                            std::vector<std::size_t> const& counts)
        {
          std::unordered_map<AllgathervAlgorithm,
                            std::unique_ptr<Collective>> mapping;
          mapping.insert(generate_map_element<AllgathervAlgorithm, Allgatherv,
                                              T, AllgathervAlgorithm::RING>(
                                              group, counts));

          return std::move(mapping[alg]);
        }
    };

    using TestCase = std::tuple<AllgathervAlgorithm, Counts, ElementType>;
    class AllgathervTest : public CollectivesFixture,
                           public testing::WithParamInterface<TestCase>
    {
      protected:
        AllgathervTest()
        : start(42.22),
          algorithm(std::get<0>(GetParam())),
          count(group_all.size()),
          offset(group_all.size(), 0),
          elem_type_string(std::get<2>(GetParam()))
        {
          Counts odd_even_counts(std::get<1>(GetParam()));
          for(auto i = 0UL; i < count.size(); ++i)
          {
            if(i % 2)
            {
              count[i] = odd_even_counts[0];
            }
            else
            {
              count[i] = odd_even_counts[1];
            }
          }

          if(count.size() > 1) 
            std::partial_sum(count.begin(), count.end() - 1, 
                             offset.begin() + 1);
        }

        auto make_allgatherv()
        {
          auto factory = select_factory_by_type<AllgathervFactory>(elem_type_string);
          return factory(algorithm, group_all, count);
        }

        auto make_input_data()
        {
          auto factory = select_factory_by_type<DataFactory>(elem_type_string);
          return factory(count[group_all.rank().get()]);
        }

        auto make_output_data()
        {
          auto factory = select_factory_by_type<DataFactory>(elem_type_string);
          return factory(std::accumulate(count.begin(), count.end(), 0));
        }

        auto fill_expected_data(std::vector<double> &expected_values)
        {
          for(auto i = 0UL; i < count.size(); ++i)
          {
            auto head = expected_values.begin() + offset[i];
            std::iota(head, head + count[i], start);
          }
        }

        double start;
        AllgathervAlgorithm algorithm;
        Counts count;
        Offsets offset;
        ElementType elem_type_string;
    };


    TEST_P(AllgathervTest, one_allgatherv)
    {
      auto allgatherv = make_allgatherv();
      auto inputs = make_input_data();
      auto outputs = make_output_data();
      auto expected = make_output_data();

      std::vector<double> input_values(inputs->get_num_elements());
      std::iota(input_values.begin(), input_values.end(), start);
      inputs->fill_from_list(input_values);
      
      std::vector<double> expected_values(expected->get_num_elements());
      fill_expected_data(expected_values);
      expected->fill_from_list(expected_values);

      outputs->fill(0);

      allgatherv->start(inputs->get_data());
      allgatherv->waitForCompletion(outputs->get_data());

      ASSERT_EQ(*outputs, *expected);
    }

    std::vector<ElementType> const elementTypes{"int", "float", "double"};
    std::vector<Counts> const oddevenCounts{
                                             {0, 0},
                                             {0, 1},
                                             {1, 0},
                                             {2, 4},
                                             {101, 203},
                                             {2313,2459}
                                           };
    INSTANTIATE_TEST_SUITE_P(AllgathervColl, AllgathervTest,
                             testing::Combine(testing::ValuesIn(allgathervAlgorithms),
                                              testing::ValuesIn(oddevenCounts),
                                              testing::ValuesIn(elementTypes)));
  }
}
