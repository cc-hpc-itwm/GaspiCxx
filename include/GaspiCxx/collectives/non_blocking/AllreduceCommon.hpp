#pragma once

#include <GaspiCxx/collectives/non_blocking/Operator.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/singlesided/write/SourceBuffer.hpp>
#include <GaspiCxx/singlesided/write/TargetBuffer.hpp>
#include <GaspiCxx/singlesided/BufferDescription.hpp>

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

    class AllreduceCommon : public Operator
    {
      public:
        AllreduceCommon(gaspi::segment::Segment& segment, // provide this automatically from segment manager?
                        gaspi::Context& context,
                        // gaspi::group::Group const& group,
                        std::size_t number_elements,
                        ReductionOp reduction_op);

        template<typename T>
        void start(std::vector<T> const& inputs);
        void start(void* inputs) override;

        template<typename T>
        void reset_and_retrieve(std::vector<T>& outputs);
        void reset_and_retrieve(void* outputs) override;

        bool is_running() const override;
        bool is_finished() const override;

      protected:
        std::atomic<Operator::State> state;
        gaspi::segment::Segment& segment;
        gaspi::Context& context;
        gaspi::group::Group const& group;
        std::size_t number_elements;
        ReductionOp reduction_op;

      private:
        virtual void copy_in(void*) = 0;
        virtual void copy_out(void*) = 0;
    };
  }
}
