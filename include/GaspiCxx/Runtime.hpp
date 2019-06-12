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
 * Runtime.hpp
 *
 */

#include <cstring>
#include <memory>
#include <GaspiCxx/Context.hpp>

extern "C" {
#include <GASPI.h>
}


#ifndef GASPIRUNTIME_HPP
#define GASPIRUNTIME_HPP

namespace gaspi {

namespace segment { class Segment; }
namespace passive { class Passive; }


  class RuntimeBase {

     public:

       RuntimeBase
         ();

       ~RuntimeBase
         ();
   };

  //! This class provides an runtime for GASPI communication.
  //! It basically combines a GASPI group and a GASPI segment.
  //! \brief   Abstraction of a GASPI segment and group
  //! \warning Each GASPI interface must have its own GASPI segment
  //!          and GASPI queue that are not used by the user
  //!          application in any other way!
  //!          Therefore, interfaces do not provide a copy constructor
  //!          or an assignment operator.
  class Runtime : public RuntimeBase
                , public Context
  {
  private:

    std::unique_ptr<segment::Segment> _psegment;
    std::unique_ptr<passive::Passive> _ppassive;

    //! A runtime cannot be copied.
    Runtime
      (const Runtime&) = delete;

    Runtime&
    operator=(const Runtime&) = delete;


  public:

    using Rank = gaspi_rank_t;

    //! Construct a GASPI interface from a group and a segment.
    //! \note GASPI and the given segment must be initialized on
    //!       all ranks of the given group!
    Runtime
      ();
    //! The destructor
    ~Runtime
      ();

    //! Return the segment
    segment::Segment &
    segment()  {
      return *_psegment;
    }

    passive::Passive &
    passive() {
      return *_ppassive;
    }
  };

  bool
  isRuntimeAvailable();

  Runtime &
  getRuntime();

} // namespace gaspi

#endif // GASPIRUNTIME_HPP

