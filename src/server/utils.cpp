#include "utils.h"
#include <vector>

bool print_map(const std::map<std::string, int> &m) {
  for (const auto &[k, v] : m) {
    std::cout << "Key: " << k << " Value: " << v << "\n";
  }
  return true;
}

int sum_map(const std::map<std::string, int> &m) {
  int sum = 0;
  for (const auto &[k, v] : m) {
    sum += v;
  }
  return sum;
}

bool remove_duplicates(std::map<std::string, int> &m) {
  std::map<int, std::string> rev;
  std::vector<std::string> to_remove;
  for (const auto &[k, v] : m) {
    if (rev.count(v) == 0) {
      rev.emplace(v, k);
    } else {
      to_remove.push_back(k);
    }
  }
  if (to_remove.empty()) {
    return false;
  } else {
    for (const auto &s : to_remove) {
      m.erase(s);
    }
    return true;
  }
}
