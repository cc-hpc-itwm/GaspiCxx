/*
 * run_tests.cpp
 */


#include <gtest/gtest.h>
#include <GlobalTestConfiguration.hpp>

GlobalTestConfiguration *globalTestConf;

int main(int argc, char *argv[])
{
  // Google Test will remove its own parameters from argv, so run it first!
  ::testing::InitGoogleTest(&argc, argv);

//  // Initialize GASPI and its specific test configuration.
  globalTestConf = new GlobalTestConfiguration(argc, argv);

  int result = RUN_ALL_TESTS();

//  // Delete test configuration and finalize GASPI.
//  delete globalTestConf;

  return result;
}

