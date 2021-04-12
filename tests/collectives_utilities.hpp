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
