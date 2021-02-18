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
 * Group.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/utility/Macros.hpp>

#include <algorithm>
#include <numeric>
#include <unordered_set>

namespace gaspi {
namespace group {

namespace detail {

std::vector<GlobalRank>
generate_group_all()
{
  gaspi_rank_t nProc;
  GASPI_CHECK( gaspi_proc_num(&nProc) );
  std::vector<GlobalRank> group_all(nProc);

  std::iota(group_all.begin(), group_all.end(), 0);
  return group_all;
}

GlobalRank
generate_current_rank()
{
  gaspi_rank_t global_rank;
  GASPI_CHECK
    (gaspi_proc_rank(&global_rank) );
  return GlobalRank(global_rank);
}

} /* namespace detail */

Group
  ::Group()
: Group( detail::generate_group_all() )
{ }

Group
  ::Group( std::vector<GlobalRank> const & group_ranks )
: _group(group_ranks.begin(), group_ranks.end()),
  _group_rank(0)
{
  if (_group.size() == 0)
  {
    throw std::runtime_error(CODE_ORIGIN + "Cannot create empty group");
  }

  std::unordered_set<GlobalRank> unique_ranks(group_ranks.begin(), group_ranks.end());
  if (unique_ranks.size() != group_ranks.size())
  {
    throw std::runtime_error(CODE_ORIGIN + "Cannot create group containing duplicated ranks");
  }

  auto const global_rank = detail::generate_current_rank();
  auto const iter = std::find(_group.begin(), _group.end(), global_rank);
  if (iter == _group.end())
  {
    throw std::runtime_error
      (CODE_ORIGIN + "Current rank does not belong to group");
  }
  _group_rank = Rank(std::distance(_group.begin(), iter));
}

std::vector<GlobalRank> const &
Group
  ::group() const
{
  return _group;
}

std::size_t
Group
  ::size() const
{
  return _group.size();
}

Rank
Group
  ::rank() const
{
  return _group_rank;
}

bool
Group
  ::contains_rank(GlobalRank const& global_rank) const
{
  auto const iter = std::find(_group.begin(), _group.end(), global_rank);
  return iter != _group.end();
}

GlobalRank
Group
  ::toGlobalRank
 ( Rank  const & group_rank ) const
{
  if(group_rank.get() >= size())
  {
    std::stringstream ss;

    ss << "Invalid group GlobalRank "
       << group_rank.get()
       << ". Allowed range [0,"
       << size()
       <<")!";

    throw std::runtime_error
    (CODE_ORIGIN+ss.str());
  }

  return group()[group_rank.get()];
}

Rank
Group
  ::toGroupRank
 ( GlobalRank const & global_rank ) const
{
  auto const iter = std::find(group().begin(), group().end(),
                              global_rank);
  if (iter != group().end())
  {
    return Rank(std::distance(group().begin(), iter));
  }

  std::stringstream ss;
  ss << "Global GlobalRank "
     << global_rank
     << " not part of group!";

  throw std::runtime_error
    (CODE_ORIGIN+ss.str());
}


} /* namespace group */
} /* namespace gaspi */
