#pragma once

#include <GaspiCxx/collectives/non_blocking/Operator.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>

#include <atomic>
#include <stdexcept>
#include <memory>
#include <vector>

namespace gaspi
{
  namespace collectives
  {
    enum class ReductionOp
    {
      SUM,
      AVERAGE,
    };

    enum class AllreduceAlgorithm
    {
      RING,
    };

    class AllreduceCommon : public Operator
    {
      public:
        AllreduceCommon(gaspi::segment::Segment& segment, // provide this automatically from segment manager?
                        gaspi::Context& context,
                        // gaspi::group::Group const& group,
                        std::size_t number_elements,
                        ReductionOp reduction_op)
        : state(Operator::State::NOT_STARTED),
          segment(segment),
          context(context),
          // group(group),
          number_elements(number_elements),
          reduction_op(reduction_op)
        { }

        template<typename T>
        virtual void start(std::vector<T> const& inputs) override
        {
          start(static_cast<void*>(inputs.data()));
        }

        void start(void* inputs) override
        {
          if (is_running())
          {
            throw std::logic_error("[AllreduceCommon::start] Operation already started.");
          }
          if (is_finished())
          {
            throw std::logic_error("[AllreduceCommon::start] Operation not reset after finish.");
          }

          copy_in(inputs);
          state = Operator::State::RUNNING;
        }

        template<typename T>
        virtual void reset_and_retrieve(std::vector<T>& outputs) override
        {
          reset_and_retrieve(static_cast<void*>(outputs.data()));
        }

        void reset_and_retrieve(void* outputs) override
        {
          if (is_running())
          {
            throw std::logic_error("[AllreduceCommon::reset_and_retrieve] Cannot reset while running.");
          }

          copy_out(outputs);
          state = Operator::State::NOT_STARTED;
        }

        bool is_running() const override
        {
          return state == Operator::State::RUNNING;
        };

        bool is_finished() const override
        {
          return state == Operator::State::FINISHED;
        }

      protected:
        std::atomic<Operator::State> state;
        gaspi::segment::Segment& segment;
        gaspi::Context& context;
        // gaspi::group::Group const& group;
        std::size_t number_elements;
        ReductionOp reduction_op;

        virtual void copy_in(void*);
        virtual void copy_out(void*);
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