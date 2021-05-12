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
 * Environment.hpp
 *
 */

#ifndef GASPIENVIRONMENT_HPP
#define GASPIENVIRONMENT_HPP

#include <segment/Segment.hpp>
#include <cstring>
extern "C" {
#include <GASPI.h>
}

namespace gaspi
{

  //! Maximum size of a supported scalar type
  const size_t MAX_SCALAR_TYPE_SIZE = 16;
  const size_t MAX_SCALAR_TYPE_NUM  = 10;

  //! This class provied an interface for GASPI communication.
  //! It basically combines a GASPI group and a GASPI segment.
  //! \brief   Abstraction of a GASPI segment and group
  //! \warning Each GASPI interface must have its own GASPI segment
  //!          and GASPI queue that are not used by the user
  //!          application in any other way!
  //!          Therefore, interfaces do not provide a copy constructor
  //!          or an assignment operator.
  class Environment
  {
  private:
    //! The rank of "this" process
    gaspi_rank_t _rank;
    //! The group of ranks that constitute the interface
    gaspi_group_t _group;
    //! The size of the group
    gaspi_number_t _groupSize;
    //! The rank within the group. Ranges from 0 to groupSize - 1.
    gaspi_rank_t _groupRank;
    //! All ranks in the group. Also used as a mapping from group rank to rank.
    gaspi_rank_t *_ranksInGroup;
    //! Segment
    segment::Segment _segment;
    //! The GASPI queue used by this interface
    gaspi_queue_id_t _queue;
    //! Maximum number of elements in a GASPI queue
    gaspi_number_t _maxQueueSize;
    //! Buffer for allreduce operations in the GASPI segment
    void *_g_allreduceBuffer;
    //! Offset of the allreduce buffer on each rank
    gaspi_offset_t *_allreduceBufferRemoteOffsets;

    //! A notification that is used if the value is of no importanGaspiCxx
    static const gaspi_notification_t NOTIFICATION_DISCARD = 1;

    //! An interface cannot be copied.
    Environment
      (const Environment&) = delete;

    Environment&
    operator=(const Environment&) = delete;

    //! Perform an allgather operation using passive GASPI communication.
    //! \note   This method uses passive GASPI communication \e without
    //!         checking maximum message sizes etc. Therefore, it should only
    //!         be used to exchange small amounts of data. The main purpose of
    //!         this method is to exchange GASPI segment offsets, as they often
    //!         are not known a priori due to dynamic memory allocation on
    //!         the GASPI segment.
    //! \note   Blocking global collective!
    //! \throws GASPIError
    void allgatherPassive
      ( const void *g_send
      , void       *g_receive
      , size_t     size );

    void allgatherRemoteOffsets( const void *g_send,
                                 void       *g_receive,
                                 size_t     size);

    //! Local reduce operation
    //! \tparam  The type of the values and the result
    //! \returns the result of the reduce operation.
    template <class S>
    static S localReduce(S                 *values,
                         int               numValues,
                         gaspi_operation_t operation);

  public:

    using Rank = gaspi_rank_t;

    //! Construct a GASPI interface from a group and a segment.
    //! \note GASPI and the given segment must be initialized on
    //!       all ranks of the given group!
    Environment
      ( gaspi_group_t      group
      , gaspi_segment_id_t segmentID
      , gaspi_queue_id_t   queue);
    //! The destructor
    ~Environment();
    //! Get the rank of the calling process.
    //! \note Most GaspiCxx methods work with the <em>group rank</em>, i.e.,
    //!       the rank within the GASPI group used for the interface!
    gaspi_rank_t rank() const;
    //! Return the GASPI group of this interface.
    gaspi_group_t group() const;
    //! Return the GASPI queue of this interface.
    gaspi_queue_id_t queue() const;
    //! Return the size of the GASPI group of this interface.
    gaspi_number_t groupSize() const;
    //! Get the rank <em>within the group</em>.
    gaspi_rank_t groupRank() const;
    //! Return the segment
    segment::Segment &
    segment()  {
      return _segment;
    }

    //! Get the maximum queue size.
    gaspi_number_t maxQueueSize() const;
    //! This wrapper function for gaspi_write_notify() prescribes the
    //! segment and queue, uses <em>group ranks</em> and provides default values
    //! for the notification value and timeout for more convenient use.
    //! \brief   Perform a gaspi_write_notify() on the GASPI interface.
    //! \note    gaspi_wait() will be called if the queue is full,
    //!          but not after each write!
    //! \warning This method uses <c>void*</c> instead of \c gaspi_offset_t
    //!          to specify memory locations for convenience and correctness.
    //!          However, \c destination is \em not a valid pointer on the
    //!          remote rank! It is only used to compute the offset from
    //!          the beginning of the GASPI segment via the \c pointerToOffset()
    //!          method.
    //! \throws  GASPIError
    void writeNotify(const void              *g_send,
                     gaspi_rank_t            groupRank,
                     gaspi_offset_t          remoteOffset,
                     size_t                  size,
                     gaspi_notification_id_t notificationID,
                     gaspi_notification_t    notificationValue
                                              = NOTIFICATION_DISCARD,
                     gaspi_timeout_t         timeout = GASPI_BLOCK)
                     const;
    //! This wrapper function for gaspi_notify() prescribes the
    //! segment and queue, uses <em>group ranks</em> and provides default values
    //! for the notification value and timeout for more convenient use.
    //! \brief   Perform a gaspi_notify() on the GASPI interface.
    //! \note    gaspi_wait() will be called if the queue is full,
    //!          but not after each notification!
    //! \throws  GASPIError
    void notify(gaspi_rank_t            groupRank,
                gaspi_notification_id_t notificationID,
                gaspi_notification_t    notificationValue
                                          = NOTIFICATION_DISCARD,
                gaspi_timeout_t         timeout = GASPI_BLOCK)
                const;

    //! Wait for \c numWaits notifications (blocking) with ID's
    //! from \c startID to <c>startID+numIDs</c>.
    //! \brief Wait for multiple notifications, ignoring their values.
    //! \param startID  Start of the block of admissible notifications
    //! \param numIDs   Size of the block of admissible notifications
    //! \param numWaits Number of notifications to wait for.
    void waitNotificationsIgnoreStatus(int startID,
                                       int numIDs,
                                       int numWaits)
                                       const;
    //! Wait for all communication requests in the queue to complete.
    //! \throws GASPIError
    void waitQueue() const;

    //! Perform a (blocking) barrier operation on the interface's GASPI group.
    //! \note Blocking global collective!
    void barrier() const;
    //! \brief   Perform an allreduce operation on the interface's GASPI group.
    //! \returns the result of the allreduce operation.
    //! \note    The main purpose of this method is to provide an allreduce
    //!          operation that supports complex numbers, which is not the case
    //!          for the standard \c gaspi_allreduce.
    //!          However, \c gaspi_allreduce is called if the template
    //!          parameter is \c float or \c double.
    //!          A blocking allreduce operation will be performed for one
    //!          element on the interface's GASPI group.
    //! \tparam  S The type of the values and the result
    //! \note    Blocking global collective!
    template <class S>
    S allreduce(S                 localValue,
                gaspi_operation_t operation) const;
    template <class S>
    void allreduce(S * const localValues,
                   S * const globalValues,
                   int numValues,
                   gaspi_operation_t operation) const;
    //! Perform an allgather operation.
    //! \note   This method will automatically exchange the receive buffer
    //!         offsets using \c allgatherPassive(). It should therefore not be
    //!         used if the remote offsets are already known, or if they can be
    //!         reused for subsequent operations.
    //! \note   Blocking global collective!
    //! \throws GASPIError
    void allgather(const void *g_send,
                   void       *g_receive,
                   size_t     size);
    //! Perform an allgather operation with variable amounts of data.
    //! \param  sizeMultiplicator Multiply all sizes by this value, e.g.,
    //!                           by the size of the data type to be sent.
    //! \note   This method will automatically exchange the receive buffer
    //!         offsets using \c alltoallPassive(). It should therefore not be
    //!         used if the remote offsets are already known, or if they can be
    //!         reused for subsequent operations.
    //! \note   Blocking global collective!
    //! \throws GASPIError
    void allgatherv(const void   *g_send,
                    void         *g_receive,
                    const size_t *sizes,
                    size_t       sizeMultiplicator = 1);
    //! Perform an alltoall operation.
    //! \note   This method will automatically exchange the receive buffer
    //!         offsets using \c allgatherPassive(). It should therefore not be
    //!         used if the remote offsets are already known, or if they can be
    //!         reused for subsequent operations.
    //! \note   Blocking global collective!
    //! \throws GASPIError
    void alltoall(const void *g_send,
                  void       *g_receive,
                  size_t     size);
    //! Perform an alltoall operation with variable amounts of data.
    //! \param  sizeMultiplicator Multiply all sizes by this value, e.g.,
    //!                           by the size of the data type to be sent.
    //! \note   This method will automatically exchange the receive buffer
    //!         offsets using \c allgatherPassive(). It should therefore not be
    //!         used if the remote offsets are already known, or if they can be
    //!         reused for subsequent operations.
    //! \note   Blocking global collective!
    //! \throws GASPIError
    void alltoallv(const void   *g_send,
                   const size_t *sendSizes,
                   void         *g_receive,
                   const size_t *receiveSizes,
                   size_t       sizeMultiplicator = 1);
  };

  // Do not include template specializations in Doxygen.
  //! \cond
  template <>
  float Environment::allreduce(float             localValue,
                                  gaspi_operation_t operation)
                                  const;
  template <>
  double Environment::allreduce(double            localValue,
                                   gaspi_operation_t operation)
                                   const;
  template <>
  void Environment::allreduce(float * const localValues,
                                 float * const globalValues,
                                 int numValues,
                                 gaspi_operation_t operation)
                                 const;
  template <>
  void Environment::allreduce(double * const localValues,
                                 double * const globalValues,
                                 int numValues,
                                 gaspi_operation_t operation)
                                 const;

  //! \endcond

  //! This class implements the RAII idiom for memory
  //! allocations on GASPI segments.
  //! \brief Managed memory allocation on a GASPI segment
  template <class T>
  class GASPIScopedAllocation
  {
  private:
    Environment &_interface;
    T *_g_pointer;
    size_t _count;

  public:
    //! Allocate <c>count*sizeof(T)</c> bytes on the given interface.
    //! \brief Constructor
    GASPIScopedAllocation(Environment &interface, size_t count)
    : _interface(interface)
    , _g_pointer
        (segment::Allocator<T>
          (_interface.segment().allocator()).allocate(count))
    , _count(count)
    { }

    ~GASPIScopedAllocation() {
      segment::Allocator<T> alloc(_interface.segment().allocator());
//      _interface.segment().allocator().deallocate(_g_pointer,_byte_size);
      alloc.deallocate(_g_pointer,_count);
    }
    //! \brief   Get a pointer to the allocated memory.
    //! \warning Do not take ownership of the pointer and make sure
    //!          it is not used outside of the current scope!
    T* pointer() const { return _g_pointer; }
    //! Access the allocated memory.
    T& operator[](int index) const { return _g_pointer[index]; }
  };

  // General implementation of allreduce.
  // Note that there are specializations for float and double
  // that use the standard gaspi_allreduce function instead.
  template <class S>
  S Environment::allreduce(S                 localValue,
                              gaspi_operation_t operation)
                              const
  {
    S result;

    allreduce( &localValue
             , &result
             , 1
             , operation );
    
    return result;
  }

  template <class S>
  void Environment::allreduce(S * const localValues,
                                 S * const globalValues,
                                 int numValues,
                                 gaspi_operation_t operation) const
  {

    if( numValues * sizeof(S) >
        MAX_SCALAR_TYPE_SIZE
      * MAX_SCALAR_TYPE_NUM ) {
      if(numValues == 1 ) {
        throw std::runtime_error("Communication buffer too small");
      }
      else {
        for(int i = 0; i < numValues; i++) {
          globalValues[i] = allreduce( localValues[i],
                                       operation);
        }
      }
    }

    S *g_values = static_cast<S*>(_g_allreduceBuffer);

    std::memcpy( &g_values[_groupRank*numValues]
               , localValues
               , numValues * sizeof(S) );

    for(int i = 0; i < static_cast<int>(_groupSize); i++) {
      if(i == _groupRank) continue;
      writeNotify( &g_values[_groupRank*numValues]
                 , i
                 , _allreduceBufferRemoteOffsets[i]
                 + _groupRank * numValues * sizeof(S)
                 , numValues * sizeof(S)
                 , _groupRank );
    }
    waitNotificationsIgnoreStatus(0, _groupSize, _groupSize - 1);

    S tmp_values[_groupSize];

    for(int iValue(0)
       ;    iValue < numValues
       ;  ++iValue ) {
      for(int iRank(0)
         ;    iRank < _groupSize
         ;  ++iRank) {
        tmp_values[iRank] = g_values[iRank * numValues + iValue];
      }

      globalValues[iValue] = localReduce(tmp_values, _groupSize, operation);
    }

    barrier();

    waitQueue();
  }


  template <class S>
  S Environment::localReduce(S                 *values,
                                int               numValues,
                                gaspi_operation_t operation)
  {
    S result = 0.0;

    switch(operation) {
    case GASPI_OP_MIN:
      result = values[0];
      for(int i = 1; i < numValues; i++) {
        if(std::abs(values[i]) < std::abs(result)) {
          result = values[i];
        }
      }
      break;
    case GASPI_OP_MAX:
      result = values[0];
      for(int i = 1; i < numValues; i++) {
        if(std::abs(values[i]) > std::abs(result)) {
          result = values[i];
        }
      }
      break;
    case GASPI_OP_SUM:
      for(int i = 0; i < numValues; i++) {
        result += values[i];
      }
      break;
    }

    return result;
  }
} // namespace GaspiCxx

#endif // GASPIINTERFACE_HPP

