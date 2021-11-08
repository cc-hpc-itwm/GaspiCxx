#include <collectives.hpp>

namespace py = pybind11;

std::string generate_implemented_primitive_name(std::string const& class_name,
                                                std::string const& dtype_name,
                                                std::string const& algorithm_name)
{
  return class_name + "_" + dtype_name + "_" + algorithm_name;
}

template <auto Start, auto End, class F>
constexpr void constexpr_range(F&& f)
{
  if constexpr (Start < End)
  {
    f(std::integral_constant<decltype(Start), Start>());
    constexpr_range<Start + 1, End>(f);
  }
}

template <typename T,
          typename AlgorithmsInfo,
          template <typename TT, typename AlgorithmsInfo::Algorithm> typename CollectiveOp,
          typename Func>
constexpr void gen_bindings_list_of_algorithms(py::module& m, Func declare_collective)
{
  constexpr_range<std::size_t(0), AlgorithmsInfo::implemented.size()>(
    [&m, &declare_collective](auto i)
    {
      auto constexpr algorithm = AlgorithmsInfo::implemented[i];
      auto const algorithm_name = AlgorithmsInfo::names[algorithm];
      declare_collective.template operator()<CollectiveOp<T, algorithm>, T>(m, algorithm_name);
    });
}
#define DEFINE_BINDINGS(COLLECTIVE, TYPE, ALG_INFO, BINDINGSCLASS)  \
  gen_bindings_list_of_algorithms<TYPE,  \
                                  gaspi::collectives::ALG_INFO, \
                                  gaspi::collectives::COLLECTIVE> \
                                  (m, BINDINGSCLASS(#COLLECTIVE, #TYPE));

auto Bindings::generate_instantiated_collective_name(std::string const& algorithm_name)
{
  return generate_implemented_primitive_name(class_name, dtype_name, algorithm_name);
}


template <class AllgathervClass, typename T>
void AllgathervBindings::operator()(py::module &m, std::string algorithm_name)
{
  auto const pyclass_name = generate_instantiated_collective_name(algorithm_name);
  py::class_<AllgathervClass>(m, pyclass_name.c_str())
    .def(py::init([](gaspi::group::Group const& group, std::size_t nelems)
        {
          return std::make_unique<AllgathervClass>(group, nelems);
        }
        ), py::arg("group"), py::arg("nelems"),
           py::return_value_policy::move)
    .def("start",
        [](AllgathervClass& allgatherv, std::optional<py::array> data)
        {
          allgatherv.start(data->data());
        })
    .def("start",
        [](AllgathervClass& allgatherv, std::optional<std::vector<T>> data)
        {
          allgatherv.start(data->data());
        })
    .def("start",
        [](AllgathervClass& allgatherv, std::optional<T> data)
        {
          allgatherv.start(&data);
        })
    .def("wait_for_completion",
        [](AllgathervClass& allgatherv)
        {
          py::array_t<T> output(allgatherv.getOutputCount());
          allgatherv.waitForCompletion(output.mutable_data());
          return output;
        },
        py::return_value_policy::move);

}

void allgatherv_factory(py::module &m)
{
  DEFINE_BINDINGS(Allgatherv, int, AllgathervInfo, AllgathervBindings)
  DEFINE_BINDINGS(Allgatherv, long, AllgathervInfo, AllgathervBindings)
  DEFINE_BINDINGS(Allgatherv, float, AllgathervInfo, AllgathervBindings)
  DEFINE_BINDINGS(Allgatherv, double, AllgathervInfo, AllgathervBindings)
}


template <class AllreduceClass, typename T>
void AllreduceBindings::operator()(py::module &m, std::string algorithm_name)
{
  auto const pyclass_name = generate_instantiated_collective_name(algorithm_name);
  py::class_<AllreduceClass>(m, pyclass_name.c_str())
    .def(py::init([](gaspi::group::Group const& group, std::size_t nelems, gaspi::collectives::ReductionOp op)
        {
          return std::make_unique<AllreduceClass>(group, nelems, op);
        }
        ), py::arg("group"), py::arg("nelems"), py::arg("op"),
           py::return_value_policy::move)
    .def("start",
        [](AllreduceClass& allreduce, py::array data)
        {
          allreduce.start(data.data());
        })
    .def("start", // overload for list inputs
        [](AllreduceClass& allreduce, std::vector<T> const& data)
        {
          allreduce.start(data.data());
        })
    .def("start", // overload for single values
        [](AllreduceClass& allreduce, T const data)
        {
          allreduce.start(&data);
        })
    .def("wait_for_completion",
        [](AllreduceClass& allreduce)
        {
          py::array_t<T> output(allreduce.getOutputCount());
          allreduce.waitForCompletion(output.mutable_data());
          return output;
        },
        py::return_value_policy::move);
}

void allreduce_factory(py::module &m)
{
  DEFINE_BINDINGS(Allreduce, int, AllreduceInfo, AllreduceBindings)
  DEFINE_BINDINGS(Allreduce, long, AllreduceInfo, AllreduceBindings)
  DEFINE_BINDINGS(Allreduce, float, AllreduceInfo, AllreduceBindings)
  DEFINE_BINDINGS(Allreduce, double, AllreduceInfo, AllreduceBindings)
}

template <class BcastClass, typename T>
void BroadcastBindings::operator()(py::module &m, std::string algorithm_name)
{
  auto const pyclass_name = generate_instantiated_collective_name(algorithm_name);
  py::class_<BcastClass>(m, pyclass_name.c_str())
    .def(py::init([](gaspi::group::Group const& group, std::size_t nelems, std::size_t root)
        {
          return std::make_unique<BcastClass>(group, nelems, gaspi::group::Rank(root));
        }
        ), py::arg("group"), py::arg("nelems"), py::arg("root"),
           py::return_value_policy::move)
    .def("start",
        [](BcastClass& bcast, std::optional<py::array> data)
        {
          if (data)
          {
            bcast.start(data->data());
          }
          else
          {
            bcast.start();
          }
        },
        py::arg("data") = py::none())
    .def("start",
        [](BcastClass& bcast, std::optional<std::vector<T>> data)
        {
          if (data)
          {
            bcast.start(data->data());
          }
          else
          {
            bcast.start();
          }
        },
        py::arg("data") = py::none())
    .def("start",
        [](BcastClass& bcast, std::optional<T> data)
        {
          if (data)
          {
            bcast.start(&data);
          }
          else
          {
            bcast.start();
          }
        },
        py::arg("data") = py::none())
    .def("wait_for_completion",
        [](BcastClass& bcast)
        {
          py::array_t<T> output(bcast.getOutputCount());
          bcast.waitForCompletion(output.mutable_data());
          return output;
        },
        py::return_value_policy::move);

}

void bcast_factory(py::module &m)
{
  DEFINE_BINDINGS(Broadcast, int, BroadcastInfo, BroadcastBindings)
  DEFINE_BINDINGS(Broadcast, long, BroadcastInfo, BroadcastBindings)
  DEFINE_BINDINGS(Broadcast, float, BroadcastInfo, BroadcastBindings)
  DEFINE_BINDINGS(Broadcast, double, BroadcastInfo, BroadcastBindings)
}
