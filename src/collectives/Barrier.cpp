#include <GaspiCxx/collectives/Barrier.hpp>
#include <GaspiCxx/Runtime.hpp>

#include <cmath>

/*
Dissemination Barrier

Debra Hensgen, Raphael Finkel, and Udi Manber
`Two Algorithms for Barrier Synchronization`
*/

namespace gaspi {
  namespace collectives {
    namespace blocking {

      Barrier::Barrier(gaspi::group::Group const& group)
      : comm_context(getRuntime().getDefaultCommunicationContext()),
        number_steps(std::ceil(std::log2(group.size())))
      {
        auto const rank = group.rank();
        auto const number_ranks = static_cast<int>(group.size());
        auto const zero_size = 0UL;

        // create buffers
        for (auto i = 0UL; i < number_steps; ++i)
        {
          source_buffers.push_back(
            std::make_unique<SourceBuffer>(zero_size));
          target_buffers.push_back(
            std::make_unique<TargetBuffer>(zero_size));
        }

        // connect buffers
        std::vector<gaspi::singlesided::Endpoint::ConnectHandle> source_handles;
        std::vector<gaspi::singlesided::Endpoint::ConnectHandle> target_handles;
        for (auto i = 0UL; i < number_steps; ++i)
        {
          auto const target = (rank + static_cast<int>(std::exp2(i))) % number_ranks;
          auto const source = (rank + number_ranks - static_cast<int>(std::exp2(i))) % number_ranks;
          SourceBuffer::Tag source_buffer_tag = target.get();
          TargetBuffer::Tag target_buffer_tag = rank.get();

          source_handles.push_back(
            source_buffers[i]->connectToRemoteTarget(group, target, source_buffer_tag));
          target_handles.push_back(
            target_buffers[i]->connectToRemoteSource(group, source, target_buffer_tag));
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
        for (auto i = 0UL; i < number_steps; ++i)
        {
          source_buffers[i]->initTransfer(comm_context);
          target_buffers[i]->waitForCompletion();
          target_buffers[i]->ackTransfer(comm_context);
          source_buffers[i]->waitForTransferAck();
        }
      }
    }
  }
}
