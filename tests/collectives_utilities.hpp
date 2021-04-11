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
  }
}
