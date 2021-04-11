#include <gtest/gtest.h>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/collectives/non_blocking/Broadcast.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastSendToAll.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastBasicLinear.hpp>
#include <GaspiCxx/group/Group.hpp>

#include "parametrized_test_utilities.hpp"
#include "collectives_utilities.hpp"

#include <numeric>
#include <stdexcept>
#include <vector>

namespace gaspi {
  namespace collectives {

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
          mapping.insert(generate_map_element<T, BroadcastAlgorithm, Broadcast, BroadcastAlgorithm::SEND_TO_ALL>(
                                                  group, num_elements, root));
          mapping.insert(generate_map_element<T, BroadcastAlgorithm, Broadcast, BroadcastAlgorithm::BASIC_LINEAR>(
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
          root(0)
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

     //auto const expected = inputs;

      if (group_all.rank() == root)
      {
        broadcast->start(inputs->get());
      }
      else
      {
        broadcast->start();
      }
      broadcast->waitForCompletion(outputs->get());

//      ASSERT_EQ(outputs, expected);
    }

    std::vector<ElementType> const elementTypes{"int", "float"};
    std::vector<DataSize> const dataSizes{0, 1, 5};
    INSTANTIATE_TEST_SUITE_P(Coll, BroadcastTest,
                             testing::Combine(testing::ValuesIn(broadcastAlgorithms),
                                              testing::ValuesIn(dataSizes),
                                              testing::ValuesIn(elementTypes)));
  }
}
