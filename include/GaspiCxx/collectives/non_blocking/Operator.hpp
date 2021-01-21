#pragma once

namespace gaspi
{
namespace collectives
{
    class Operator
    {
      public:
        enum class State
        {
          NOT_STARTED,
          RUNNING,
          FINISHED,
        };

        virtual ~Operator() = default;

        // Initiates the Operator (non-blocking):
        // copies in the `inputs` and sets `is_running` to TRUE
        virtual void start(void* inputs) = 0;
  
        // Makes partial progress towards computing the Operator's result
        // and has to be called multiple times until completion.
        // Then is_finished will be set to TRUE.
        // `trigger_communication_step` can be called independently of
        // the Operator's state, but it only tries to make progress if is_running == TRUE
        virtual void trigger_communication_step() = 0;

        // Enables the Allreduce to be started again:
        // copies out the results to `outputs`
        // and sets `is_running` to FALSE and `is_finished` to FALSE
        virtual void reset_and_retrieve(void* outputs) = 0;

        // CAUTION:
        // virtual void finish(void* outputs) = 0;
        // Cannot guarantee that `trigger_communication_step` and `finish`
        // is not called at the same time by different threads

        // Alternative:
        // Only expose `start`, `finish` and `is_running`
        // to public and let the library manage progress by itself

        // Return TRUE if `start()` was called, and the Operator execution
        // is not finished
        virtual bool is_running() const = 0;
  
        // If TRUE, results are available until `reset_and_retrieve()` is called
        virtual bool is_finished() const = 0;
    };
}
}