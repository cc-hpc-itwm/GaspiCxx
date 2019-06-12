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
 * Group.hpp
 *
 */

#ifndef GROUP_HPP_
#define GROUP_HPP_

#include <GaspiCxx/group/Rank.hpp>
#include <memory>
#include <set>

extern "C" {
#include <GASPI.h>
}

namespace gaspi {
namespace group {

class Rank;

class Group
{
  private:

    //! The group of ranks that constitute the interface
    std::unique_ptr<gaspi_group_t> _pgroup;

  public:

    Group
      ();

    Group
      (Group &&);

    Group
      (std::set<gaspi_rank_t> const &);

    virtual
    ~Group();

    gaspi_group_t const &
    group
      () const;

    Rank
    size
      () const;

//    bool
//    contains
//      () const;

    Rank
    rank() const;
};

gaspi_rank_t
groupToGlobalRank
  ( Group const & group
  , Rank const & rank );

Rank
globalToGroupRank
  ( Group const & group
  , gaspi_rank_t const & rank );

} /* namespace group */
} /* namespace gaspi */

#endif /* GROUP_HPP_ */
