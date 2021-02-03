#pragma once

namespace gaspi {
namespace collectives {

class Collective
{
public:

  virtual	~Collective() = default;

  virtual void start(void* inputs) = 0;
  virtual void waitForCompletion(void* outputs) = 0;

};

}
}
