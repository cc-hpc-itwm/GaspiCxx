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
 * run_tests.cpp
 *
 */

#include <GaspiCxx/Runtime.hpp>

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
  // Google Test will remove its own parameters from argv, so run it first!
  ::testing::InitGoogleTest(&argc, argv);

  gaspi::initGaspiCxx();
  int result = RUN_ALL_TESTS();
  return result;
}
