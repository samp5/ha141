#include "utils.h"
#include <vector>

bool print_map(const std::map<std::string, double> &m) {
  for (const auto &[k, v] : m) {
    std::cout << "Key: " << k << " Value: " << v << "\n";
  }
  return true;
}

/* (0,0) => 0
 * (m, n) => m*n - 1.
 *
 * For (a, b) => j, (c,d) => k, if a > c, j > k.
 *
 * @param pair coordinate (x,y) in a tuple
 * @param maxLayer the value of m (number of rows)
 * @return index of the 1D array
 */
int get_index(const std::tuple<int, int> &pair, int maxLayer) {
  int x = std::get<0>(pair);
  int y = std::get<1>(pair);

  int index = x + y * maxLayer;
  return index;
}
