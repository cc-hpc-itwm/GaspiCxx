#pragma once

#include <GaspiCxx/collectives/non_blocking/Operator.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
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
        : AllreduceCommon(segment, context, number_elements, reduction_op),
          source_buffers(),
          target_buffers()
        {
          auto const number_ranks = group.size().get();

          auto const left_neighbor = (group.rank() - 1 + number_ranks) % number_ranks;
          auto const right_neighbor = (group.rank() + 1) % number_ranks;

          auto const size_buffers = sizeof(T) * number_elements;
          auto const number_elements_buffer = number_elements / number_ranks + (number_elements % number_ranks != 0);
          auto const size_buffer = sizeof(T) * number_elements_buffer;

          std::cout << "rank=" << group.rank().get() << " left=" << left_neighbor.get() <<
                   " right=" << right_neighbor.get() << " size_buffer=" << size_buffer <<
                   " total_size=" << size_buffers << std::endl;
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
            gaspi::singlesided::write::SourceBuffer::Tag source_tag = i;
            gaspi::singlesided::write::TargetBuffer::Tag target_tag = i;
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
        {
          auto total_size = number_elements * sizeof(T);
          auto total_copied_size = 0UL;
          auto current_input_ptr = static_cast<char*>(inputs);

          for (auto& buffer : source_buffers)
          {
            auto size_to_copy = buffer->description().size();
            if (total_copied_size + size_to_copy > total_size)
            {
              size_to_copy = total_size - total_copied_size;
            }
            std::cout << "rank=" << group.rank().get() << " size=" << size_to_copy <<
                   " total_size=" << total_size<< std::endl;

            std::memcpy(buffer->address(), current_input_ptr, size_to_copy);
            current_input_ptr = current_input_ptr + size_to_copy;
            total_copied_size += size_to_copy;

            if (total_copied_size >= total_size)
            {
              break;
            }
          }
        }

        void copy_out(void* outputs) override
        {
          auto total_size = number_elements * sizeof(T);
          auto total_copied_size = 0UL;
          auto current_output_ptr = static_cast<char*>(outputs);

          for (auto& buffer : target_buffers)
          {
            auto size_to_copy = buffer->description().size();
            if (total_copied_size + size_to_copy > total_size)
            {
              size_to_copy = total_size - total_copied_size;
            }

            std::memcpy(current_output_ptr, buffer->address(), size_to_copy);
            current_output_ptr = current_output_ptr + size_to_copy;
            total_copied_size += size_to_copy;

            if (total_copied_size >= total_size)
            {
              break;
            }
          }
        }
    };
  }
}