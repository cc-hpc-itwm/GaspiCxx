set(SOURCE_FILES
    CommunicationContext.cpp
    SingleQueueContext.cpp
    RoundRobinQueuesContext.cpp
    Runtime.cpp
    RuntimeConfiguration.cpp
    collectives/Allgather.cpp
    collectives/Alltoall.cpp
    group/Group.cpp
    group/Rank.cpp
    passive/Passive.cpp
    segment/Segment.cpp
    segment/SegmentManager.cpp
    segment/SegmentResource.cpp
    segment/SingleSegmentPool.cpp
    segment/DynamicSegmentPool.cpp
    segment/MemoryManager.cpp
    segment/NotificationManager.cpp
    singlesided/Buffer.cpp
    singlesided/BufferDescription.cpp
    singlesided/Endpoint.cpp
    singlesided/Queue.cpp
    singlesided/write/SourceBuffer.cpp
    singlesided/write/TargetBuffer.cpp
    utility/Filesystem.cpp
    utility/LockGuard.cpp
    utility/serialization.cpp
    collectives/Barrier.cpp
    collectives/non_blocking/collectives_lowlevel/AllreduceCommon.cpp
    collectives/non_blocking/collectives_lowlevel/BroadcastCommon.cpp
    collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.cpp
    progress_engine/RoundRobinDedicatedThread.cpp)

add_library(GaspiCxx ${SOURCE_FILES})
target_include_directories(GaspiCxx 
                           PUBLIC $<BUILD_INTERFACE:${INCLUDE_DIR}>
                                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GaspiCxx
                      Threads::Threads
                      optimized GPI2::GPI2
                      debug GPI2::GPI2dbg)

install(TARGETS GaspiCxx
        EXPORT GaspiCxxConfig
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY
          ${INCLUDE_DIR}/GaspiCxx
        DESTINATION
          ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT GaspiCxxConfig
        FILE
          GaspiCxxConfig.cmake
        NAMESPACE
          GaspiCxx::
        DESTINATION
          ${CMAKE_INSTALL_LIBDIR}/cmake)

