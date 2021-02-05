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
 * Rank.hpp
 *
 */

#ifndef GROUP_RANK_HPP_
#define GROUP_RANK_HPP_

#include <memory>
#include <set>

extern "C" {
#include <GASPI.h>
}

namespace gaspi {
namespace group {

class Rank {

public:

  using Type = unsigned short;

  explicit
  Rank
    ( Type rank );

  Rank
    ( Rank const&) = default;
  ~Rank() = default;

  Type
  get
    () const;

  Rank &
  operator++();

  Rank
  operator++(int);

  Rank &
  operator--();

  Rank
  operator--(int);

  Rank
  operator+(Rank const & other) const;

  Rank
  operator+(int const & other) const;

  Rank
  operator-(Rank const & other) const;

  Rank
  operator-(int const & other) const;

  Rank
  operator%(Rank const & other) const;

  Rank
  operator%(int const & other) const;

  bool
  operator==( Rank const & other ) const;

  bool
  operator!=( Rank const & other ) const;

  bool
  operator<( Rank const & other ) const;

  bool
  operator<=( Rank const & other ) const;

  bool
  operator>( Rank const & other ) const;

  bool
  operator>=( Rank const & other ) const;

private:

  Type _rank;

};

} /* namespace group */
} /* namespace gaspi */

#endif /* GROUP_RANK_HPP_ */
