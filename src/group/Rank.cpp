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
 * Rank.cpp
 *
 */

#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {
namespace group {

Rank
  ::Rank
      ( Type rank )
: _rank(rank)
{}

Rank::Type
Rank
  ::get
    () const {
  return _rank;
}

Rank &
Rank
  ::operator++() {
  ++_rank;
  return *this;
}

Rank
Rank
  ::operator++(int) {
  Rank tmp(*this);
  ++(*this);
  return tmp;
}

Rank &
Rank
  ::operator--() {
  --_rank;
  return *this;
}

Rank
Rank
  ::operator--(int) {
  Rank tmp(*this);
  --(*this);
  return tmp;
}

Rank
Rank
  ::operator+(Rank const & other) const {
  return Rank(this->get()+other.get());
}

Rank
Rank
  ::operator+(int const & other) const {
  return *this + Rank(other);
}

Rank
Rank
  ::operator-(Rank const & other) const {
  return Rank(this->get()-other.get());
}

Rank
Rank
  ::operator-(int const & other) const {
  return *this - Rank(other);
}

Rank
Rank
  ::operator%(Rank const & other) const {
  return Rank(this->get()%other.get());
}


bool
Rank
  ::operator==
    ( Rank const & other ) const{
  return ( _rank == other._rank );
}

bool
Rank
  ::operator!=
    ( Rank const & other ) const {
  return !(this->operator==(other) );
}

bool
Rank
  ::operator<( Rank const & other ) const {
  return _rank < other._rank;
}


bool
Rank
  ::operator<=( Rank const & other ) const {
  return !(this->operator>(other) );
}

bool
Rank
  ::operator>( Rank const & other ) const {
  return _rank > other._rank;
}

bool
Rank
  ::operator>=( Rank const & other ) const {
  return !(this->operator<(other) );
}

} /* namespace group */
} /* namespace gaspi */
