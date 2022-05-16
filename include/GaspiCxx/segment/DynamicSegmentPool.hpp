/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2019 - 2021
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
 * DynamicSegmentPool.hpp
 *
 */

#pragma once

#include <GaspiCxx/segment/SegmentPool.hpp>

#include <list>
#include <mutex>

namespace gaspi
{
  namespace segment
  {
    class DynamicSegmentPool : public SegmentPool
    {
      public:
        DynamicSegmentPool();
        DynamicSegmentPool(std::size_t);
        DynamicSegmentPool(DynamicSegmentPool const&) = delete;
        DynamicSegmentPool& operator=(const DynamicSegmentPool&) = delete;
        ~DynamicSegmentPool() override = default;

        Segment& getSegment(std::size_t) override;

      private:

        std::size_t	       _sizePerSegment;
        std::size_t	       _maxSegmentSize;
        std::list<Segment> _segments;
        std::mutex         _mutex;
    };
  }
}
