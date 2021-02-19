#include <gtest/gtest.h>

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/progress_engine/RoundRobinDedicatedThread.hpp>

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

  TEST(RoundRobinDedicatedThreadTest, init)
  {
    ASSERT_NO_THROW(std::make_unique<progress_engine::RoundRobinDedicatedThread>());
  }

  TEST(RoundRobinDedicatedThreadTest, register_collective)
  {
    progress_engine::RoundRobinDedicatedThread engine;
    auto col = std::make_shared<CollectiveMock>();

    ASSERT_NO_THROW(engine.register_collective(col));
  }

  TEST(RoundRobinDedicatedThreadTest, deregister_collective)
  {
    progress_engine::RoundRobinDedicatedThread engine;
    auto col = std::make_shared<CollectiveMock>();

    auto handle = engine.register_collective(col);
    ASSERT_NO_THROW(engine.deregister_collective(handle));
  }

  TEST(RoundRobinDedicatedThreadTest, execute_collective)
  {
    progress_engine::RoundRobinDedicatedThread engine;
    auto collective = std::make_shared<CollectiveMock>();

    auto handle = engine.register_collective(collective);
    collective->init();

    while(true)
    {
      if (collective->checkForCompletion()) break;
    }

    ASSERT_NO_THROW(engine.deregister_collective(handle));
  }

  TEST(RoundRobinDedicatedThreadTest, execute_multiple_collectives)
  {
    progress_engine::RoundRobinDedicatedThread engine;
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
