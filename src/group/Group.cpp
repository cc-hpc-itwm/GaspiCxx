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
 * Group.cpp
 *
 */

#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/utility/Macros.hpp>

namespace gaspi {
namespace group {

namespace detail {

std::set<gaspi_rank_t>
generate_group_all_set()
{
  std::set<gaspi_rank_t> group_all_set;

  gaspi_rank_t nProc;
  GASPI_CHECK( gaspi_proc_num(&nProc) );

  for( gaspi_rank_t iProc(0)
     ;              iProc < nProc
     ;            ++iProc ) {
    group_all_set.insert(iProc);
  }

  return group_all_set;
}

} /* namespace detail */

Group
  ::Group()
: Group( detail::generate_group_all_set() )
{ }

Group
  ::Group( std::set<gaspi_rank_t> const & group_ranks )
: _pgroup(new gaspi_group_t)
{

  GASPI_CHECK
    ( gaspi_group_create(_pgroup.get()) );

  for( auto rank : group_ranks ) {
    GASPI_CHECK
      (gaspi_group_add( *_pgroup, rank ) );
  }

  if( group_ranks.size() > 1 ) {
    GASPI_CHECK
      (gaspi_group_commit(*_pgroup,GASPI_BLOCK));
  }

}

Group
  ::Group(Group && other)
: _pgroup(other._pgroup.release())
{}

Group
  ::~Group()
{
  if(static_cast<bool>(_pgroup)) {
    GASPI_CHECK
      (gaspi_group_delete(*_pgroup));
  }
}

gaspi_group_t const &
Group
  ::group() const
{
  if(!static_cast<bool>(_pgroup)) {
    throw std::runtime_error
      (CODE_ORIGIN+"Group not active");
  }
  return *_pgroup;
}

Rank
Group
  ::size() const
{

  if(!static_cast<bool>(_pgroup)) {
    throw std::runtime_error
    (CODE_ORIGIN+"Group not active");
  }

  gaspi_number_t group_size;
  GASPI_CHECK
    ( gaspi_group_size ( *_pgroup, &group_size ) );

  return Rank(group_size);
}

Rank
Group
  ::rank() const
{
  std::unique_ptr<gaspi_rank_t[]> ranksInGroup
    (new gaspi_rank_t[size().get()]);

  GASPI_CHECK
    (gaspi_group_ranks
       ( group()
       , ranksInGroup.get()));

  gaspi_rank_t globalRank;

  GASPI_CHECK
    (gaspi_proc_rank(&globalRank) );

  Rank::Type groupRank ( 0 );
  while(globalRank != ranksInGroup[groupRank]) {
    groupRank++;
  }

  return Rank(groupRank);
}

gaspi_rank_t
groupToGlobalRank
 ( Group const & group
 , Rank  const & rank )
{
  if( (rank >= group.size()) ||
      (rank < Rank(0)) ) {
    std::stringstream ss;

    ss << "Invalid group Rank "
       << rank.get()
       << ". Allowed range [0,"
       << group.size().get()
       <<")!";

    throw std::runtime_error
    (CODE_ORIGIN+ss.str());
  }

  std::unique_ptr<gaspi_rank_t[]> ranksInGroup
    (new gaspi_rank_t[group.size().get()]);

  GASPI_CHECK
    (gaspi_group_ranks(group.group(), ranksInGroup.get()));

  return ranksInGroup[rank.get()];
}

Rank
globalToGroupRank
 ( Group const & group
 , gaspi_rank_t  const & rank )
{

  std::unique_ptr<gaspi_rank_t[]> ranksInGroup
    (new gaspi_rank_t[group.size().get()]);


  GASPI_CHECK
    (gaspi_group_ranks
       ( group.group()
       , ranksInGroup.get()));

  Rank groupRank (0);
  while(rank != ranksInGroup[groupRank.get()]) {
     ++groupRank;
  }

  if(groupRank == group.size()) {
    std::stringstream ss;

    ss << "Global Rank "
       << rank
       << " not part of group!";

    throw std::runtime_error
      (CODE_ORIGIN+ss.str());
  }

  return groupRank;
}


} /* namespace group */
} /* namespace gaspi */
