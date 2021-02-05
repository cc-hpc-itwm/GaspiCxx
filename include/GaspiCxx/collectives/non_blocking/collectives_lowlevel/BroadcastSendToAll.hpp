#pragma once

#include <GaspiCxx/LocalBuffer.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    template<typename T>
    class BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL> : public BroadcastCommon
    {
      using SourceBuffer = gaspi::singlesided::write::SourceBuffer;
      using TargetBuffer = gaspi::singlesided::write::TargetBuffer;

      public:
        using BroadcastCommon::BroadcastCommon;

        BroadcastLowLevel(gaspi::segment::Segment& segment,
                          gaspi::group::Group const& group,
                          std::size_t number_elements,
                          gaspi::group::Rank const& root_rank);

      private:
        // std::vector<std::unique_ptr<SourceBuffer>>
        //   source_buffers;
        // std::vector<std::unique_ptr<TargetBuffer>>
        //   target_buffers;
        // std::vector<gaspi::singlesided::Endpoint::ConnectHandle> source_handles;
        // std::vector<gaspi::singlesided::Endpoint::ConnectHandle> target_handles;

        void waitForSetupImpl() override;
        void copyInImpl(void*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

    };

    template<typename T>
    BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::BroadcastLowLevel(
                      gaspi::segment::Segment& segment,
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      gaspi::group::Rank const& root_rank)
    : BroadcastCommon(segment, group, number_elements, root_rank),
      source_buffers(),
      target_buffers(),
    {
      auto const number_ranks = group.size().get();

      // neighbor ranks within the group
      auto const left_neighbor = (group.rank() - 1 + number_ranks) % number_ranks;
      auto const right_neighbor = (group.rank() + 1) % number_ranks;

      auto const number_elements_buffer = number_elements / number_ranks +
                                          (number_elements % number_ranks != 0 ? 1 : 0);
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
      for (auto i = 0; i < number_ranks; ++i)
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
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::waitForSetupImpl()
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
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::startImpl()
    { }

    template<typename T>
    bool BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::triggerProgressImpl()
    {
      auto const number_ranks = group.size().get();

      return false;
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::copyInImpl(void* inputs)
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
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::copyOutImpl(void* outputs)
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

  }
}