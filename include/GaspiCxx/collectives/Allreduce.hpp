/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2017
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
 * Allreduce.hpp
 *
 */

#ifndef COLLECTIVES_ALLREDUCE_HPP_
#define COLLECTIVES_ALLREDUCE_HPP_

#include <GaspiCxx/Context.hpp>

namespace gaspi {
namespace collectives {

struct Allreduce {

  enum Type { SUM
            , MAX
            , MIN };

};

template <typename T>
T
allreduce
  ( T const &
  , Allreduce::Type const &
  , Context & context );

}
}

#endif
