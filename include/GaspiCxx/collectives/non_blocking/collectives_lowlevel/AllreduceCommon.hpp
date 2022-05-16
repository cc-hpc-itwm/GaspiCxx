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
 * AllreduceCommon.hpp
 *
 */

#pragma once

#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

#include <array>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace gaspi
{
  namespace collectives
  {
    enum class ReductionOp
    {
      PROD,
      SUM,
      MIN,
      MAX,
      AND,
      OR
    };

    class AllreduceInfo
    {
      public:
        enum class Algorithm
        {
          RING,
          RECURSIVE_DOUBLING,
        };
        static inline std::unordered_map<Algorithm, std::string> names
                      { {Algorithm::RING, "ring" },
                        {Algorithm::RECURSIVE_DOUBLING, "recursivedoubling" } };
        static inline constexpr std::array<Algorithm, 2> implemented
                      { Algorithm::RING, Algorithm::RECURSIVE_DOUBLING };
    };
    using AllreduceAlgorithm = AllreduceInfo::Algorithm;

    class AllreduceCommon : public CollectiveLowLevel
    {
      public:
        AllreduceCommon(gaspi::group::Group const& group,
                        std::size_t number_elements,
                        ReductionOp reduction_op);
        virtual ~AllreduceCommon() override = default;
        std::size_t getOutputCount() override;

      protected:
        gaspi::group::Group group;
        std::size_t number_elements;
        ReductionOp reduction_op;

        template<typename T>
        void apply_reduce_op(gaspi::singlesided::write::SourceBuffer& source_comm,
                             gaspi::singlesided::write::TargetBuffer& target_comm)
        {
          auto const source_begin = static_cast<T*>(source_comm.address());
          auto const source_end = source_begin +
                                  source_comm.description().size()/sizeof(T);
          auto const target_begin = static_cast<T*>(target_comm.address());

          std::function<T(T const&, T const&)> reduction_functor;
          switch (reduction_op)
          {
            case ReductionOp::PROD:
            {
              reduction_functor = std::multiplies<T>();
              break;
            }
            case ReductionOp::SUM:
            {
              reduction_functor = std::plus<T>();
              break;
            }
            case ReductionOp::MIN:
            {
              reduction_functor = [](T const& a, T const& b)
                                  {
                                    return (a<b)?a:b;
                                  };
              break;
            }
            case ReductionOp::MAX:
            {
              reduction_functor = [](T const& a, T const& b)
                                  {
                                    return (a>b)?a:b;
                                  };
              break;
            }
            case ReductionOp::AND:
            {
              reduction_functor = std::logical_and<T>();
              break;
            }
            case ReductionOp::OR:
            {
              reduction_functor = std::logical_or<T>();
              break;
            }
          }

          std::transform(source_begin, source_end, target_begin,
                         source_begin, reduction_functor);
        }
    };

    template<typename T, AllreduceAlgorithm Algorithm>
    class AllreduceLowLevel : public AllreduceCommon
    { };
  }
}
