#include "../../extern/pybind/include/pybind11/pybind11.h"
#include "../../extern/pybind/include/pybind11/stl.h"
#include <iostream>
// #include "../network.hpp"
#include <vector>
namespace py = pybind11;

int add(int i, int j) { return i + j; }

void test(py::buffer buff) {
  typedef std::vector<std::vector<int>> matrix;

  py::buffer_info info = buff.request();

  matrix m;
  std::cout << "shape: " << info.shape.at(0) << ", " << info.shape.at(1)
            << std::endl;
  std::cout << "ptr: " << info.ptr << std::endl;

  std::cout << "strides: " << std::endl;
  for (auto i : info.strides) {
    std::cout << i << " ";
  }

  std::cout << "row_ptr" << *((int *)info.ptr) << std::endl;
  std::cout << "row_ptr + 8" << *((int *)info.ptr + info.strides.at(0))
            << std::endl;
  std::cout << "row_ptr + 4" << *((int *)info.ptr + 4) << std::endl;

  std::cout << "row_ptr + 2" << *((int *)info.ptr + 2) << std::endl;
  std::cout << "row_ptr + 6" << *((int *)info.ptr + 6) << std::endl;
  std::cout << "row_ptr + 10" << *((int *)info.ptr + 10) << std::endl;
  std::cout << "row_ptr + 16" << *((int *)info.ptr + 16) << std::endl;

  std::cout << std::endl;

  for (auto i = 0; i < info.shape.at(0); i++) {
    const int *row_ptr = (int *)info.ptr + i * info.strides.at(0) / sizeof(int);
    std::cout << "*row_ptr: " << *row_ptr << std::endl;
    std::vector<int> row;
    for (auto j = 0; j < info.shape.at(1); j++) {
      row.push_back(*(row_ptr + info.strides.at(0) / sizeof(int)));
    }
    m.push_back(row);
  }

  for (auto r : m) {
    for (auto e : r) {
      std::cout << e << " ";
    }
    std::cout << '\n';
  }
}

PYBIND11_MODULE(snn, m) {
  m.def("add", &add, "A function that adds two numbers");
  m.def("test", &test);

  // py::class_<SNN>(m, "SNN")
  //     .def(py::init<std::vector<std::string>>())
  //     .def("generateSynapses", &SNN::generateRandomSynapses)
  //     .def("start", &SNN::start)
  //     .def("join", &SNN::join)
  //     .def("writeData", &SNN::pyWrite);
}
