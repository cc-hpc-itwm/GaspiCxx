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
 * Group.hpp
 *
 */

#ifndef GROUP_HPP_
#define GROUP_HPP_

#include <GaspiCxx/group/Rank.hpp>

#include <memory>
#include <vector>

namespace gaspi {
namespace group {

using GlobalRank = unsigned short;

class Group
{
  private:

    //! The group of global ranks
    std::vector<GlobalRank> const _group;
    Rank _group_rank;

  public:

    Group
      ();

    Group
      (std::vector<GlobalRank> const &);

    std::vector<GlobalRank> const &
    group
      () const;

    std::size_t
    size
      () const;

    bool
    contains_rank
      (Rank const &) const;

    bool
    contains_global_rank
      (GlobalRank const &) const;

    // returns a `Rank` between [0, size-1] that corresponds to
    // the `GlobalRank` of the current process
    Rank
    rank() const;

    GlobalRank
    global_rank() const;

    GlobalRank
    toGlobalRank
      ( Rank const & ) const;

    Rank
    toGroupRank
      ( GlobalRank const & ) const;
};

} /* namespace group */
} /* namespace gaspi */

#endif /* GROUP_HPP_ */
