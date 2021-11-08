#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllreduceCommon.hpp>
#include <GaspiCxx/collectives/non_blocking/Allreduce.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/AllgathervCommon.hpp>
#include <GaspiCxx/collectives/non_blocking/Allgatherv.hpp>
#include <GaspiCxx/collectives/non_blocking/collectives_lowlevel/BroadcastCommon.hpp>
#include <GaspiCxx/collectives/non_blocking/Broadcast.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/Runtime.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

std::string generate_implemented_primitive_name(std::string const& class_name,
                                                std::string const& dtype_name,
                                                std::string const& algorithm_name);

class Bindings
{
  public:
    Bindings(std::string class_name, std::string dtype_name)
    : class_name(class_name), dtype_name(dtype_name)
    {}
    virtual ~Bindings()
    {}

  protected:
    auto generate_instantiated_collective_name(std::string const& algorithm_name);

  private:
    std::string class_name;
    std::string dtype_name;
};


class AllgathervBindings : public Bindings
{
  public:
    using Bindings::Bindings;

    template <class AllgathervClass, typename T>
    void operator()(py::module &m, std::string algorithm_name);
};

void allgatherv_factory(py::module &m);


class AllreduceBindings : public Bindings
{
  public:
    using Bindings::Bindings;

    template <class AllreduceClass, typename T>
    void operator()(py::module &m, std::string algorithm_name);
};

void allreduce_factory(py::module &m);

class BroadcastBindings : public Bindings
{
  public:
    using Bindings::Bindings;

    template <class BcastClass, typename T>
    void operator()(py::module &m, std::string algorithm_name);
};

void bcast_factory(py::module &m);
