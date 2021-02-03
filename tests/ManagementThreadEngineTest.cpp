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

      private:
        std::size_t const nsteps = 100;
        std::size_t current_step = 0;

        void waitForSetupImpl() override
        {}

        void copyInImpl(void*) override
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
    auto engine = std::make_unique<ManagementThreadEngine>();
    auto col = std::make_shared<CollectiveMock>();

    ASSERT_NO_THROW(engine->register_collective(col));
  }

  TEST(ManagementThreadEngineTest, deregister_collective)
  {
    auto engine = std::make_unique<ManagementThreadEngine>();
    auto col = std::make_shared<CollectiveMock>();

    auto handle = engine->register_collective(col);
    ASSERT_NO_THROW(engine->deregister_collective(handle));
  }

  TEST(ManagementThreadEngineTest, execute_collectives)
  {
    auto engine = std::make_unique<ManagementThreadEngine>();
    auto collective = std::make_shared<CollectiveMock>();

    auto handle = engine->register_collective(collective);
    std::vector<int> in_buffer;
    collective->waitForSetup();
    collective->copyIn(in_buffer.data());
    collective->start();

    collective->waitForCompletion();
    ASSERT_TRUE(collective->checkForCompletion());
    ASSERT_NO_THROW(engine->deregister_collective(handle));
  }

  namespace
  {
    auto register_collectives(ManagementThreadEngine& engine,
                      std::vector<std::shared_ptr<collectives::CollectiveLowLevel>>& data)
    {
      std::vector<std::future<ProgressEngine::CollectiveHandle>> futures;
      std::vector<ProgressEngine::CollectiveHandle> handles;

      // create multiple allreduce calls in parallel
      for (auto &col : data)
      {
        futures.emplace_back(std::async(
            std::launch::async,
            [&engine](std::shared_ptr<collectives::CollectiveLowLevel> col) 
                              -> ProgressEngine::CollectiveHandle
            {
              auto handle = engine.register_collective(col);
              return handle;
            },
            col));
      }
      // wait for all allreduce operations to be submitted
      for (auto &f : futures)
      {
        handles.push_back(f.get());
      }
      return handles;
    }
  }

  TEST(ManagementThreadEngineTest, multiple_async_collectives_start_stop)
  {
    auto n_collectives = 10UL;
    ManagementThreadEngine engine;
    std::vector<std::shared_ptr<collectives::CollectiveLowLevel>> collectives;

    // create collective ops
    for (auto i=0UL; i<n_collectives; i++)
    {
      collectives.push_back(std::make_shared<CollectiveMock>());
    }

    // add them asynchronously to the management engine
    auto handles = register_collectives(engine, collectives);

    // remove collectives from the management thread
    for (auto const& handle : handles)
    {
      ASSERT_NO_THROW(engine.deregister_collective(handle));

      // deregister again should fail
      ASSERT_THROW(engine.deregister_collective(handle), std::logic_error);
    }
  }

  TEST(ManagementThreadEngineTest, multiple_async_collectives)
  {
    auto n_collectives = 10UL;
    ManagementThreadEngine engine;
    std::vector<std::shared_ptr<collectives::CollectiveLowLevel>> collectives;

    // create collective ops
    for (auto i=0UL; i<n_collectives; i++)
    {
      collectives.push_back(std::make_shared<CollectiveMock>());
    }

    // add them asynchronously to the management engine
    auto handles = register_collectives(engine, collectives);
    
    // start collectives so that they can make progress
    for (auto const& collective : collectives)
    {
      std::vector<int> in_buffer;
      collective->waitForSetup();
      collective->copyIn(in_buffer.data());
      collective->start();
    }

    for (auto const& collective : collectives)
    {
      collective->waitForCompletion();
      ASSERT_TRUE(collective->checkForCompletion());
    }

    // remove collectives from the management thread
    for (auto const& handle : handles)
    {
      ASSERT_NO_THROW(engine.deregister_collective(handle));
    }
  }
 
}