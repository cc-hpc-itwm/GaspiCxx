#include <iostream>
#include <memory>
#include <vector>

#include <GaspiCxx/Runtime.hpp>
#include <GaspiCxx/Context.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/group/Rank.hpp>
#include <GaspiCxx/segment/MemoryManager.hpp>
#include <GaspiCxx/segment/NotificationManager.hpp>
#include <GaspiCxx/segment/Segment.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/utility/ScopedAllocation.hpp>

#include <chrono>

int
main
  ( int /*argc*/
  , char *[] /*argv*/) try {

  using namespace std;
  using namespace std::chrono;

  double aggregate_init_time = 0.0;
  double aggregate_wait_time = 0.0;

  auto & context = gaspi::getRuntime();

  gaspi::segment::Segment segment1(1024*1024);
  gaspi::segment::Segment segment2(1024*1024);

  std::size_t size(1);

  gaspi::singlesided::write::SourceBuffer lb1(segment1,size);
  gaspi::singlesided::write::SourceBuffer rb1(segment1,size);
  gaspi::singlesided::write::SourceBuffer lb2(segment2,size);
  gaspi::singlesided::write::SourceBuffer rb2(segment2,size);

  gaspi::singlesided::write::TargetBuffer lh1(segment1,size);
  gaspi::singlesided::write::TargetBuffer rh1(segment1,size);
  gaspi::singlesided::write::TargetBuffer lh2(segment2,size);
  gaspi::singlesided::write::TargetBuffer rh2(segment2,size);

  gaspi::group::Rank rightNeighbour
    ( ( context.rank()
      + context.size()
      + 1 ) % context.size() );

  gaspi::group::Rank leftNeighbour
    ( ( context.rank()
      + context.size()
      - 1 ) % context.size() );

  int lb1_tag(1);
  int rb1_tag(2);
  int lb2_tag(3);
  int rb2_tag(4);

  gaspi::singlesided::Endpoint::ConnectHandle
    lh1_connect_handle
      ( lh1.connectToRemoteSource
        ( context
        , leftNeighbour
        , rb1_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    rh1_connect_handle
      ( rh1.connectToRemoteSource
        ( context
        , rightNeighbour
        , lb1_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    lh2_connect_handle
      ( lh2.connectToRemoteSource
        ( context
        , leftNeighbour
        , rb2_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    rh2_connect_handle
      ( rh2.connectToRemoteSource
        ( context
        , rightNeighbour
        , lb2_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    lb1_connect_handle
      ( lb1.connectToRemoteTarget
        ( context
        , leftNeighbour
        , lb1_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    rb1_connect_handle
      ( rb1.connectToRemoteTarget
        ( context
        , rightNeighbour
        , rb1_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    lb2_connect_handle
      ( lb2.connectToRemoteTarget
        ( context
        , leftNeighbour
        , lb2_tag
        )
      );

  gaspi::singlesided::Endpoint::ConnectHandle
    rb2_connect_handle
      ( rb2.connectToRemoteTarget
        ( context
          , rightNeighbour
          , rb2_tag
        )
      );

  lh1_connect_handle.waitForCompletion();
  rh1_connect_handle.waitForCompletion();
  lh2_connect_handle.waitForCompletion();
  rh2_connect_handle.waitForCompletion();

  lb1_connect_handle.waitForCompletion();
  rb1_connect_handle.waitForCompletion();
  lb2_connect_handle.waitForCompletion();
  rb2_connect_handle.waitForCompletion();

  for(int it(0)
     ;    it<1000
     ;  ++it ) {

    if(it%2==0) {
      high_resolution_clock::time_point t1 = high_resolution_clock::now();
      lb1.initTransfer(context);
      rb1.initTransfer(context);
      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> duration1 = duration_cast<duration<double>>(t2 - t1);
      aggregate_init_time += duration1.count();

      high_resolution_clock::time_point t3 = high_resolution_clock::now();
      lh1.waitForCompletion();
      rh1.waitForCompletion();
      high_resolution_clock::time_point t4 = high_resolution_clock::now();
      duration<double> duration2 = duration_cast<duration<double>>(t4 - t3);
      aggregate_wait_time += duration2.count();
    }
    else {
      high_resolution_clock::time_point t1 = high_resolution_clock::now();
      lb2.initTransfer(context);
      rb2.initTransfer(context);
      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> duration1 = duration_cast<duration<double>>(t2 - t1);
      aggregate_init_time += duration1.count();

      high_resolution_clock::time_point t3 = high_resolution_clock::now();
      lh2.waitForCompletion();
      rh2.waitForCompletion();
      high_resolution_clock::time_point t4 = high_resolution_clock::now();
      duration<double> duration2 = duration_cast<duration<double>>(t4 - t3);
      aggregate_wait_time += duration2.count();
    }
  }

  std::cout << "Aggregate GaspiCxx init time = " << aggregate_init_time * 1000 << " ms" << std::endl;
  aggregate_init_time = 0.;
  std::cout << "Aggregate GaspiCxx wait time = " << aggregate_wait_time * 1000 << " ms" << std::endl;
  aggregate_wait_time = 0.;

  context.flush();
  context.barrier();


  gaspi_segment_id_t        lb1_SegmId(segment1.id());
  gaspi_offset_t            lb1_Offset(1*size);

  gaspi_rank_t              rh1_RankId(leftNeighbour.get());
  gaspi_segment_id_t        rh1_SegmId(segment1.id());
  gaspi_offset_t            rh1_Offset(3*size);
  gaspi_notification_id_t   rh1_Notify(1);

  gaspi_segment_id_t        rb1_SegmId(segment1.id());
  gaspi_offset_t            rb1_Offset(2*size);

  gaspi_rank_t              lh1_RankId(rightNeighbour.get());
  gaspi_segment_id_t        lh1_SegmId(segment1.id());
  gaspi_offset_t            lh1_Offset(0*size);
  gaspi_notification_id_t   lh1_Notify(0);

  gaspi_segment_id_t        lb2_SegmId(segment2.id());
  gaspi_offset_t            lb2_Offset(1*size);

  gaspi_rank_t              rh2_RankId(leftNeighbour.get());
  gaspi_segment_id_t        rh2_SegmId(segment2.id());
  gaspi_offset_t            rh2_Offset(3*size);
  gaspi_notification_id_t   rh2_Notify(1);

  gaspi_segment_id_t        rb2_SegmId(segment2.id());
  gaspi_offset_t            rb2_Offset(2*size);

  gaspi_rank_t              lh2_RankId(rightNeighbour.get());
  gaspi_segment_id_t        lh2_SegmId(segment2.id());
  gaspi_offset_t            lh2_Offset(0*size);
  gaspi_notification_id_t   lh2_Notify(0);

  for(int it(0)
       ;    it<1000
       ;  ++it ) {

    if(it%2==0) {
      high_resolution_clock::time_point t1 = high_resolution_clock::now();

      //      lb1.initTransfer(context);
      {
        gaspi_return_t ret(GASPI_ERROR);

        while( ( ret =  gaspi_write_notify
            ( lb1_SegmId
            , lb1_Offset
            , rh1_RankId
            , rh1_SegmId
            , rh1_Offset
            , size
            , rh1_Notify
            , 1
            , 0
            , GASPI_BLOCK) ) == GASPI_QUEUE_FULL ) {
          gaspi_wait(0,GASPI_BLOCK);
        }
      }

//      rb1.initTransfer(context);

      {
        gaspi_return_t ret(GASPI_ERROR);

        while( ( ret =  gaspi_write_notify
            ( rb1_SegmId
            , rb1_Offset
            , lh1_RankId
            , lh1_SegmId
            , lh1_Offset
            , size
            , lh1_Notify
            , 1
            , 0
            , GASPI_BLOCK) ) == GASPI_QUEUE_FULL ) {
          gaspi_wait(0,GASPI_BLOCK);
        }
      }

      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> duration1 = duration_cast<duration<double>>(t2 - t1);
      aggregate_init_time += duration1.count();

      high_resolution_clock::time_point t3 = high_resolution_clock::now();
//      lh1.waitForCompletion();
      {
        gaspi_notification_id_t id;
        gaspi_notification_t    value;

        gaspi_notify_waitsome
             ( lh1_SegmId
             , lh1_Notify
             , 1
             , &id
             , GASPI_BLOCK );

        gaspi_notify_reset
             ( lh1_SegmId
             , id
             , &value);
      }

//      rh1.waitForCompletion();
      {
        gaspi_notification_id_t id;
        gaspi_notification_t    value;

        gaspi_notify_waitsome
             ( rh1_SegmId
             , rh1_Notify
             , 1
             , &id
             , GASPI_BLOCK );

        gaspi_notify_reset
             ( rh1_SegmId
             , id
             , &value);
      }

      high_resolution_clock::time_point t4 = high_resolution_clock::now();
      duration<double> duration2 = duration_cast<duration<double>>(t4 - t3);
      aggregate_wait_time += duration2.count();

    }
    else {

      high_resolution_clock::time_point t1 = high_resolution_clock::now();
      //      lb2.initTransfer(context);
      {
        gaspi_return_t ret(GASPI_ERROR);

        while( ( ret =  gaspi_write_notify
            ( lb2_SegmId
            , lb2_Offset
            , rh2_RankId
            , rh2_SegmId
            , rh2_Offset
            , size
            , rh2_Notify
            , 1
            , 0
            , GASPI_BLOCK) ) == GASPI_QUEUE_FULL ) {
          gaspi_wait(0,GASPI_BLOCK);
        }
      }

  //      rb2.initTransfer(context);

      {
        gaspi_return_t ret(GASPI_ERROR);

        while( ( ret =  gaspi_write_notify
            ( rb2_SegmId
            , rb2_Offset
            , lh2_RankId
            , lh2_SegmId
            , lh2_Offset
            , size
            , lh2_Notify
            , 1
            , 0
            , GASPI_BLOCK) ) == GASPI_QUEUE_FULL ) {
          gaspi_wait(0,GASPI_BLOCK);
        }
      }

      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> duration1 = duration_cast<duration<double>>(t2 - t1);
      aggregate_init_time += duration1.count();

      high_resolution_clock::time_point t3 = high_resolution_clock::now();
  //      lh2.waitForCompletion();
      {
        gaspi_notification_id_t id;
        gaspi_notification_t    value;

        gaspi_notify_waitsome
             ( lh2_SegmId
             , lh2_Notify
             , 1
             , &id
             , GASPI_BLOCK );

        gaspi_notify_reset
             ( lh2_SegmId
             , id
             , &value);
      }

  //      rh2.waitForCompletion();
      {
        gaspi_notification_id_t id;
        gaspi_notification_t    value;

        gaspi_notify_waitsome
             ( rh2_SegmId
             , rh2_Notify
             , 1
             , &id
             , GASPI_BLOCK );

        gaspi_notify_reset
             ( rh2_SegmId
             , id
             , &value);
      }

      high_resolution_clock::time_point t4 = high_resolution_clock::now();
      duration<double> duration2 = duration_cast<duration<double>>(t4 - t3);
      aggregate_wait_time += duration2.count();

    }
  }

  context.flush();
  context.barrier();

  std::cout << "Aggregate plain init time = " << aggregate_init_time * 1000 << " ms" << std::endl;
  std::cout << "Aggregate plain wait time = " << aggregate_wait_time * 1000 << " ms" << std::endl;

  return EXIT_SUCCESS;
} catch(...) {
  return EXIT_FAILURE;
}
