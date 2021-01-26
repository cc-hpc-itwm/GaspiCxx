#ifndef COLLECTIVES_COLLECTIVE_HPP_
#define COLLECTIVES_COLLECTIVE_HPP_

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

#endif
