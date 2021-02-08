#include <GaspiCxx/collectives/Barrier.hpp>

#include <cmath>

/*
Dissemination Barrier

Debra Hensgen, Raphael Finkel, and Udi Manber
`Two Algorithms for Barrier Synchronization`
*/

namespace gaspi {
  namespace collectives {
    namespace blocking {

      Barrier::Barrier(gaspi::segment::Segment& segment,
                       gaspi::group::Group const& group)
      : group(group),
        context(group)
      {
        auto const number_ranks = group.size();
        auto const size_buffer = 0UL;

        // create buffers
        for (auto i = 0UL; i < std::ceil(log2(number_ranks)); ++i)
        {
          source_buffers.push_back(
            std::make_unique<SourceBuffer>(segment, size_buffer));
          target_buffers.push_back(
            std::make_unique<TargetBuffer>(segment, size_buffer));
        }

        // connect buffers
        std::vector<gaspi::singlesided::Endpoint::ConnectHandle> source_handles;
        std::vector<gaspi::singlesided::Endpoint::ConnectHandle> target_handles;
        for (auto i = 0UL; i < std::ceil(log2(number_ranks)); ++i)
        {
          auto const right_neighbor = (group.rank() + static_cast<int>(std::exp2(i))) % number_ranks;
          auto const left_neighbor = (group.rank() - static_cast<int>(std::exp2(i)) + number_ranks) % number_ranks;
          SourceBuffer::Tag source_tag = right_neighbor.get();
          TargetBuffer::Tag target_tag = group.rank().get();

          source_handles.push_back(
            source_buffers[i]->connectToRemoteTarget(context, right_neighbor, source_tag));
          target_handles.push_back(
            target_buffers[i]->connectToRemoteSource(context, left_neighbor, target_tag));
        }

        // wait for connections
        for (auto& handle : source_handles)
        {
          handle.waitForCompletion();
        }
        for (auto& handle : target_handles)
        {
          handle.waitForCompletion();
        }
      }

      void Barrier::execute()
      {
        for (auto i = 0UL; i < std::ceil(log2(group.size())); ++i)
        {
          source_buffers[i]->initTransfer(context);
          target_buffers[i]->waitForCompletion();
          target_buffers[i]->ackTransfer(context);
          source_buffers[i]->waitForTransferAck();
        }
      }

    }
  }
}

