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
 * ProgressEngine.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace gaspi
{
  namespace progress_engine
  {
    class ProgressEngine
    {
      public:
        using CollectiveHandle = std::size_t;

        virtual ~ProgressEngine() = default;

        virtual CollectiveHandle register_collective(
                std::shared_ptr<collectives::CollectiveLowLevel>) = 0;
        virtual void deregister_collective(CollectiveHandle const&) = 0;

      protected:
        virtual void generate_progress() = 0;
    };
  }
}
