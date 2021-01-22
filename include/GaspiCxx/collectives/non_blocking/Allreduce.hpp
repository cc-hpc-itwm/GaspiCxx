#pragma once

#include <GaspiCxx/collectives/non_blocking/Operator.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/LocalBuffer.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/AllreduceCommon.hpp>

#include <atomic>
#include <memory>
#include <stdexcept>
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
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;

      public:
        using AllreduceCommon::AllreduceCommon;

        Allreduce(gaspi::segment::Segment& segment,
                  gaspi::Context& context,
                  std::size_t number_elements,
                  ReductionOp reduction_op);

        void start(void* inputs) override;
        void trigger_communication_step() override;

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
        std::size_t current_step;
        RingStage stage;
        std::size_t steps_per_stage;

        bool step_reduce_stage(std::size_t step, SourceBuffer& source, TargetBuffer& target);
        bool step_gather_stage(std::size_t step, SourceBuffer& source, TargetBuffer& target);
        void update_current_step();
        void update_current_stage();
        bool algorithm_final_state();
        void algorithm_reset_state();
        void apply_reduce_op(SourceBuffer& source_comm, TargetBuffer& target_comm);
        void copy_to_source(SourceBuffer& source_comm, TargetBuffer& target_comm);

        void copy_in(void*) override;
        void copy_out(void*) override;
    };


    template<typename T>
    Allreduce<T, AllreduceAlgorithm::RING>::Allreduce(gaspi::segment::Segment& segment,
                                                      gaspi::Context& context,
                                                      // gaspi::group::Group const& group,
                                                      std::size_t number_elements,
                                                      ReductionOp reduction_op)
    : AllreduceCommon(segment, context, number_elements, reduction_op),
      source_buffers(),
      target_buffers(),
      current_step(0),
      stage(RingStage::REDUCE),
      steps_per_stage(group.size().get())
    {
      auto const number_ranks = group.size().get();

      auto const left_neighbor = (group.rank() - 1 + number_ranks) % number_ranks;
      auto const right_neighbor = (group.rank() + 1) % number_ranks;

      auto const number_elements_buffer = number_elements / number_ranks + (number_elements % number_ranks != 0);
      auto const size_buffer = sizeof(T) * number_elements_buffer;

      // create buffers
      for (auto i = 0; i < number_ranks; ++i)
      {
        source_buffers.push_back(
          std::make_unique<SourceBuffer>(segment, size_buffer));
        target_buffers.push_back(
          std::make_unique<TargetBuffer>(segment, size_buffer));
      }

      // connect buffers
      std::vector<gaspi::singlesided::Endpoint::ConnectHandle> source_handles;
      std::vector<gaspi::singlesided::Endpoint::ConnectHandle> target_handles;
      for (auto i = 0; i < number_ranks; ++i)
      {
        SourceBuffer::Tag source_tag = i;
        TargetBuffer::Tag target_tag = i;
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
    void Allreduce<T, AllreduceAlgorithm::RING>::start(void* inputs)
    {
      AllreduceCommon::start(inputs);
    }

    template<typename T>
    void Allreduce<T, AllreduceAlgorithm::RING>::trigger_communication_step()
    {
      if (is_finished())
      {
        return;
      }

      auto const number_ranks = group.size().get();
      auto const rank = group.rank().get();

      switch(stage)
      {
        case RingStage::REDUCE:
        {
          auto const send_buffer_index = (rank - current_step + number_ranks) % number_ranks;
          auto const receive_buffer_index = (rank - current_step + number_ranks - 1) % number_ranks;

          source_buffers[send_buffer_index]->initTransfer(context);
          target_buffers[receive_buffer_index]->waitForCompletion();

          apply_reduce_op(*source_buffers[receive_buffer_index], *target_buffers[receive_buffer_index]);
          break;
        }
        case RingStage::GATHER:
        {
          auto const send_buffer_index = (rank - current_step + 1 + number_ranks) % number_ranks;
          auto const receive_buffer_index = (rank - current_step + number_ranks) % number_ranks;

          source_buffers[send_buffer_index]->initTransfer(context);
          target_buffers[receive_buffer_index]->waitForCompletion();

          copy_to_source(*source_buffers[receive_buffer_index], *target_buffers[receive_buffer_index]);
          break;
        }
      }

      update_current_step();
      update_current_stage();

      if (is_running() && algorithm_final_state())
      {
        state = Operator::State::FINISHED;
      }
    }

    template<typename T>
    void Allreduce<T, AllreduceAlgorithm::RING>::copy_in(void* inputs)
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
    void Allreduce<T, AllreduceAlgorithm::RING>::copy_out(void* outputs)
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
    void Allreduce<T, AllreduceAlgorithm::RING>::update_current_step()
    {
      current_step++;
      if (current_step == steps_per_stage)
      {
        // move to next stage
        current_step = 0;
      }
    }

    template<typename T>
    void Allreduce<T, AllreduceAlgorithm::RING>::update_current_stage()
    {
      if (current_step == 0) // step count was just reset
      {
        stage = (stage == RingStage::REDUCE)? RingStage::GATHER : RingStage::REDUCE;
      }
    }

    template<typename T>
    bool Allreduce<T, AllreduceAlgorithm::RING>::algorithm_final_state()
    {
      // check whether the algorithm reached the final step of the final stage
      if (current_step == steps_per_stage - 1
         && stage == RingStage::GATHER)
      {
        return true;
      }
      return false;
    }

    template<typename T>
    void Allreduce<T, AllreduceAlgorithm::RING>::algorithm_reset_state()
    {
      current_step = 0;
      stage = RingStage::REDUCE;
    }


    template<typename T>
    bool Allreduce<T, AllreduceAlgorithm::RING>::step_reduce_stage(
              std::size_t step, SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
        source_comm.initTransfer(context);
        target_comm.waitForCompletion();

      return true;
    }

    template<typename T>
    bool Allreduce<T, AllreduceAlgorithm::RING>::step_gather_stage(
              std::size_t step, SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
        source_comm.initTransfer(context);
        target_comm.waitForCompletion();

      return true;
    }

    template<typename T>
    void Allreduce<T, AllreduceAlgorithm::RING>::apply_reduce_op(
              SourceBuffer& source_comm, TargetBuffer& target_comm)
    {
      auto num_elems = source_comm.description().size()/sizeof(T);
      gaspi::LocalBuffer<T> source_local(static_cast<T*>(source_comm.address()), num_elems);
      gaspi::LocalBuffer<T> target_local(static_cast<T*>(target_comm.address()), num_elems);

      std::transform(source_local.begin(), source_local.end(), target_local.begin(),
                     source_local.begin(), std::plus<T>());
    }

    template<typename T>
    void Allreduce<T, AllreduceAlgorithm::RING>::copy_to_source(
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