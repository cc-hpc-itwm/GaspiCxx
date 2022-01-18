#include <GaspiCxx/collectives/Barrier.hpp>
#include <GaspiCxx/group/Group.hpp>
#include <GaspiCxx/Runtime.hpp>
#include <collectives.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

PYBIND11_MODULE(pygpi_wrappers, m)
{
  m.doc() = "GaspiCxx communication library";

  m.def("init_pygpi", []()
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
    .def("contains_global_rank", &gaspi::group::Group::contains_global_rank)
    .def("to_global_rank", [](gaspi::group::Group const& g, std::size_t local_rank)
        {
          return g.toGlobalRank(gaspi::group::Rank(local_rank));
        })
    .def("to_group_rank", [](gaspi::group::Group const& g, gaspi::group::GlobalRank const& global_rank)
        {
          return g.toGroupRank(global_rank).get();
        });

  py::class_<gaspi::collectives::blocking::Barrier>(m, "Barrier")
    .def(py::init([](std::optional<gaspi::group::Group> group)
        {
          using Barrier = gaspi::collectives::blocking::Barrier;
          if (!group)
          {
            return std::make_unique<Barrier>(gaspi::group::Group());
          }
          else
          {
            return std::make_unique<Barrier>(*group);
          }
        }
        ), py::arg("group") = py::none(),
           py::return_value_policy::move)
    .def("execute", &gaspi::collectives::blocking::Barrier::execute);

  py::enum_<gaspi::collectives::ReductionOp>(m, "ReductionOp")
     .value("SUM", gaspi::collectives::ReductionOp::SUM)
     .value("PROD", gaspi::collectives::ReductionOp::PROD);

  m.def("generate_implemented_primitive_name", &generate_implemented_primitive_name,
        py::arg("collective"), py::arg("dtype"), py::arg("algorithm"));

  allgatherv_factory(m);
  allreduce_factory(m);
  bcast_factory(m);
}
