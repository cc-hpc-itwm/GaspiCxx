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
 * Runtime.hpp
 *
 */

#include <cstring>
#include <memory>
#include <GaspiCxx/CommunicationContext.hpp>
#include <GaspiCxx/SingleQueueContext.hpp>
#include <GaspiCxx/collectives/Barrier.hpp>
#include <GaspiCxx/progress_engine/ProgressEngine.hpp>
#include <GaspiCxx/RuntimeConfiguration.hpp>
#include <GaspiCxx/segment/SegmentPool.hpp>

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
                , public SingleQueueContext
  {
  private:

    group::Group _group_all;
    std::size_t _segmentSize;
    std::unique_ptr<segment::Segment> _psegment;
    std::unique_ptr<passive::Passive> _ppassive;
    std::unique_ptr<segment::SegmentPool> _psegment_pool;
    std::unique_ptr<CommunicationContext> _pcomm_context;
    std::unique_ptr<progress_engine::ProgressEngine> _pengine;
    std::unique_ptr<gaspi::collectives::blocking::Barrier> _pglobal_barrier;

    //! A runtime is a singleton.
    Runtime
      ();

    Runtime
      (const Runtime&) = delete;

    Runtime&
    operator=(const Runtime&) = delete;


  public:

    //! Global library configuration
    inline static RuntimeConfiguration configuration{
                        SegmentPoolType::DynamicSegmentPool,
                        ProgressEngineType::RoundRobinDedicatedThread,
                        CommunicationContextType::RoundRobinQueues};

    //! Construct a GASPI interface from a group and a segment.
    //! \note GASPI and the given segment must be initialized on
    //!       all ranks of the given group!
    static
    Runtime &
    getRuntime();

    ~Runtime();

    //! Return the segment
    segment::Segment &
    segment();

    passive::Passive &
    passive();

    segment::Segment &
    getFreeSegment(std::size_t size);

    progress_engine::ProgressEngine &
    getDefaultProgressEngine();

    CommunicationContext &
    getDefaultCommunicationContext();

    void
    synchCurrentWorkingDirectory();

    void
    barrier();

    group::GlobalRank
    global_rank();

    std::size_t
    size();
  };

  Runtime &
  getRuntime();

  void
  initGaspiCxx();

  void
  initGaspiCxx(RuntimeConfiguration const&);

} // namespace gaspi

#endif // GASPIRUNTIME_HPP

