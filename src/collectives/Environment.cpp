/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
 * 
 * This file is part of GaspiLS.
 * 
 * GaspiLS is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 * 
 * GaspiLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GaspiLS. If not, see <http://www.gnu.org/licenses/>.
 */


// GASPIInterface.cpp - implementation of the GASPIInterface class

#include <Environment.hpp>
#include <passive/Passive.hpp>
#include <Runtime.hpp>
#include <segment/Segment.hpp>

#include <utility/Macros.hpp>

#include <sstream>
#include <cstring> // for memcpy()
#include <exception>
//#include <Thread.h>
extern "C" {
#include <GASPI.h>
#include <GASPI_Ext.h>
}


namespace gaspi
{

  namespace detail {

    gaspi_rank_t
    globalToGroupRank( gaspi_rank_t const & glbRank
                     , gaspi_rank_t const * const ranksInGroup
                     , gaspi_rank_t const & groupSize ) {

      gaspi_rank_t groupRank(0);

      for (; groupRank != groupSize; ++groupRank) {
        if (ranksInGroup[groupRank] == glbRank) {
            return groupRank;
        }
      }

      std::stringstream ss;

      ss << "Rank " << glbRank << "is not part of group: (";
      for(int i(0);i<groupSize;++i) {
        if(i == 0) {
          ss << ", " << ranksInGroup[i];
        }
        else {
          ss << ranksInGroup[i];
        }
      }
      ss << ")";

      throw std::runtime_error(ss.str());
    }

  }

  Environment
    ::Environment
    ( gaspi_group_t      group
    , gaspi_segment_id_t segmentID
    , gaspi_queue_id_t   queue )
  : _group(group)
  , _queue(queue)
  , _segment(segmentID)
  {
    GASPI_CHECK(gaspi_proc_rank(&_rank));
    GASPI_CHECK(gaspi_group_size(_group, &_groupSize));

    _ranksInGroup                 = NULL;
    _allreduceBufferRemoteOffsets = NULL;
    try {
      _ranksInGroup                 = new gaspi_rank_t[_groupSize];
      _allreduceBufferRemoteOffsets = new gaspi_offset_t[_groupSize];
    }
    catch(const std::exception &e) {
      delete[] _ranksInGroup;
      delete[] _allreduceBufferRemoteOffsets;
      throw;
    }

    GASPI_CHECK(gaspi_group_ranks(_group, _ranksInGroup));
    _groupRank = 0;
    while(_rank != _ranksInGroup[_groupRank]) _groupRank++;

    GASPI_CHECK(gaspi_queue_size_max(&_maxQueueSize));

    // Set up allreduce buffer and related offsets.
    _g_allreduceBuffer           = NULL;
    try {
      _g_allreduceBuffer
        = _segment.allocator().allocate
          ( _groupSize
          * MAX_SCALAR_TYPE_SIZE
          * MAX_SCALAR_TYPE_NUM );

      gaspi_offset_t allreduceBufferOffset
        ( _segment.pointerToOffset(_g_allreduceBuffer) );

      allgatherPassive
        ( &allreduceBufferOffset
        , _allreduceBufferRemoteOffsets
        , sizeof(gaspi_offset_t) );

    }
    catch(const std::exception &e) {
      _segment.allocator().deallocate( reinterpret_cast<char*>(_g_allreduceBuffer)
                            , _groupSize
                            * MAX_SCALAR_TYPE_SIZE
                            * MAX_SCALAR_TYPE_NUM );
      throw;
    }
  }

  Environment::~Environment()
  {
    _segment.allocator().deallocate( reinterpret_cast<char*>(_g_allreduceBuffer)
                          , _groupSize
                          * MAX_SCALAR_TYPE_SIZE
                          * MAX_SCALAR_TYPE_NUM );
    delete[] _allreduceBufferRemoteOffsets;
    delete[] _ranksInGroup;
  }

  gaspi_rank_t Environment::rank() const
  {
    return _rank;
  }

  gaspi_group_t Environment::group() const
  {
    return _group;
  }

  gaspi_queue_id_t Environment::queue() const
  {
    return _queue;
  }

  gaspi_number_t Environment::groupSize() const
  {
    return _groupSize;
  }

  gaspi_rank_t Environment::groupRank() const
  {
    return _groupRank;
  }

  gaspi_number_t Environment::maxQueueSize() const
  {
    return _maxQueueSize;
  }


  void Environment::writeNotify( const void              *g_source,
                                 gaspi_rank_t            groupRank,
                                 gaspi_offset_t          remoteOffset,
                                 size_t                  size,
                                 gaspi_notification_id_t notificationID,
                                 gaspi_notification_t    notificationValue,
                                 gaspi_timeout_t         timeout)
                                 const
  {
    gaspi_number_t queueSize;
    GASPI_CHECK(gaspi_write_notify(_segment.id(),
                                   _segment.pointerToOffset(g_source),
                                   _ranksInGroup[groupRank],
                                   _segment.id(),
                                   remoteOffset,
                                   size,
                                   notificationID,
                                   notificationValue,
                                   _queue,
                                   timeout));
    GASPI_CHECK(gaspi_queue_size(_queue, &queueSize));
    if(queueSize >= _maxQueueSize) {
      GASPI_CHECK(gaspi_wait(_queue, GASPI_BLOCK));
    }
  }

  void
  Environment
    ::notify
     ( gaspi_rank_t            groupRank,
       gaspi_notification_id_t notificationID,
       gaspi_notification_t    notificationValue,
       gaspi_timeout_t         timeout)
       const
  {
    gaspi_number_t queueSize;
    GASPI_CHECK(gaspi_notify(_segment.id(),
                             _ranksInGroup[groupRank],
                             notificationID,
                             notificationValue,
                             _queue,
                             timeout));
    GASPI_CHECK(gaspi_queue_size(_queue, &queueSize));
    if(queueSize >= _maxQueueSize) {
      GASPI_CHECK(gaspi_wait(_queue, GASPI_BLOCK));
    }
  }

  void Environment::waitQueue() const
  {
    GASPI_CHECK(gaspi_wait(_queue, GASPI_BLOCK));
  }

  void
  Environment
    ::waitNotificationsIgnoreStatus( int startID
                                   , int numIDs
                                   , int numWaits ) const
  {
    // We do not really need id and value, but the GASPI API requires them.
    gaspi_notification_id_t id;
    gaspi_notification_t    value;

    for(int i = 0; i < numWaits; i++) {
      GASPI_CHECK(gaspi_notify_waitsome(_segment.id(),
                                        startID,
                                        numIDs,
                                        &id,
                                        GASPI_BLOCK));
      GASPI_CHECK(gaspi_notify_reset(_segment.id(), id, &value));
    }
  }

  /// --------------------------------------------------------------------------
  /// collective operations
  /// --------------------------------------------------------------------------

  void Environment::barrier() const
  {
    if(_groupSize > 1) {
      GASPI_CHECK(gaspi_barrier(_group, GASPI_BLOCK));
    }
  }

  // Specialization of allreduce for float.
  // This makes sure that the standard gaspi_allreduce is used
  // whenever possible, since it is probably the fastest.
#ifdef ETI_FLOAT
  template <>
  float Environment::allreduce(float             localValue,
                                  gaspi_operation_t operation)
                                  const
  {
    float result(localValue);
    if(_groupSize > 1) {
      GASPI_CHECK(gaspi_allreduce(&localValue,
                                  &result,
                                  1,
                                  operation,
                                  GASPI_TYPE_FLOAT,
                                  _group,
                                  GASPI_BLOCK));
    }

    return result;
  }

  template <>
  void Environment::allreduce ( float * const localValues,
                                float * const globalValues,
                                int numValues,
                                gaspi_operation_t operation) const {

    if(_groupSize > 1) {
      GASPI_CHECK(gaspi_allreduce(reinterpret_cast<gaspi_pointer_t>(localValues),
                                  reinterpret_cast<gaspi_pointer_t>(globalValues),
                                  numValues,
                                  operation,
                                  GASPI_TYPE_FLOAT,
                                  _group,
                                  GASPI_BLOCK));
    }
    else {
      for(int i(0);i<numValues;++i) {
        globalValues[i] = localValues[i];
      }
    }
  }
#endif

  // Specialization of allreduce for double.
  // This makes sure that the standard gaspi_allreduce is used
  // whenever possible, since it is probably the fastest.
#ifdef ETI_DOUBLE
  template <>
  double Environment::allreduce(double            localValue,
                                   gaspi_operation_t operation)
                                   const
  {
    double result(localValue);

    if(_groupSize > 1) {
      GASPI_CHECK(gaspi_allreduce(&localValue,
                                  &result,
                                  1,
                                  operation,
                                  GASPI_TYPE_DOUBLE,
                                  _group,
                                  GASPI_BLOCK));
    }

    return result;
  }

  template <>
  void Environment::allreduce(double * const localValues,
                                 double * const globalValues,
                                 int numValues,
                                 gaspi_operation_t operation) const {

    if(_groupSize > 1) {
      GASPI_CHECK(gaspi_allreduce(localValues,
                                  globalValues,
                                  numValues,
                                  operation,
                                  GASPI_TYPE_DOUBLE,
                                  _group,
                                  GASPI_BLOCK));
    }
    else {
      for(int i(0);i<numValues;++i) {
        globalValues[i] = localValues[i];
      }
    }
  }
#endif

void
Environment
  ::allgatherRemoteOffsets
     ( const void *g_send
     , void       *g_receive
     , size_t     size)
  {

    if (size == 0) return;

    if (size > MAX_SCALAR_TYPE_SIZE * MAX_SCALAR_TYPE_NUM ) {
      allgatherPassive(g_send,g_receive,size);
    }

    char * const g_char_receive ( static_cast<char*>(_g_allreduceBuffer) );

    std::memcpy( &g_char_receive[_groupRank*size]
                                 , g_send
                                 , size );

    for( int i = 0
        ; i < static_cast<int>(_groupSize)
        ; i++) {
      if(i == _groupRank) continue;
      writeNotify( &g_char_receive[_groupRank*size]
                 , i
                 , _allreduceBufferRemoteOffsets[i] + _groupRank*size
                 , size
                 , _groupRank );
    }
    waitNotificationsIgnoreStatus(0, _groupSize, _groupSize - 1);

    std::memcpy( g_receive
               , g_char_receive
               , _groupSize*size );

    barrier();

    waitQueue();

}

void
Environment
  ::allgatherPassive
   ( const void *sendbuf
   , void       *recvbuf
   , size_t     size )
{

  if (size == 0) return;

  passive::Passive & passive(getRuntime().passive());

  if( _groupRank == 0 ) {

    std::memcpy( &static_cast<char*>(recvbuf)[_groupRank*size]
               , sendbuf
               , size );

    for( int i = 0
             ;     i < static_cast<int>(_groupSize) - 1
             ;   ++i ) {

      std::vector<char> tmp_buf;
      int               senderRank;
      passive.recvMessg
        ( tmp_buf
        , senderRank );


      gaspi_rank_t senderGroupRank
          ( detail::globalToGroupRank( senderRank
                                     , _ranksInGroup
                                     , _groupSize ) );

      std::memcpy( &static_cast<char*>(recvbuf)[senderGroupRank*size]
                 , &tmp_buf.front()
                 , tmp_buf.size() );
    }

    for( int i = 1
        ;     i < static_cast<int>(_groupSize)
        ;   ++i ) {
      passive.sendMessg
              ( reinterpret_cast<const char * const>(recvbuf)
              , _groupSize * size
              , _ranksInGroup[i] );
    }

  } else {

    passive.sendMessg
      ( reinterpret_cast<const char * const>(sendbuf)
      , size
      , _ranksInGroup[0] );

    std::vector<char> tmp_buf;
    int               senderRank;
    passive.recvMessg
      ( tmp_buf
      , senderRank );

    std::memcpy( recvbuf
               , &tmp_buf.front()
               , tmp_buf.size() );
  }

  barrier();

}

void
Environment
  ::allgather( const void *g_send
             , void       *g_receive
             , size_t     size)
{
  if(size == 0) return;

  GASPIScopedAllocation<gaspi_offset_t> g_remoteOffsets(*this, _groupSize);

  g_remoteOffsets[_groupRank] = _segment.pointerToOffset(g_receive);
  allgatherRemoteOffsets(&g_remoteOffsets[_groupRank],
                   g_remoteOffsets.pointer(),
                   sizeof(gaspi_offset_t));

  for(int i = 0; i < static_cast<int>(_groupSize); i++) {
    writeNotify(g_send,
                i,
                g_remoteOffsets[i] + _groupRank*size,
                size,
                _groupRank);
  }
  waitNotificationsIgnoreStatus(0, _groupSize, _groupSize);

  barrier();

  waitQueue();
}

void Environment::allgatherv( const void   *g_send,
                              void         *g_receive,
                              const size_t *sizes,
                              const size_t sizeMultiplicator )
{
  gaspi_offset_t offset;
  GASPIScopedAllocation<gaspi_offset_t> g_remoteOffsets(*this, _groupSize);

  g_remoteOffsets[_groupRank] = _segment.pointerToOffset(g_receive);
  allgatherRemoteOffsets(&g_remoteOffsets[_groupRank],
                   g_remoteOffsets.pointer(),
                   sizeof(gaspi_offset_t));

  offset = 0;
  for(int i = 0; i < _groupRank; i++) {
    offset += sizes[i]*sizeMultiplicator;
  }

  for(int i = 0; i < static_cast<int>(_groupSize); i++) {
    // Apparently, GASPI does not send a notification when
    // gaspi_write_notify() is called with size zero,
    // so we have to check this case manually!
    if(sizes[_groupRank] == 0) {
      notify(i, _groupRank);
      continue;
    }
    writeNotify(g_send,
                i,
                g_remoteOffsets[i] + offset,
                sizes[_groupRank]*sizeMultiplicator,
                _groupRank);
  }
  waitNotificationsIgnoreStatus(0, _groupSize, _groupSize);

  barrier();

  waitQueue();
}

void Environment::alltoall(const void *g_send,
                              void       *g_receive,
                              size_t     size)
{
  if(size == 0) return;

  // Implementation is very similar to allgather()!
  GASPIScopedAllocation<gaspi_offset_t> g_remoteOffsets(*this, _groupSize);

  g_remoteOffsets[_groupRank] = _segment.pointerToOffset(g_receive);
  allgatherRemoteOffsets(&g_remoteOffsets[_groupRank],
                   g_remoteOffsets.pointer(),
                   sizeof(gaspi_offset_t));

  for(int i = 0; i < static_cast<int>(_groupSize); i++) {
    // Just write different data to each rank.
    writeNotify(&static_cast<const char*>(g_send)[i*size],
                i,
                g_remoteOffsets[i] + _groupRank*size,
                size,
                _groupRank);
  }
  waitNotificationsIgnoreStatus(0, _groupSize, _groupSize);

  barrier();

  waitQueue();
}

void Environment::alltoallv(const void   *g_send,
                               const size_t *sendSizes,
                               void         *g_receive,
                               const size_t *receiveSizes,
                               size_t       sizeMultiplicator)
{
  GASPIScopedAllocation<gaspi_offset_t> g_localOffsets(*this, _groupSize);
  GASPIScopedAllocation<gaspi_offset_t> g_remoteOffsets(*this, _groupSize);

  g_localOffsets[0] = _segment.pointerToOffset(g_receive);
  for(int i = 1; i < static_cast<int>(_groupSize); i++) {
    g_localOffsets[i] = g_localOffsets[i - 1]
                        + receiveSizes[i - 1]*sizeMultiplicator;
  }

  alltoall( g_localOffsets.pointer()
          , g_remoteOffsets.pointer()
          , sizeof(gaspi_offset_t) );

  size_t offset = 0;
  for(int i = 0; i < static_cast<int>(_groupSize); i++) {
    // Apparently, GASPI does not send a notification when
    // gaspi_write_notify() is called with size zero,
    // so we have to check this case manually!
    if(sendSizes[i] == 0) {
      notify(i, _groupRank);
      continue;
    }
    writeNotify(&static_cast<const char*>(g_send)[offset],
                i,
                g_remoteOffsets[i],
                sendSizes[i]*sizeMultiplicator,
                _groupRank);
    offset += sendSizes[i]*sizeMultiplicator;
  }
  waitNotificationsIgnoreStatus(0, _groupSize, _groupSize);

  barrier();

  waitQueue();
}
} // namespace GaspiLS

