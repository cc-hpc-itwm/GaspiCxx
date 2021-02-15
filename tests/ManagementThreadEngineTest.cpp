#include <gtest/gtest.h>

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GlobalTestConfiguration.hpp>
#include <progress_engine/ManagementThreadEngine.hpp>

#include <future>
#include <stdexcept>
#include <vector>

extern GlobalTestConfiguration *globalTestConf;
namespace gaspi
{
  namespace
  {
    class CollectiveMock : public collectives::CollectiveLowLevel
    {
      public:
        CollectiveMock()
        {}
        ~CollectiveMock() = default;

        void init()
        {
          waitForSetup();
          copyIn(nullptr);
          start();
        }

      private:
        std::size_t const nsteps = 100;
        std::size_t current_step = 0;

        void waitForSetupImpl() override
        {}

        void copyInImpl(void const*) override
        {}
        void copyOutImpl(void*) override
        {}

        void startImpl() override
        {}
        bool triggerProgressImpl() override
        {
          if (current_step < nsteps)
          {
            ++current_step;
            return false;
          }
          return true;
        }

    };
  }

  TEST(ManagementThreadEngineTest, init)
  {
    ASSERT_NO_THROW(std::make_unique<ManagementThreadEngine>());
  }

  TEST(ManagementThreadEngineTest, register_collective)
  {
    ManagementThreadEngine engine;
    auto col = std::make_shared<CollectiveMock>();

    ASSERT_NO_THROW(engine.register_collective(col));
  }

  TEST(ManagementThreadEngineTest, deregister_collective)
  {
    ManagementThreadEngine engine;
    auto col = std::make_shared<CollectiveMock>();

    auto handle = engine.register_collective(col);
    ASSERT_NO_THROW(engine.deregister_collective(handle));
  }

  TEST(ManagementThreadEngineTest, execute_collective)
  {
    ManagementThreadEngine engine;
    auto collective = std::make_shared<CollectiveMock>();

    auto handle = engine.register_collective(collective);
    collective->init();

    while(true)
    {
      if (collective->checkForCompletion()) break;
    }

    ASSERT_NO_THROW(engine.deregister_collective(handle));
  }

  TEST(ManagementThreadEngineTest, execute_multiple_collectives)
  {
    ManagementThreadEngine engine;
    auto collective1 = std::make_shared<CollectiveMock>();
    auto collective2 = std::make_shared<CollectiveMock>();
    auto collective3 = std::make_shared<CollectiveMock>();

    auto handle1 = engine.register_collective(collective1);
    collective1->init();

    auto handle2 = engine.register_collective(collective2);
    collective2->init();

    while(true)
    {
      if (collective1->checkForCompletion()) break;
    }

    auto handle3 = engine.register_collective(collective3);

    engine.deregister_collective(handle1);

    collective3->init();

    while(true)
    {
      if (collective3->checkForCompletion()) break;
    }

    while(true)
    {
      if (collective2->checkForCompletion()) break;
    }

    engine.deregister_collective(handle3);
    engine.deregister_collective(handle2);
  }
 
}
