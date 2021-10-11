#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/Runtime.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <utility>

namespace py = pybind11;

PYBIND11_MODULE(pygpi, m)
{
  m.doc() = "GaspiCxx communication library";

  m.def("initPyGPI", []()
                {
                  gaspi::initGaspiCxx();
                });
  m.def("get_rank", []()
                {
                  return gaspi::getRuntime().global_rank();
                });
  m.def("get_size", []()
                {
                  return gaspi::getRuntime().size();
                });
  py::class_<gaspi::group::Group>(m, "Group")
    .def(py::init<>())
    .def(py::init<std::vector<gaspi::group::GlobalRank> const&>())
    .def_property_readonly("group", &gaspi::group::Group::group)
    .def_property_readonly("size", &gaspi::group::Group::size)
    .def_property_readonly("rank", [](gaspi::group::Group const& g) { return g.rank().get(); })
    .def_property_readonly("global_rank", [](gaspi::group::Group const& g) { return g.global_rank(); })
    .def("contains_rank", &gaspi::group::Group::contains_rank)
    .def("contains_global_rank", &gaspi::group::Group::contains_global_rank);
}
