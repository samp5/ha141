#include "../pybind/snn_connect.h"
#include <map>
#include <tuple>

using AdjDictEntry =
    std::map<std::tuple<int, int>, std::map<std::string, float>>;

int main() {
  auto config = clientSNN::getDefaultConfig();

  clientSNN net(config);

  // (0, 0) (0, 1) (1, 0)
  // (0, 1) (0, 0) (1, 1)
  // (1, 0) (0, 0) (1, 1)
  // (1, 1) (0, 1) (1, 0)
  AdjDictEntry e1 = {{{0, 1}, {{"weight", 1}, {"delay", 2}}},
                     {{1, 0}, {{"weight", 3}, {"delay", 4}}}};

  AdjDictEntry e2 = {{{0, 0}, {{"weight", 5}, {"delay", 6}}},
                     {{1, 1}, {{"weight", 7}, {"delay", 8}}}};

  AdjDictEntry e3 = {{{0, 0}, {{"weight", 9}, {"delay", 10}}},
                     {{1, 1}, {{"weight", 11}, {"delay", 12}}}};

  AdjDictEntry e4 = {{{0, 1}, {{"weight", 13}, {"delay", 14}}},
                     {{1, 0}, {{"weight", 15}, {"delay", 16}}}};
  AdjDict d = {
      {{0, 0}, e1},
      {{0, 1}, e2},
      {{1, 0}, e3},
      {{1, 1}, e4},
  };

  // 0,1:1:2,2:3:4

  net.initialize(d);
}
