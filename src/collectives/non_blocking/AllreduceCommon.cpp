
#include <GaspiCxx/collectives/non_blocking/Operator.hpp>
#include <GaspiCxx/Context.hpp>
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
    AllreduceCommon::AllreduceCommon(gaspi::segment::Segment& segment, // provide this automatically from segment manager?
                    gaspi::Context& context,
                    // gaspi::group::Group const& group,
                    std::size_t number_elements,
                    ReductionOp reduction_op)
    : state(Operator::State::NOT_STARTED),
      segment(segment),
      context(context),
      group(context.group()),
      number_elements(number_elements),
      reduction_op(reduction_op)
    { }

    template<typename T>
    void AllreduceCommon::start(std::vector<T> const& inputs)
    {
      start(static_cast<void*>(inputs.data()));
    }

    void AllreduceCommon::start(void* inputs)
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
    void AllreduceCommon::reset_and_retrieve(std::vector<T>& outputs)
    {
      reset_and_retrieve(static_cast<void*>(outputs.data()));
    }

    void AllreduceCommon::reset_and_retrieve(void* outputs)
    {
      if (is_running())
      {
        throw std::logic_error("[AllreduceCommon::reset_and_retrieve] Cannot reset while running.");
      }

      copy_out(outputs);
      state = Operator::State::NOT_STARTED;
    }

    bool AllreduceCommon::is_running() const
    {
      return state == Operator::State::RUNNING;
    }

    bool AllreduceCommon::is_finished() const
    {
      return state == Operator::State::FINISHED;
    }
  }
}