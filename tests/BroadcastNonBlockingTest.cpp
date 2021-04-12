#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Broadcast.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastSendToAll.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastBasicLinear.hpp>
#include <GaspiCxx/group/Group.hpp>

#include "parametrized_test_utilities.hpp"
#include "collectives_utilities.hpp"

#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

    std::mt19937 generator(42);
    std::vector<BroadcastAlgorithm> const broadcastAlgorithms{BroadcastAlgorithm::BASIC_LINEAR,
                                                              BroadcastAlgorithm::SEND_TO_ALL};

    template<typename T>
    class BroadcastFactory
    {
      public:
        static auto factory(BroadcastAlgorithm alg,
                            gaspi::group::Group const& group, std::size_t num_elements,
                            gaspi::group::Rank const& root)
        {
          std::unordered_map<BroadcastAlgorithm,
                            std::unique_ptr<RootedSendCollective>> mapping;
          mapping.insert(generate_map_element<BroadcastAlgorithm, Broadcast,
                                              T, BroadcastAlgorithm::SEND_TO_ALL>(
                                                        group, num_elements, root));
          mapping.insert(generate_map_element<BroadcastAlgorithm, Broadcast,
                                              T, BroadcastAlgorithm::BASIC_LINEAR>(
                                                        group, num_elements, root));
          return std::move(mapping[alg]);
        }
    };

    using TestCase = std::tuple<BroadcastAlgorithm, DataSize, ElementType>;
    class BroadcastTest : public CollectivesFixture,
                          public testing::WithParamInterface<TestCase>
    {
      protected:
        BroadcastTest()
        : algorithm(std::get<0>(GetParam())),
          num_elements(std::get<1>(GetParam())),
          elem_type_string(std::get<2>(GetParam())),
          root(get_root())
        {}

        auto make_bcast()
        {
          auto factory = select_factory_by_type<BroadcastFactory>(elem_type_string);          
          return factory(algorithm, group_all, num_elements, root);
        }

        auto make_data()
        {
          auto factory = select_factory_by_type<DataFactory>(elem_type_string);
          return factory(num_elements);
        }

        gaspi::group::Rank get_root()
        {
          std::uniform_int_distribution<> distribution(0, gaspi::getRuntime().size() - 1);

          return gaspi::group::Rank(distribution(generator));
        }

        BroadcastAlgorithm algorithm;
        DataSize num_elements;
        ElementType elem_type_string;
        gaspi::group::Rank root;
    };

    TEST_P(BroadcastTest, one_bcast)
    {
      auto broadcast = make_bcast();
      auto inputs = make_data();
      auto outputs = make_data();
      auto expected = make_data();

      inputs->fill(42.22);
      expected->fill(42.22);

      outputs->fill(0);
      if (group_all.rank() == root)
      {
        broadcast->start(inputs->get_data());
      }
      else
      {
        broadcast->start();
      }
      broadcast->waitForCompletion(outputs->get_data());

      ASSERT_EQ(*outputs, *expected);
    }

    std::vector<ElementType> const elementTypes{"int", "float", "double"};
    std::vector<DataSize> const dataSizes{0, 1, 5, 32, 1003};
    INSTANTIATE_TEST_SUITE_P(Coll, BroadcastTest,
                             testing::Combine(testing::ValuesIn(broadcastAlgorithms),
                                              testing::ValuesIn(dataSizes),
                                              testing::ValuesIn(elementTypes)));
  }
}
