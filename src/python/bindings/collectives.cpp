#include <collectives.hpp>

namespace py = pybind11;

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
      declare_collective.template operator()<CollectiveOp<T, algorithm>>(m, algorithm_name);
    });
}
#define DEFINE_BINDINGS(COLLECTIVE, TYPE, ALG_INFO, BINDINGSCLASS)  \
  gen_bindings_list_of_algorithms<TYPE,  \
                                  gaspi::collectives::ALG_INFO, \
                                  gaspi::collectives::COLLECTIVE> \
                                  (m, BINDINGSCLASS(#COLLECTIVE, #TYPE));

    auto Bindings::generate_instantiated_collective_name(std::string const& algorithm_name)
    {
      return class_name + "_" + dtype_name + "_" + algorithm_name;
    }


template <class BcastClass>
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
        [](BcastClass& bcast, std::optional<py::array> input)
        {
          if (input)
          {
            auto input_ptr = input->data(0);
            bcast.start(input_ptr);
          }
          else
          {
            bcast.start();
          }
        }, py::arg("input").none(true))
    .def("waitForCompletion",
        [](BcastClass& bcast) { return bcast.waitForCompletion(); },
          py::return_value_policy::move);
}

void bcast_factory(py::module &m)
{
  DEFINE_BINDINGS(Broadcast, int, BroadcastInfo, BroadcastBindings)
  DEFINE_BINDINGS(Broadcast, float, BroadcastInfo, BroadcastBindings)
  DEFINE_BINDINGS(Broadcast, double, BroadcastInfo, BroadcastBindings)
}

template <class AllreduceClass>
void AllreduceBindings::operator()(py::module &m, std::string algorithm_name)
{
  auto const pyclass_name = generate_instantiated_collective_name(algorithm_name);
  py::class_<AllreduceClass>(m, pyclass_name.c_str())
    .def(py::init([](gaspi::group::Group const& group, std::size_t nelems, gaspi::collectives::ReductionOp op)
        {
          return std::make_unique<AllreduceClass>(group, nelems, op);
        }
        ), py::return_value_policy::move)
    .def("start",
        [](AllreduceClass& allreduce, std::optional<py::array> input)
        {
          auto input_ptr = input->data(0);
          allreduce.start(input_ptr);
        })
    .def("waitForCompletion",
        [](AllreduceClass& allreduce) { return allreduce.waitForCompletion(); },
        py::return_value_policy::move);
}

void allreduce_factory(py::module &m)
{
  DEFINE_BINDINGS(Allreduce, int, AllreduceInfo, AllreduceBindings)
  DEFINE_BINDINGS(Allreduce, float, AllreduceInfo, AllreduceBindings)
  DEFINE_BINDINGS(Allreduce, double, AllreduceInfo, AllreduceBindings)
}
