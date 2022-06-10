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
 * Passive.hpp
 *
 */

#ifndef PASSIVE_H
#define PASSIVE_H

#include <GaspiCxx/CommunicationContext.hpp>
#include <memory>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/utility/ScopedAllocation.hpp>

#include <execinfo.h>
#include <signal.h>
#include <string.h>

#include <sstream>

#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <cstdio>

/// standard includes
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <limits>
#include <pthread.h>
#include <tuple>
#include <vector>
#include <cstring>
#include <map>

namespace gaspi {

namespace singlesided {
namespace write {
class TargetBuffer;
}
}

namespace passive {

class Passive
{
    using Rank = unsigned int;

  public:

    using Tag = unsigned int;

    Passive
      ( segment::Segment &
      , CommunicationContext & );

    ~Passive
      ();

    // \brief sending an error message to the master
      //        process with rank = 0
    bool
    sendError
      ( const std::string & ErrMsg );

    // \brief sending an ordinary message to the process
    //        having rank
    bool
    sendMessg
      ( const char *  const pData
      , const size_t  DataSize
      , const Rank rank );

    // \brief receiving a message
    bool
    recvMessg( std::vector<char> & Data
             , Rank & rank );

    bool
    iSendTagMessg
      ( const Rank rank
      , const Tag tag
      , singlesided::Buffer & );

    bool
    iRecvTagMessg
      ( const Rank rank
      , const Tag tag
      , singlesided::Buffer & );

  private:

    gaspi_rank_t
    global_rank();

    // \brief telling the spawned process to finish
    bool
    finish();

    static void *
    passive_thread_func_(void * args);

    bool
    sendPassive( unsigned int msg_tag
               , const char * const pMessage
               , std::size_t size
               , Rank destRank );

    std::unique_ptr<char[]>
    recvPassive( unsigned int & msg_tag
               , std::size_t & size
               , Rank & srcRank );

    void
    iSendRecvComm
      ( singlesided::BufferDescription const & srcTargetBufferDesc
      , singlesided::BufferDescription const & rcvTargetBufferDesc
      , CommunicationContext & context );

    void
        handleTaggedSrcBufferDesc
         ( Rank rank
         , Tag tag
         , singlesided::BufferDescription const & srcTargetBufferDesc );

    void
    handleTaggedRcvBufferDesc
         ( Rank rank
         , Tag tag
         , singlesided::BufferDescription const & rcvTargetBufferDesc );

    segment::Segment & _segment;
    CommunicationContext &          _context;

    std::size_t _passiveBufSize;
    void *      _passiveBufPointer;


    pthread_t      gpi_passive_thread_id_;
    pthread_attr_t gpi_passive_thread_attr_;

    bool            passive_isendrcv_bool_;
    pthread_mutex_t passive_isendrcv_mutx_;
    pthread_cond_t  passive_isendrcv_cond_;

    bool            passive_fwd_recv_bool_;
    pthread_mutex_t passive_fwd_recv_mutx_;
    pthread_cond_t  passive_fwd_recv_cond_;

    enum class MSG_TAG_t { GERR	  // global error
                         , FNSH	  // finish passive communication thread
                         , DATP   // communicate partial data
                         , DATE   // communicate end data
                         , RTBD   // receive target buffer description
                         };

    std::size_t          passive_msg_size_;
    Rank                 passive_msg_rank_;
    std::unique_ptr<singlesided::Buffer> passive_msg_ptr_;


    std::size_t           MAX_PASSIVE_MESSAGE_SIZE_;
    std::size_t           PASSIVE_SENDRECVBUF_SIZE_;

    using Key = std::tuple<Tag,Rank>;

    std::map<Key,singlesided::BufferDescription> srcTargetBuffers_;
    std::map<Key,singlesided::BufferDescription> rcvTargetBuffers_;
};

}	// end of namespace passive
}   // end of namespace gaspi

#endif /* PASSIVECOMMMAN_H_ */
