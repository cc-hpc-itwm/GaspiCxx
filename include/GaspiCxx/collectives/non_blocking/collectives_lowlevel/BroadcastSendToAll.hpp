#pragma once

#include <GaspiCxx/LocalBuffer.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/CollectiveLowLevel.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>

#include <algorithm>
#include <memory>
#include <numeric>
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
      using ConnectHandle = gaspi::singlesided::Endpoint::ConnectHandle;

      public:
        using BroadcastCommon::BroadcastCommon;

        BroadcastLowLevel(gaspi::segment::Segment& segment,
                          gaspi::group::Group const& group,
                          std::size_t number_elements,
                          gaspi::group::Rank const& root);

      private:
        gaspi::group::Rank rank;
        gaspi::group::Rank root;
        std::size_t number_ranks;
        std::size_t buffer_size_bytes;
        void* source_memory;

        std::vector<std::unique_ptr<SourceBuffer>> source_buffers;
        std::unique_ptr<TargetBuffer> target_buffer;

        std::vector<ConnectHandle> source_handles;
        std::unique_ptr<ConnectHandle> target_handle;

        void waitForSetupImpl() override;
        void copyInImpl(void const*) override;
        void copyOutImpl(void*) override;

        void startImpl() override;
        bool triggerProgressImpl() override;

        // implementation details
        void check_root_is_in_group() const;
        std::vector<gaspi::group::Rank> generate_non_root_ranks() const;
        void create_and_connect_first_source_buffer(
          gaspi::segment::Segment&,
          std::size_t,
          gaspi::group::Rank const&);
        void create_and_connect_remaining_source_buffers(
          gaspi::segment::Segment&,
          std::size_t,
          std::vector<gaspi::group::Rank> const&);
        void create_and_connect_target_buffer(gaspi::segment::Segment &,
                                              std::size_t);
    };

    template<typename T>
    BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::BroadcastLowLevel(
                      gaspi::segment::Segment& segment,
                      gaspi::group::Group const& group,
                      std::size_t number_elements,
                      gaspi::group::Rank const& root)
    : BroadcastCommon(segment, group, number_elements, root),
      rank(group.rank()),
      root(root),
      number_ranks(group.size()),
      buffer_size_bytes(sizeof(T) * number_elements),
      source_memory(nullptr),
      source_buffers(),
      target_buffer(),
      source_handles(),
      target_handle()
    {
      check_root_is_in_group();

      if (number_ranks == 1)
      {
        return;
      }

      if (rank == root)
      {
        auto non_root_ranks = generate_non_root_ranks();
        create_and_connect_first_source_buffer(
          segment, buffer_size_bytes, non_root_ranks.back());
        non_root_ranks.pop_back();
        create_and_connect_remaining_source_buffers(
          segment, buffer_size_bytes, non_root_ranks);
      }
      else
      {
        create_and_connect_target_buffer(segment, buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::waitForSetupImpl()
    {
      if (rank == root)
      {
        for (auto& source_handle : source_handles)
        {
          source_handle.waitForCompletion();
        }
      }
      else
      {
        target_handle->waitForCompletion();
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::startImpl()
    {
      if (rank == root)
      {
        for (auto& source_buffer : source_buffers)
        {
          source_buffer->initTransfer(context);
        }
      }
    }

    template<typename T>
    bool BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::triggerProgressImpl()
    {
      if (rank == root)
      {
        for (auto& source_buffer : source_buffers)
        {
          source_buffer->waitForTransferAck();
        }
      }
      else
      {
        target_buffer->waitForCompletion();
        target_buffer->ackTransfer(context);
      }
      return true;
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::copyInImpl(void const* inputs)
    {
      if (rank == root)
      {
        std::memcpy(source_memory, inputs, buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>::copyOutImpl(void* outputs)
    {
      if (rank == root)
      {
        std::memcpy(outputs, source_memory, buffer_size_bytes);
      }
      else
      {
        std::memcpy(outputs, target_buffer->address(), buffer_size_bytes);
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::check_root_is_in_group() const
    {
      if (root >= gaspi::group::Rank(number_ranks))
      {
        throw std::logic_error(
          "BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>: "
          "`group` must contain `root`");
      }
    }

    template<typename T>
    std::vector<gaspi::group::Rank>
    BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::generate_non_root_ranks() const
    {
      std::vector<gaspi::group::Rank> ranks;
      for (auto i = 0UL; i < number_ranks; ++i)
      {
        if(root != gaspi::group::Rank(i))
        {
          ranks.push_back(gaspi::group::Rank(i));
        }
      }
      return ranks;
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::create_and_connect_first_source_buffer(
                      gaspi::segment::Segment& segment,
                      std::size_t size_bytes,
                      gaspi::group::Rank const& other_rank)
    {
      SourceBuffer::Tag const tag = other_rank.get();
      source_buffers.push_back(
          std::make_unique<SourceBuffer>(segment, size_bytes));
      source_handles.push_back(
          source_buffers.back()->connectToRemoteTarget(
              context, other_rank, tag));
      source_memory = source_buffers.back()->address();
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::create_and_connect_remaining_source_buffers(
        gaspi::segment::Segment& segment,
        std::size_t size_bytes,
        std::vector<gaspi::group::Rank> const& ranks)
    {
      for (auto const& rank : ranks)
      {
        SourceBuffer::Tag const tag = rank.get();
        source_buffers.push_back(
            std::make_unique<SourceBuffer>(source_memory, segment, size_bytes));
        source_handles.push_back(
            source_buffers.back()->connectToRemoteTarget(context, rank, tag));
      }
    }

    template<typename T>
    void BroadcastLowLevel<T, BroadcastAlgorithm::SEND_TO_ALL>
      ::create_and_connect_target_buffer(
        gaspi::segment::Segment& segment,
        std::size_t size_bytes)
    {
      TargetBuffer::Tag const tag = rank.get();
      target_buffer = std::make_unique<TargetBuffer>(segment, size_bytes);
      target_handle = std::make_unique<ConnectHandle>(
        target_buffer->connectToRemoteSource(context, root, tag));
    }
  }
}