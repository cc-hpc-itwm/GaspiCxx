#pragma once

#include <GaspiCxx/LocalBuffer.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    template<typename T>
    class AllreduceLowLevel<T, AllreduceAlgorithm::RING> : public AllreduceCommon
    {
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;

      public:
        using AllreduceCommon::AllreduceCommon;

        AllreduceLowLevel(gaspi::segment::Segment& segment,
                          gaspi::group::Group const& group,
                          std::size_t number_elements,
                          ReductionOp reduction_op);

      private:
        enum class RingStage
        {
          REDUCE,
          GATHER
        };

        std::vector<std::unique_ptr<SourceBuffer>>
          source_buffers;
        std::vector<std::unique_ptr<TargetBuffer>>
          target_buffers;
        std::vector<gaspi::singlesided::Endpoint::ConnectHandle> source_handles;
        std::vector<gaspi::singlesided::Endpoint::ConnectHandle> target_handles;
      
        std::size_t current_step;
        std::size_t steps_per_stage;
        std::size_t send_buffer_index;
        std::size_t receive_buffer_index;

        void waitForSetupImpl() override;
        void copyInImpl(void*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        // algorithm-specific methods
        auto algorithm_get_current_stage();
        bool algorithm_is_finished();
        void algorithm_reset_state();
        void apply_reduce_op(SourceBuffer& source_comm, TargetBuffer& target_comm);
        void copy_to_source(SourceBuffer& source_comm, TargetBuffer& target_comm);
    };

    template<typename T>
    AllreduceLowLevel<T, AllreduceAlgorithm::RING>::AllreduceLowLevel(
                      gaspi::segment::Segment& segment,
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      ReductionOp reduction_op)
    : AllreduceCommon(segment, group, number_elements, reduction_op),
      source_buffers(),
      target_buffers(),
      steps_per_stage(group.size()-1)
    {
      auto const number_ranks = group.size();

      // neighbor ranks within the group
      auto const left_neighbor = (group.rank() - 1 + number_ranks) % number_ranks;
      auto const right_neighbor = (group.rank() + 1) % number_ranks;

      auto const number_elements_buffer = number_elements / number_ranks +
                                          (number_elements % number_ranks != 0 ? 1 : 0);
      auto const size_buffer = sizeof(T) * number_elements_buffer;

      // create buffers
      for (auto i = 0UL; i < number_ranks; ++i)
      {
        source_buffers.push_back(
          std::make_unique<SourceBuffer>(segment, size_buffer));
        target_buffers.push_back(
          std::make_unique<TargetBuffer>(segment, size_buffer));
      }

      // connect buffers
      for (auto i = 0UL; i < number_ranks; ++i)
      {
        SourceBuffer::Tag source_tag = i;
        TargetBuffer::Tag target_tag = i;
        source_handles.push_back(
          source_buffers[i]->connectToRemoteTarget(context, left_neighbor, source_tag));
        target_handles.push_back(
          target_buffers[i]->connectToRemoteSource(context, right_neighbor, target_tag));
      }
      algorithm_reset_state();
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::waitForSetupImpl()
    {
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

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::startImpl()
    {
      algorithm_reset_state();
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::triggerProgressImpl()
    {
      auto const number_ranks = group.size();

      send_buffer_index = (send_buffer_index + 1) % number_ranks;
      receive_buffer_index = (receive_buffer_index + 1) % number_ranks;

      source_buffers[send_buffer_index]->initTransfer(context);
      target_buffers[receive_buffer_index]->waitForCompletion();

      switch(algorithm_get_current_stage())
      {
        case RingStage::REDUCE:
        {
          apply_reduce_op(*source_buffers[receive_buffer_index], *target_buffers[receive_buffer_index]);
          break;
        }
        case RingStage::GATHER:
        {
          copy_to_source(*source_buffers[receive_buffer_index], *target_buffers[receive_buffer_index]);
          break;
        }
      }

      current_step++;
      return algorithm_is_finished();
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::copyInImpl(void* inputs)
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

        std::memcpy(buffer->address(), current_input_ptr, size_to_copy);
        current_input_ptr = current_input_ptr + size_to_copy;
        total_copied_size += size_to_copy;

        if (total_copied_size >= total_size)
        {
          break;
        }
      }
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::copyOutImpl(void* outputs)
    {
      auto total_size = number_elements * sizeof(T);
      auto total_copied_size = 0UL;
      auto current_output_ptr = static_cast<char*>(outputs);

      for (auto& buffer : source_buffers)
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

    template<typename T>
    auto AllreduceLowLevel<T, AllreduceAlgorithm::RING>::algorithm_get_current_stage()
    {
      return (current_step / steps_per_stage == 0)? RingStage::REDUCE : RingStage::GATHER;
    }

    template<typename T>
    bool AllreduceLowLevel<T, AllreduceAlgorithm::RING>::algorithm_is_finished()
    {
      // check whether the algorithm reached the final step of the second stage
      return (algorithm_get_current_stage() == RingStage::GATHER &&
              current_step == 2 * steps_per_stage);
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::algorithm_reset_state()
    {
      auto const number_ranks = group.size();

      current_step = 0;
      receive_buffer_index = group.rank().get();
      send_buffer_index = (receive_buffer_index - 1 + number_ranks) % number_ranks;
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::apply_reduce_op(
              SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
      auto num_elems = source_comm.description().size()/sizeof(T);
      gaspi::LocalBuffer<T> source_local(static_cast<T*>(source_comm.address()), num_elems);
      gaspi::LocalBuffer<T> target_local(static_cast<T*>(target_comm.address()), num_elems);

      std::transform(source_local.begin(), source_local.end(), target_local.begin(),
                     source_local.begin(), std::plus<T>());
    }

    template<typename T>
    void AllreduceLowLevel<T, AllreduceAlgorithm::RING>::copy_to_source(
              SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
      auto num_elems = source_comm.description().size()/sizeof(T);
      gaspi::LocalBuffer<T> source_local(static_cast<T*>(source_comm.address()), num_elems);
      gaspi::LocalBuffer<T> target_local(static_cast<T*>(target_comm.address()), num_elems);

      std::copy(target_local.begin(), target_local.end(),
                source_local.begin());
    }
  }
}