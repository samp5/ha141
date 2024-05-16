#include "../extern/pybind11/include/pybind11/pybind11.h"
#include "../src/main.cpp"
namespace py = pybind11;
PYBIND11_MODULE(snn, m) {

  m.def("main", &main);
  // py::class_<RuntimConfig>(m, "RuntimConfig");
  // py::class_<SNN>(m, "SNN")
  //     .def(py::init<RuntimConfig &>())
  //     .def("run", &SNN::start);
}
