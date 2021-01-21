#pragma once

#include <GaspiCxx/collectives/non_blocking/Operator.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/collectives/non_blocking/AllreduceCommon.hpp>

#include <atomic>
#include <stdexcept>
#include <memory>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    enum class AllreduceAlgorithm
    {
      RING,
    };

    template<typename T, AllreduceAlgorithm Algorithm>
    class Allreduce : public AllreduceCommon { };

    template<typename T>
    class Allreduce<T, AllreduceAlgorithm::RING> : public AllreduceCommon
    {
      public:
        using AllreduceCommon::AllreduceCommon;

        Allreduce(gaspi::segment::Segment& segment,
                  gaspi::Context& context,
                  // gaspi::group::Group const& group,
                  std::size_t number_elements,
                  ReductionOp reduction_op)
        : AllreduceCommon(segment, group, number_elements, reduction_op),
          source_buffers(),
          target_buffers()
        {
          auto const number_ranks = group.size().get();
          auto const left_neighbor = (group.rank() - 1 + number_ranks) % number_ranks;
          auto const right_neighbor = (group.rank() + 1) % number_ranks;

          auto const size_buffers = sizeof(T) * number_elements;
          auto const size_buffer = (size_buffers + number_ranks) / number_ranks;

          // create buffers
          for (auto i = 0; i < number_ranks; ++i)
          {
            source_buffers.push_back(
              std::make_unique<gaspi::singlesided::write::SourceBuffer>(segment, size_buffer));
            target_buffers.push_back(
              std::make_unique<gaspi::singlesided::write::TargetBuffer>(segment, size_buffer));
          }

          // connect buffers
          std::vector<gaspi::singlesided::Endpoint::ConnectHandle> source_handles;
          std::vector<gaspi::singlesided::Endpoint::ConnectHandle> target_handles;
          for (auto i = 0; i < number_ranks; ++i)
          {
            auto const source_tag = i;
            auto const target_tag = i + number_ranks;
            source_handles.push_back(
              source_buffers[i]->connectToRemoteTarget(context, right_neighbor, source_tag));
            target_handles.push_back(
              target_buffers[i]->connectToRemoteSource(context, left_neighbor, target_tag));
          }

          // wait for connections
          for (auto i = 0; i < number_ranks; ++i)
          {
            source_handles[i].waitForCompletion();
            target_handles[i].waitForCompletion();
          }
        }

        template<typename T>
        void start(std::vector<T> const& inputs) override
        {
          AllreduceCommon::start(inputs);
        }

        void start(void* inputs) override
        {
          AllreduceCommon::start(inputs);
        }

        void trigger_communication_step() override
        {
          // FIXME: Implement
          state = Operator::State::FINISHED;
        }

      private:
        std::vector<std::unique_ptr<gaspi::singlesided::write::SourceBuffer>>
          source_buffers;
        std::vector<std::unique_ptr<gaspi::singlesided::write::TargetBuffer>>
          target_buffers;

        void copy_in(void* inputs) override
        { }
    };
  }
}