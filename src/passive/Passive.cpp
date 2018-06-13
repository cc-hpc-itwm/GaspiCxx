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
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 *
 * Passive.cpp
 *
 */

#include <GaspiCxx/passive/Passive.hpp>
#include <GaspiCxx/singlesided/Buffer.hpp>
#include <GaspiCxx/utility/Macros.hpp>
#include <GaspiCxx/utility/serialization.hpp>

extern "C" {
#include <GASPI.h>
}

#include <unistd.h>

namespace gaspi {
namespace passive {

Passive
  ::Passive
   ( segment::Segment & segment
   , Context & context )
: _segment(segment)
, _context(context)
, _passiveBufSize(std::min(static_cast<std::size_t>(1024),segment.size()/2))
, _passiveBufPointer( _segment.allocator().allocate(_passiveBufSize) )
{
	MAX_PASSIVE_MESSAGE_SIZE_ = ( ( _passiveBufSize - 4 * (int)sizeof(int) ) / 2 );  // maximal passive message size
    PASSIVE_SENDRECVBUF_SIZE_ = ( MAX_PASSIVE_MESSAGE_SIZE_
                                + (int)sizeof(int)                  // size for the message tag
                                + (int)sizeof(int)                  // size for the message size
                                );

    // doing the passive forwarding initalization
    {
        passive_isendrcv_bool_ = false;

        pthread_mutex_init( &passive_isendrcv_mutx_
                , NULL );
        pthread_cond_init ( &passive_isendrcv_cond_
                , NULL );

        passive_fwd_recv_bool_ = false;

        pthread_mutex_init( &passive_fwd_recv_mutx_
                , NULL );
        pthread_cond_init ( &passive_fwd_recv_cond_
                , NULL );
    }

    // For portability, explicitly create threads in a joinable state
    pthread_attr_init(&gpi_passive_thread_attr_);
    pthread_attr_setdetachstate
      ( &gpi_passive_thread_attr_
      , PTHREAD_CREATE_JOINABLE );

    // spawning the passive receive thread
    pthread_create( &gpi_passive_thread_id_
            , &gpi_passive_thread_attr_
            , &passive_thread_func_
            , reinterpret_cast<void *>(this) );
}

Passive
  ::~Passive()
{
  this->finish();

  _segment.allocator().deallocate
       ( reinterpret_cast<char*>(_passiveBufPointer)
       , _passiveBufSize );
}

bool
Passive
  ::sendError
   (const std::string & ErrMsg )
{
	const char * const pMessage = ErrMsg.c_str();
	int                size     = (int)ErrMsg.size();
	unsigned int       destRank = 0;

	return sendPassive( GERR
			          , pMessage
					  , size
					  , destRank
					  );
}

bool
Passive
  ::sendMessg
     ( const char *  const pData
	 , const size_t datSize
	 , const int targetRank )
{

  /// Request target buffer description as target for
  /// my data
  singlesided::BufferDescription targetBufferDesc;

  {
    _segment.remoteRegistration(targetRank);

    singlesided::Buffer tempBuffer
      ( _segment
      , serialization::size(targetBufferDesc) );

    {
      singlesided::BufferDescription const tempBufferDesc
          ( tempBuffer.description() );

      std::size_t const pkgSize( serialization::size(datSize )
                               + serialization::size(tempBufferDesc) );

      char * pkgBuffer( new char[pkgSize] );
      {
        char * cPtr( pkgBuffer );
        cPtr += serialization::serialize (cPtr, datSize);
        cPtr += serialization::serialize (cPtr, tempBufferDesc);
      }

      sendPassive
        ( DATE
        , pkgBuffer
        , pkgSize
        , targetRank );

      delete[] pkgBuffer;
    }

    if(!tempBuffer.waitForNotification()) {
      throw std::runtime_error(CODE_ORIGIN + "Unexpected behavior");
    }

    serialization::deserialize
      ( targetBufferDesc
      , tempBuffer.address() );
  }

  /// allocate source buffer
  singlesided::Buffer sourceBuffer
    ( _segment
    , datSize );

  memcpy
    ( sourceBuffer.address() //g_source_buffer
    , pData
    , datSize );

  // do the actual transfer
  {
    singlesided::BufferDescription
      sourceBufferDesc( sourceBuffer.description() );

    _context.write
      ( sourceBufferDesc
      , targetBufferDesc );

    _context.flush();
  }

  return true;
}

bool
Passive::recvMessg( std::vector<char> & Data
		          , int & rank )
{
  bool finished = false;

  Data.resize(0);

  while(!finished)
  {

    // two thread barrier
    pthread_mutex_lock(&passive_fwd_recv_mutx_);
    {
      if (!passive_fwd_recv_bool_)
      {
          passive_fwd_recv_bool_ = true;

          pthread_cond_wait( &passive_fwd_recv_cond_
                           , &passive_fwd_recv_mutx_ );
      }
      else
      {
          passive_fwd_recv_bool_ = false;

          pthread_cond_signal(&passive_fwd_recv_cond_);
      }
    }
    pthread_mutex_unlock(&passive_fwd_recv_mutx_);


    { // copy the data

      size_t DataSize = Data.size();

      Data.resize( DataSize + passive_msg_size_ );
      rank = passive_msg_rank_;

      passive_msg_ptr_->waitForNotification();

      memcpy( reinterpret_cast<void *>(&Data[DataSize])
            , reinterpret_cast<void *>(passive_msg_ptr_->address())
            , passive_msg_size_);

      passive_msg_ptr_.reset(nullptr);

      finished = true;
    }

    // two thread barrier
    pthread_mutex_lock(&passive_fwd_recv_mutx_);
    {
      if (!passive_fwd_recv_bool_)
      {
          passive_fwd_recv_bool_ = true;

          pthread_cond_wait( &passive_fwd_recv_cond_
                  , &passive_fwd_recv_mutx_ );
      }
      else
      {
          passive_fwd_recv_bool_ = false;

          pthread_cond_signal(&passive_fwd_recv_cond_);
      }
    }
    pthread_mutex_unlock(&passive_fwd_recv_mutx_);
  }

  return true;
}

bool
Passive
  ::iSendTagMessg
    ( const int rank
    , const int tag
    , singlesided::Buffer & srcTargetBuffer )
{
  bool ret(false);

  // list of src target buffer for tag and rank
  //    std::map<std::pair<tag,rank> , BufferDescription> >
  // list of rcv target buffer for tag and rank
  //    std::map<std::pair<Tag,Rank> , BufferDescription> >

  // iSendTagMessg:
  // look for queued rcv target buffer with given tag and rank
  // if there
  //    initiate data transfer
  //    wait for completion
  //    do local notification of src target buffer
  // if not there
  //    append src target buffer to corresponding queue

  // passive thread
  // incoming rcv target buffer description
  // look for queued src target buffer with given tag and rank
  // if there
  //    initiate data transfer
  //    wait for completion
  //    do local notification of src target buffer
  // if not there
  //    append rcv target buffer to corresponding queue

  // iRecvTagMessg
  // send rcv target buffer description to passive sender thread

  singlesided::BufferDescription const
    srcTargetBufferDesc
      ( srcTargetBuffer.description() );

  Key key(tag,rank);

  pthread_mutex_lock(&passive_isendrcv_mutx_);

  auto search (rcvTargetBuffers_.find(key));

  if(search!=rcvTargetBuffers_.end()) {
    // found matching rcvTargetBuffer
    singlesided::BufferDescription const
      rcvTargetBufferDesc
        (search->second);

    rcvTargetBuffers_.erase(search);

    pthread_mutex_unlock(&passive_isendrcv_mutx_);

    // do the communication

    iSendRecvComm( srcTargetBufferDesc
                 , rcvTargetBufferDesc
                 , _context );

  }
  else {
    // did not find matching rcvTargetBuffer
    // append srcTargetBuffer

    auto ret = srcTargetBuffers_.insert
      ( std::make_pair(key,srcTargetBufferDesc) );

    pthread_mutex_unlock(&passive_isendrcv_mutx_);

    if(!ret.second) {
      throw std::runtime_error("key exists already");
    }
  }

  return ret;
}

bool
Passive
  ::iRecvTagMessg
    ( const int rank
    , const int tag
    , singlesided::Buffer & rcvTargetBuffer )
{
  // iRecvTagMessg
  // send rcv target buffer description to passive sender thread

  bool ret(false);

  singlesided::BufferDescription rcvTargetBufferDesc
      ( rcvTargetBuffer.description() );

  std::size_t const pkgSize( serialization::size(tag)
                           + serialization::size(rcvTargetBufferDesc) );

  {
    char * const pkgBuffer( new char[pkgSize] );
    {
      char * cPtr( pkgBuffer );
      cPtr += serialization::serialize (cPtr, tag);
      cPtr += serialization::serialize (cPtr, rcvTargetBufferDesc);
    }

    sendPassive
      ( RTBD
      , pkgBuffer
      , pkgSize
      , rank );

    delete[] pkgBuffer;
  }

  ret = true;

  return ret;
}

void
Passive
  ::iSendRecvComm
    ( singlesided::BufferDescription const & srcTargetBufferDesc
    , singlesided::BufferDescription const & rcvTargetBufferDesc
    , Context const & context )
{
  context.write
    ( srcTargetBufferDesc
    , rcvTargetBufferDesc );

  context.flush
    ();

  context.notify
    ( srcTargetBufferDesc );

  context.flush
    ();
}

gaspi_rank_t
Passive
  ::global_rank()
{
  gaspi_rank_t rank;
  GASPI_CHECK(gaspi_proc_rank(&rank));

  return rank;
}

// TODO assure FNSH tag is received before communication is shut down
//      (e.g. wait for ACK before return to calling function)
bool
Passive::finish()
{
  gaspi_rank_t const rank(global_rank());

  const char * const pMessage = NULL;
  int                size     = 0;
  unsigned int       destRank = rank;

  sendPassive( FNSH
             , pMessage
             , size
             , destRank
             );

  /* Wait for the passive thread to complete */
  pthread_join( gpi_passive_thread_id_
              , NULL );


  /* Clean up and exit */
  pthread_attr_destroy(&gpi_passive_thread_attr_);

  // doing passive forwarding  cleanup
  {
    pthread_mutex_destroy(&passive_fwd_recv_mutx_);
    pthread_cond_destroy (&passive_fwd_recv_cond_);

    pthread_mutex_destroy(&passive_isendrcv_mutx_);
    pthread_cond_destroy (&passive_isendrcv_cond_);
  }

  return true;
}

void *
Passive::passive_thread_func_(void * arg)
{
  Passive * pPassiveCommMan(reinterpret_cast<Passive *>(arg));

  bool FINISHED = false;

  while (!FINISHED)
  {
    int                   msg_tag;
    int                   msg_size;
    int                   msg_rank;
    std::unique_ptr<char> msg_ptr;

    msg_ptr = pPassiveCommMan->recvPassive( msg_tag
                                          , msg_size
                                          , msg_rank );

    switch (msg_tag)
    {
      case GERR:
      {

          std::string buf_string(msg_ptr.get(),msg_size);

          size_t sub_beg(0);
          size_t sub_end(0);

          std::cout<<std::endl<<std::endl;

          while( sub_beg < buf_string.size() )
          {
              size_t const pos = buf_string.find('\n', sub_beg );

              sub_end = ( pos == std::string::npos ) ? buf_string.size()
                      : pos;

              std::cout<<"Exception[ rank "<<msg_rank<<" ] : ";
              std::cout<<buf_string.substr( sub_beg , sub_end - sub_beg )
                       <<std::endl;

              sub_beg = sub_end + 1;
          }

          std::cout<<"Exception[ rank "<<msg_rank<<" ] : ";
          raise(SIGINT);
          break;
      }

      case FNSH:
      {
          FINISHED = true;
          break;
      }

      case RTBD:
      {
        Tag rcvTargetBufferTag;
        singlesided::BufferDescription rcvTargetBufferDesc;
        {
            char * cPtr( msg_ptr.get() );
            cPtr += serialization::deserialize
                ( rcvTargetBufferTag, cPtr);
            cPtr += serialization::deserialize
                ( rcvTargetBufferDesc, cPtr);

            msg_ptr.reset( nullptr );
        }

        Key key(rcvTargetBufferTag,msg_rank);

        pthread_mutex_lock(&pPassiveCommMan->passive_isendrcv_mutx_);

        auto search (pPassiveCommMan->srcTargetBuffers_.find(key));

        if(search!=pPassiveCommMan->srcTargetBuffers_.end()) {
          // found matching rcvTargetBuffer
          singlesided::BufferDescription const
            srcTargetBufferDesc
              (search->second);

          pPassiveCommMan->srcTargetBuffers_.erase(search);

          pthread_mutex_unlock(&pPassiveCommMan->passive_isendrcv_mutx_);

          // do the communication

          pPassiveCommMan->iSendRecvComm( srcTargetBufferDesc
                                        , rcvTargetBufferDesc
                                        , pPassiveCommMan->_context );

        }
        else {
          // did not find matching srcTargetBuffer
          // append rcvTargetBuffer

          auto ret = pPassiveCommMan->rcvTargetBuffers_.insert
            ( std::make_pair(key,rcvTargetBufferDesc) );

          pthread_mutex_unlock(&pPassiveCommMan->passive_isendrcv_mutx_);

          if(!ret.second) {
            std::cout << "key exists already" << std::endl;
            throw std::runtime_error("key exists already");
          }
        }

        break;
      }

      case DATP:
      {
          // make the same as DATE
      }

      case DATE:
      {

        { // do nothing here

          std::size_t g_requested_size;

          singlesided::BufferDescription remoteInfoBufferDesc;
          {
              char * cPtr( msg_ptr.get() );
              cPtr += serialization::deserialize
                  ( g_requested_size, cPtr);
              cPtr += serialization::deserialize
                  ( remoteInfoBufferDesc, cPtr);

              msg_ptr.reset( nullptr );
          }

          segment::Segment & _segment( pPassiveCommMan->_segment );

          _segment.remoteRegistration(remoteInfoBufferDesc.rank());

          std::unique_ptr<singlesided::Buffer>
            pRequestedBuffer( new singlesided::Buffer
              ( _segment
              , g_requested_size ) );

          {

            singlesided::BufferDescription descr
              ( pRequestedBuffer->description() );

            singlesided::Buffer g_temp_buffer
              ( _segment
              , serialization::size(descr) );

            {
              Context & _context(pPassiveCommMan->_context);

              serialization::serialize( g_temp_buffer.address(), descr);

              _context.write( g_temp_buffer.description()
                            , remoteInfoBufferDesc );

              _context.flush();
            }
          }

          pPassiveCommMan->passive_msg_tag_  = -1;
          pPassiveCommMan->passive_msg_size_ = g_requested_size;
          pPassiveCommMan->passive_msg_rank_ = msg_rank;
          pPassiveCommMan->passive_msg_ptr_  = std::move(pRequestedBuffer);
        }

        pthread_mutex_lock(&pPassiveCommMan->passive_fwd_recv_mutx_);
        {
          if (!pPassiveCommMan->passive_fwd_recv_bool_)
          {
            pPassiveCommMan->passive_fwd_recv_bool_ = true;

            pthread_cond_wait( &pPassiveCommMan->passive_fwd_recv_cond_
                             , &pPassiveCommMan->passive_fwd_recv_mutx_ );
          }
          else
          {
            pPassiveCommMan->passive_fwd_recv_bool_ = false;

            pthread_cond_signal(&pPassiveCommMan->passive_fwd_recv_cond_);
          }
        }
        pthread_mutex_unlock(&pPassiveCommMan->passive_fwd_recv_mutx_);




        pthread_mutex_lock(&pPassiveCommMan->passive_fwd_recv_mutx_);
        {
          if (!pPassiveCommMan->passive_fwd_recv_bool_)
          {
            pPassiveCommMan->passive_fwd_recv_bool_ = true;

            pthread_cond_wait( &pPassiveCommMan->passive_fwd_recv_cond_
                             , &pPassiveCommMan->passive_fwd_recv_mutx_ );
          }
          else
          {
            pPassiveCommMan->passive_fwd_recv_bool_ = false;

            pthread_cond_signal(&pPassiveCommMan->passive_fwd_recv_cond_);
          }
        }
        pthread_mutex_unlock(&pPassiveCommMan->passive_fwd_recv_mutx_);

        break;
      }
    }
  }

  return NULL;
}

bool
Passive
  ::sendPassive( int msg_tag
		       , const char * const pMessage
               , int srcSize
               , unsigned int destRank )
{
	//const unsigned long localOffset( PASSIVE_SENDBUF_OFF_ );
	const int           size( (int)(srcSize + 2 * sizeof(int) ) );
	const unsigned int  rank(destRank);

	char * g_ptr (_segment.allocator().allocate(size));

	int * const pMsgTag  = reinterpret_cast<int *> ( g_ptr + 0 * sizeof(int) );
	int * const pMsgSize = reinterpret_cast<int *> ( g_ptr + 1 * sizeof(int) );
	char * const pMsg    = reinterpret_cast<char *>( g_ptr + 2 * sizeof(int) );

	*pMsgTag  = msg_tag;
	*pMsgSize = srcSize;

	memcpy( reinterpret_cast<void *>(pMsg)
		  , reinterpret_cast<const void *>(pMessage)
		  , srcSize );

	GASPI_CHECK
	  ( gaspi_passive_send
	    ( _segment.id()
	    , _segment.pointerToOffset(g_ptr)
	    , rank
	    , size
	    , GASPI_BLOCK ) );

	_segment.allocator().deallocate(g_ptr,size);

	return true;
}

std::unique_ptr<char>
Passive
  ::recvPassive
   ( int & msg_tag
   , int & size
   , int & srcRank )
{
    gaspi_rank_t  senderRank;

    char * g_ptr (_segment.allocator().allocate(PASSIVE_SENDRECVBUF_SIZE_) );

    GASPI_CHECK
      ( gaspi_passive_receive
        ( _segment.id()
        , _segment.pointerToOffset(g_ptr)
        , &senderRank
        , PASSIVE_SENDRECVBUF_SIZE_
        , GASPI_BLOCK ) );

    srcRank = senderRank;

	int * const  pMsgTag  = reinterpret_cast<int *> ( g_ptr + 0L * sizeof(int) );
	int * const  pMsgSize = reinterpret_cast<int *> ( g_ptr + 1L * sizeof(int) );
	char * const pMsgData = reinterpret_cast<char *>( g_ptr + 2L * sizeof(int) );

	msg_tag = *pMsgTag;
	size = *pMsgSize;

	std::unique_ptr<char> pMsg( new char[size] );


    memcpy( reinterpret_cast<void *>(pMsg.get())
          , reinterpret_cast<const void *>(pMsgData)
          , size );

	_segment.allocator().deallocate(g_ptr,PASSIVE_SENDRECVBUF_SIZE_);

	return pMsg;
}

}   // end of namespace passive
} 	// end of namespace gaspi
